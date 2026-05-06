/* global describe it cy beforeEach require expect */

var helper = require('../../common/helper');

describe(['tagdesktop', 'tagnextcloud', 'tagproxy'], 'Writer multi-page view layout reset.', function() {

	beforeEach(function() {
		helper.setupAndLoadDocument('writer/copy_paste.odt');
		cy.viewport(1920, 1080);
	});

	// Regression test for: https://github.com/CollaboraOnline/online/pull/14606
	// Bug: When pages change in multi-page view, the layout was not reset,
	// so newly added pages were not visible. The fix calls reset() on the
	// ViewLayoutMultiPage when page rectangles change.
	it('Adding a page in multi-page view updates layout rectangles.', function() {
		// Switch to multi-page view.
		cy.cGet('#multi-page-view-button').click();

		cy.getFrameWindow().then(function(win) {
			helper.processToIdle(win);
		});

		// Verify we are in multi-page layout and have 1 page.
		cy.getFrameWindow().then(function(win) {
			var layout = win.app.activeDocument.activeLayout;
			expect(layout.type).to.equal('ViewLayoutMultiPage');
			expect(layout.documentRectangles.length).to.equal(1);
			expect(layout.viewRectangles.length).to.equal(1);
		});

		// Insert a page break to create a second page.
		helper.typeIntoDocument('{ctrl}{enter}');

		cy.getFrameWindow().then(function(win) {
			helper.processToIdle(win);
		});

		// After page break, layout should have been reset with 2 pages.
		cy.getFrameWindow().then(function(win) {
			var layout = win.app.activeDocument.activeLayout;
			expect(layout.documentRectangles.length).to.equal(2);
			expect(layout.viewRectangles.length).to.equal(2);
		});
	});
});
