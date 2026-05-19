/* global describe it cy beforeEach require expect */

var helper = require('../../common/helper');
var impressHelper = require('../../common/impress_helper');
var desktopHelper = require('../../common/desktop_helper')

describe(['tagdesktop', 'tagnextcloud', 'tagproxy'], 'Impress shape SVG flicker.', function() {

	beforeEach(function() {
		helper.setupAndLoadDocument('impress/undo_redo.odp');
		cy.viewport(1920, 1080);
	});

	// Regression test for: https://github.com/CollaboraOnline/online/pull/15037
	// Bug: After editing text in a shape and pressing ESC, the SVG overlay
	// briefly flickers because it is shown with stale content before being
	// hidden again. The fix removes the showSVG() call from the visibility
	// toggle; the SVG is only shown on demand (drag/resize).
	it('No SVG flicker after editing text and pressing Escape.', function() {
		desktopHelper.selectZoomLevel(70);
		// Select the text shape in the center.
		impressHelper.selectTextShapeInTheCenter();

		// Double-click to enter text editing mode.
		impressHelper.dblclickOnSelectedShape();

		// Type some text.
		helper.typeIntoDocument('Test text');

		// Install a MutationObserver on the SVG element to detect if
		// it ever becomes visible (display != 'none') after we press Escape.
		cy.cGet('#canvas-container > svg').then(function($svg) {
			var svg = $svg[0];

			// Ensure SVG reference is stored on the window for later access.
			cy.getFrameWindow().then(function(win) {
				win._testSvgWasShown = false;

				var observer = new MutationObserver(function(mutations) {
					mutations.forEach(function(mutation) {
						if (mutation.attributeName === 'style') {
							var display = svg.style.display;
							if (display !== 'none') {
								win._testSvgWasShown = true;
							}
						}
					});
				});

				observer.observe(svg, { attributes: true, attributeFilter: ['style'] });
				win._testSvgObserver = observer;
			});
		});

		// Press Escape to exit text editing mode.
		helper.typeIntoDocument('{esc}');

		cy.getFrameWindow().then(function(win) {
			helper.processToIdle(win);
		});

		// Verify shape handles are visible (we are back to shape selection).
		cy.cGet('#test-div-shapeHandlesSection').should('exist');

		// Check that the SVG was never shown during the transition.
		cy.getFrameWindow().then(function(win) {
			expect(win._testSvgWasShown).to.be.false;

			// Clean up the observer.
			if (win._testSvgObserver) {
				win._testSvgObserver.disconnect();
			}
		});
	});
});
