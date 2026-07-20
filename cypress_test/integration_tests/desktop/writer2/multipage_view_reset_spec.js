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

describe(['tagdesktop'], 'Writer remembers the last used view mode.', function() {

	var documentFilePath;

	// Persisting the view mode needs a per-user settings store. The dev WOPI
	// server only persists userconfig uploads for a test that opts in with an
	// explicit &userid; the default user ("test") is intentionally not
	// persisted (see skipPersist in test/TestWopiFileServer.hpp). Without this
	// the reload would never see the remembered mode and the test would fail.
	var viewModeUser = 'userid=writer-multipage';

	beforeEach(function() {
		documentFilePath = helper.setupAndLoadDocument('writer/copy_paste.odt', false, false, undefined, viewModeUser);
		cy.viewport(1920, 1080);
	});

	// Regression test: the last used view mode (multi-page vs normal) must be
	// remembered per document across reopens. The hard part is the SECOND
	// change: switching multi-page -> normal and reopening has to restore
	// normal. This used to reopen in multi-page instead, because the dev WOPI
	// returned a content-independent "stamp" for viewsetting.json, so coolwsd's
	// config cache kept serving the first (multi-page) copy and never
	// re-fetched the updated value. A single change appeared to work, which
	// hid the bug. Verify this catches the regression by reverting the
	// TestWopiFileServer.hpp stamp change: the final assertion must then fail.
	it('Restores multi-page then normal view across document reloads.', function() {
		// Drive to a known starting state (multi-page), independent of any mode
		// a previous test may have persisted for this document.
		cy.getFrameWindow().then(function(win) {
			if (win.app.activeDocument.activeLayout.type !== 'ViewLayoutMultiPage')
				win.app.dispatcher.dispatch('multipageview');
			return helper.processToIdle(win);
		});
		cy.getFrameWindow().its('app.activeDocument.activeLayout.type')
			.should('equal', 'ViewLayoutMultiPage');

		// Reopen: the multi-page choice must be restored (first change - this
		// passed even with the bug).
		helper.reloadDocument(documentFilePath, viewModeUser);
		cy.getFrameWindow().then(function(win) { return helper.processToIdle(win); });
		cy.getFrameWindow().its('app.activeDocument.activeLayout.type')
			.should('equal', 'ViewLayoutMultiPage');

		// Switch back to normal view.
		cy.getFrameWindow().then(function(win) {
			win.app.dispatcher.dispatch('multipageview');
			return helper.processToIdle(win);
		});
		cy.getFrameWindow().its('app.activeDocument.activeLayout.type')
			.should('equal', 'ViewLayoutWriter');

		// Reopen: normal must be restored. This is the regression - before the
		// fix the document reopened in multi-page view here.
		helper.reloadDocument(documentFilePath, viewModeUser);
		cy.getFrameWindow().then(function(win) { return helper.processToIdle(win); });
		cy.getFrameWindow().its('app.activeDocument.activeLayout.type')
			.should('equal', 'ViewLayoutWriter');
	});
});

describe(['tagdesktop'], 'Writer multi-page to normal view keeps the page centered.', function() {

	beforeEach(function() {
		helper.setupAndLoadDocument('writer/copy_paste.odt');
		cy.viewport(1920, 1080);
	});

	// The page's left edge on the canvas, and the empty space to its left and
	// right within the document drawing area. Equal left and right space means
	// the page sits in the middle. All values are canvas (core) pixels.
	function pageMargins(win) {
		var layout = win.app.activeDocument.activeLayout;
		var anchor = win.app.sectionContainer.getDocumentAnchorSection();
		var pageRect = win.app.file.writer.pageRectangleList[0];

		var leftEdge = layout.documentToViewX(
			new win.cool.SimplePoint(pageRect[0], pageRect[1]));
		var rightEdge = layout.documentToViewX(
			new win.cool.SimplePoint(pageRect[0] + pageRect[2], pageRect[1]));

		return {
			left: leftEdge - anchor.myTopLeft[0],
			right: anchor.myTopLeft[0] + anchor.size[0] - rightEdge,
		};
	}

	// Regression test: returning from multi-page view to normal view must seed
	// the new normal-view layout with the document size again, so the page is
	// centered instead of drifting to the left with a spurious horizontal scroll
	// bar.
	it('Switching multi-page then back centers the page.', function() {
		// Enter multi-page view.
		cy.getFrameWindow().then(function(win) {
			win.app.dispatcher.dispatch('multipageview');
			return helper.processToIdle(win);
		});
		cy.getFrameWindow().its('app.activeDocument.activeLayout.type')
			.should('equal', 'ViewLayoutMultiPage');

		// Return to normal view.
		cy.getFrameWindow().then(function(win) {
			win.app.dispatcher.dispatch('multipageview');
			return helper.processToIdle(win);
		});

		// The page sits in the middle: equal space on its left and right. Read
		// through a retried assertion so the check waits for the layout to settle
		// rather than sampling a single mid-relayout frame.
		cy.getFrameWindow().should(function(win) {
			expect(win.app.activeDocument.activeLayout.type)
				.to.equal('ViewLayoutWriter');

			var margins = pageMargins(win);
			expect(Math.abs(margins.left - margins.right), 'left vs right spacing')
				.to.be.lessThan(6);
		});
	});
});
