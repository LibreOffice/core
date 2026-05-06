/* global describe it cy beforeEach require expect */

var helper = require('../../common/helper');

describe(['tagdesktop', 'tagnextcloud', 'tagproxy'], 'Multipage View scrollbar pressing.', function() {

	beforeEach(function() {
		helper.setupAndLoadDocument('writer/scrolling.odt');
		cy.viewport(1920, 1080);
	});

	// Regression test for: Multipage View: Fix "scroll by pressing on scrollbar" feature.
	// Bug: Scrollbar dragging did not scroll the document because
	// verticalScrollRatio and horizontalScrollRatio were not set,
	// so the drag delta was multiplied by 0 (or undefined).
	it('Scrollbar scroll ratios are set in multi-page view.', function() {
		// Switch to multi-page view.
		cy.cGet('#multi-page-view-button').click();

		cy.getFrameWindow().then(function(win) {
			helper.processToIdle(win);
		});

		// Verify scroll ratios are set correctly for scrollbar dragging.
		cy.getFrameWindow().then(function(win) {
			var layout = win.app.activeDocument.activeLayout;
			expect(layout.type).to.equal('ViewLayoutMultiPage');

			var scrollProps = layout.scrollProperties;
			// verticalScrollRatio must be 20 to compensate for the /20 dampening in scroll().
			expect(scrollProps.verticalScrollRatio, 'verticalScrollRatio').to.equal(20);
			// horizontalScrollRatio must be 1 (no dampening on horizontal scroll).
			expect(scrollProps.horizontalScrollRatio, 'horizontalScrollRatio').to.equal(1);
		});
	});

	// Verify that scrolling actually works via the scrollbar API in multi-page view.
	it('Vertical scroll via scrollbar offset works in multi-page view.', function() {
		// Switch to multi-page view.
		cy.cGet('#multi-page-view-button').click();

		cy.getFrameWindow().then(function(win) {
			helper.processToIdle(win);
		});

		// Record initial scroll position.
		var initialViewY;
		cy.getFrameWindow().then(function(win) {
			var layout = win.app.activeDocument.activeLayout;
			initialViewY = layout.scrollProperties.viewY;
		});

		// Scroll down using the scroll section API.
		cy.getFrameWindow().then(function(win) {
			win.app.sectionContainer.getSectionWithName('scroll').scrollVerticalWithOffset(200);
		});

		// viewY should have increased (scrolled down).
		cy.getFrameWindow().then(function(win) {
			cy.wrap(null).should(function() {
				var layout = win.app.activeDocument.activeLayout;
				expect(layout.scrollProperties.viewY, 'viewY after scroll').to.be.greaterThan(initialViewY);
			});
		});
	});
});
