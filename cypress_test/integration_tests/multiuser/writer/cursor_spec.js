/* global describe it cy beforeEach require expect */

var helper = require('../../common/helper');
var desktopHelper = require('../../common/desktop_helper');
var writerHelper = require('../../common/writer_helper.js');

describe(['tagmultiuser'], 'Check cursor and view behavior', function() {

	beforeEach(function() {
		helper.setupAndLoadDocument('writer/cursor_jump.odt', true);
		desktopHelper.switchUIToNotebookbar();
	});

	it('Show user name on mouse hover over other view cursor', function() {
		// Given a view cursor for the second iframe which is currently not visible:
		// Move cursor in first view to make sure it was shown already once in the second
		// view.
		cy.cSetActiveFrame('#iframe1');
		helper.typeIntoDocument('{rightArrow}');

		cy.cSetActiveFrame('#iframe2');
		// Wait for the cursor header to appear (shown when cursor moves), then
		// wait for it to auto-hide.
		cy.cGet('#canvas-container .cursor-header-section').should('exist');
		cy.cGet('#canvas-container .cursor-header-section').should('not.exist');

		// When moving the mouse over the view cursor in the second view:
		cy.getFrameWindow().then((win) => {
			const cursorSections = win.app.sectionContainer.sections.filter(
				(s) => s.name.startsWith('OtherViewCursor ')
			);
			expect(cursorSections.length).to.be.greaterThan(0);

			const cursorSection = cursorSections[0];
			// Get the cursor's position in CSS pixels relative to canvas.
			const x = cursorSection.myTopLeft[0] / win.app.dpiScale;
			const y = cursorSection.myTopLeft[1] / win.app.dpiScale;

			// Get canvas bounding rect to calculate viewport-relative coordinates.
			const canvas = win.document.getElementById('document-canvas');
			const rect = canvas.getBoundingClientRect();

			// Dispatch mouse events manually, which work on the canvas.
			const mouseEnterEvent = new win.MouseEvent('mouseenter', {
				clientX: rect.left + x,
				clientY: rect.top + y,
				bubbles: true,
				cancelable: true,
				view: win
			});
			canvas.dispatchEvent(mouseEnterEvent);
			const mouseMoveEvent = new win.MouseEvent('mousemove', {
				clientX: rect.left + x,
				clientY: rect.top + y,
				bubbles: true,
				cancelable: true,
				view: win
			});
			canvas.dispatchEvent(mouseMoveEvent);
		});

		// Then make sure that the cursor header appears on mouse enter:
		// Without the accompanying fix in place, this test would have failed with:
		// Timed out retrying after 10000ms: Expected to find element: `#canvas-container
		// .cursor-header-section`, but never found it.
		cy.cGet('#canvas-container .cursor-header-section').should('exist');
	});

	it('Do not center the view if cursor is already visible', function() {
		// second view follow the first one
		cy.cSetActiveFrame('#iframe2');
		cy.cGet('#userListHeader').click();
		cy.cGet('.user-list-item').eq(1).click();
		cy.cGet('.jsdialog-overlay').should('not.exist');
		desktopHelper.assertScrollbarPosition('vertical', 0, 30);

		// first view goes somewhere down
		cy.cSetActiveFrame('#iframe1');
		writerHelper.openQuickFind();
		writerHelper.searchInQuickFind('Pellentesque porttitor');
		desktopHelper.assertScrollbarPosition('vertical', 375, 400);

		// verify that second view is scrolled to the editor
		cy.cSetActiveFrame('#iframe2');
		desktopHelper.assertScrollbarPosition('vertical', 375, 400);

		// now move cursor a bit in the first view
		cy.cSetActiveFrame('#iframe1');
		helper.typeIntoDocument('{downArrow}{downArrow}{downArrow}{downArrow}{downArrow}{downArrow}');

		// verify that second view is still at the same position (no jump)
		cy.cSetActiveFrame('#iframe2');
		desktopHelper.assertScrollbarPosition('vertical', 375, 400);
	});

	it('Follow the editor cursor in multi-page view', function() {
		cy.viewport(1920, 1080);

		// Second view switches to multi-page view and follows the first one.
		cy.cSetActiveFrame('#iframe2');
		cy.getFrameWindow().then(function(win) {
			win.app.dispatcher.dispatch('multipageview');
		});
		cy.getFrameWindow().then(function(win) {
			return helper.processToIdle(win);
		});
		cy.getFrameWindow().then(function(win) {
			expect(win.app.activeDocument.activeLayout.type).to.equal('ViewLayoutMultiPage');
		});

		cy.cGet('#userListHeader').click();
		cy.cGet('.user-list-item').eq(1).click();
		cy.cGet('.jsdialog-overlay').should('not.exist');

		// First view jumps far down the document, moving its cursor off the
		// pages the second view is currently showing.
		cy.cSetActiveFrame('#iframe1');
		writerHelper.openQuickFind();
		writerHelper.searchInQuickFind('Pellentesque porttitor');

		// The second view must scroll so the followed cursor becomes visible.
		// Without the fix, scrollToPos handed the multi-page layout a point half
		// a viewport away from the cursor, so scrollTo() either targeted the
		// wrong page or hit its "already visible" guard and did nothing - the
		// followed cursor stayed off-screen.
		cy.cSetActiveFrame('#iframe2');
		cy.getFrameWindow().then(function(win) {
			return helper.processToIdle(win);
		});
		cy.getFrameWindow().then(function(win) {
			cy.wrap(null).should(function() {
				var sections = win.app.sectionContainer.sections.filter(
					function(s) { return s.name.startsWith('OtherViewCursor '); });
				expect(sections.length, 'followed view cursor exists').to.be.greaterThan(0);

				// Same document point and visibility check that goToSection() uses
				// to decide whether following needs to scroll.
				var section = sections[0];
				var cursorTwips = [
					section.position[0] * win.app.pixelsToTwips,
					section.position[1] * win.app.pixelsToTwips,
				];
				expect(
					win.app.isPointVisibleInTheDisplayedArea(cursorTwips),
					'followed cursor is visible in multi-page view'
				).to.be.true;
			});
		});
	});
});

