/* -*- js-indent-level: 8 -*- */
/* global describe it cy beforeEach require expect */

var helper = require('../../common/helper');
var desktopHelper = require('../../common/desktop_helper');

// Guards the zoom-animation drawing contract:
//  - while the scale animates, the section container is asked to redraw on
//    every frame (the rafFunc -> requestReDraw loop), so vector sections track
//    the zoom instead of freezing;
//  - once the animation settles, the global app.pixelsToTwips returns to the
//    exact scale of the (restored) zoom level, i.e. it is not left at an
//    intermediate base*frameScale value from mid-animation.
describe(['tagdesktop'], 'Zoom animation redraw', function() {
	beforeEach(function() {
		cy.viewport(1400, 1000);
		helper.setupAndLoadDocument('writer/help_dialog.odt');
	});

	it('redraws each frame while zooming and restores the scale', function() {
		// Snapshot the settled scale and start counting redraw requests.
		cy.getFrameWindow().then(function(win) {
			return helper.processToIdle(win).then(function() {
				var sc = win.app.sectionContainer;
				sc.__initialP2T = win.app.pixelsToTwips;
				sc.__drawRequests = 0;
				sc.__origRequestReDraw = sc.requestReDraw.bind(sc);
				sc.requestReDraw = function() {
					sc.__drawRequests++;
					return sc.__origRequestReDraw();
				};
			});
		});

		desktopHelper.zoomIn();

		cy.getFrameWindow().then(function(win) {
			var sc = win.app.sectionContainer;
			// A zoom animation runs many frames; each one issues a redraw
			// request. If the per-frame loop regressed this would be ~0.
			expect(sc.__drawRequests, 'redraw requests during zoom').to.be.greaterThan(2);
			// Sanity: the zoom actually changed the scale.
			expect(win.app.pixelsToTwips).to.not.equal(sc.__initialP2T);
			// Restore the real method before the rest of the run.
			sc.requestReDraw = sc.__origRequestReDraw;
		});

		// Zoom back to the starting level.
		desktopHelper.zoomOut();

		cy.getFrameWindow().then(function(win) {
			return helper.processToIdle(win).then(function() {
				// Back at the original zoom, app.pixelsToTwips must equal the
				// snapshot: the animation has to leave it at the scale of the
				// committed zoom (derived from the tiles core renders), not a
				// stale value from a mid-animation frame.
				expect(win.app.pixelsToTwips).to.equal(win.app.sectionContainer.__initialP2T);
			});
		});
	});
});