describe(['tagmultiuser'], 'Keep the view fixed while another view edits', function() {

	beforeEach(function() {
		// Give each iframe a distinct user id so their per-user settings stay
		// separate and one user's view state does not leak into the other.
		helper.setupAndLoadDocument('writer/cursor_jump.odt', true, false, undefined,
			'userid1=1&userid2=2');
		desktopHelper.switchUIToNotebookbar();
	});

	it('Move a view down for edits above it, but leave it still for edits below', function() {
		// Scroll offsets carried between the queued command callbacks below.
		const before = {};

		// The document is already several pages long. The first view keeps its
		// caret at the very start, and the second view sends its caret to the
		// very end, so the two views look at opposite ends of the document.
		cy.cSetActiveFrame('#iframe1');
		helper.typeIntoDocument('{ctrl}{home}');

		cy.cSetActiveFrame('#iframe2');
		helper.typeIntoDocument('{ctrl}{end}');
		cy.getFrameWindow().then(function(win) {
			return helper.processToIdle(win);
		});
		cy.getFrameWindow().then(function(win) {
			before.secondViewY = win.app.activeDocument.activeLayout.viewedRectangle.pY1;
		});

		// The first view inserts several paragraphs at the very start. That
		// reflow pushes the second view's caret, at the end, further down the
		// document.
		cy.cSetActiveFrame('#iframe1');
		for (let i = 0; i < 8; i++)
			helper.typeIntoDocument('{enter}');

		// The second view scrolls down to keep its caret at the same spot on
		// screen, so its scroll offset grows. Without the fix the view would
		// stay put and the caret would drift up off the bottom.
		cy.cSetActiveFrame('#iframe2');
		cy.getFrameWindow().then(function(win) {
			return helper.processToIdle(win);
		});
		cy.getFrameWindow().then(function(win) {
			cy.wrap(null).should(function() {
				const secondViewY = win.app.activeDocument.activeLayout.viewedRectangle.pY1;
				expect(secondViewY, 'second view scrolled down to follow the reflow above it')
					.to.be.greaterThan(before.secondViewY);
			});
		});

		// Now record where the first view, still at the top, is scrolled to.
		cy.cSetActiveFrame('#iframe1');
		cy.getFrameWindow().then(function(win) {
			return helper.processToIdle(win);
		});
		cy.getFrameWindow().then(function(win) {
			before.firstViewY = win.app.activeDocument.activeLayout.viewedRectangle.pY1;
		});

		// The second view inserts a paragraph at the very end, below everything
		// the first view can see.
		cy.cSetActiveFrame('#iframe2');
		helper.typeIntoDocument('{enter}');

		// An edit below the first view does not move its caret, so its view
		// stays exactly where it was.
		cy.cSetActiveFrame('#iframe1');
		cy.getFrameWindow().then(function(win) {
			return helper.processToIdle(win);
		});
		cy.getFrameWindow().then(function(win) {
			cy.wrap(null).should(function() {
				const firstViewY = win.app.activeDocument.activeLayout.viewedRectangle.pY1;
				expect(firstViewY, 'first view stays put for an edit below it')
					.to.be.closeTo(before.firstViewY, 1);
			});
		});
	});
});
