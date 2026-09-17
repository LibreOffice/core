/* global describe it cy beforeEach require expect */

var helper = require('../../common/helper');
var desktopHelper = require('../../common/desktop_helper');
var writerHelper = require('../../common/writer_helper.js');

// The tests below jump to the far end of the eight page document right after
// the load. Each view has to know every page first, or the jump stops at the
// last page the view has heard of.
function waitForAllPages(frames) {
	frames.forEach(function(frame) {
		cy.cSetActiveFrame(frame);
		writerHelper.waitForPageCount(8);
	});
}

describe(['tagmultiuser'], 'Check cursor and view behavior', function() {

	beforeEach(function() {
		helper.setupAndLoadDocument('writer/cursor_jump.odt', true);
		waitForAllPages(['#iframe1', '#iframe2']);
		desktopHelper.switchUIToNotebookbar();
	});

	it.skip('Show user name on mouse hover over other view cursor', function() {
		// Given a view cursor for the second iframe which is currently not visible:
		// Move cursor in first view to make sure it was shown already once in the second
		// view.
		cy.cSetActiveFrame('#iframe1');
		helper.typeIntoDocument('{rightArrow}');

		cy.cSetActiveFrame('#iframe2');
		// The hover shows the name, so the section has to carry one: the cursor
		// arrives from invalidateviewcursor while the name comes with the view
		// info, and onMouseEnter returns early until it does.
		cy.getFrameWindow().should((win) => {
			const sections = win.app.sectionContainer.sections.filter(
				(s) => s.name.startsWith('OtherViewCursor ')
			);
			expect(sections.length).to.be.greaterThan(0);
			expect(sections[0].sectionProperties.username || '').to.not.equal('');
		});
		cy.cGet('#canvas-container .cursor-header-section').should('not.exist');

		// When moving the mouse over the view cursor in the second view:
		cy.getFrameWindow().then((win) => {
			const cursorSections = win.app.sectionContainer.sections.filter(
				(s) => s.name.startsWith('OtherViewCursor ')
			);
			expect(cursorSections.length).to.be.greaterThan(0);

			const cursorSection = cursorSections[0];
			// Aim at the middle of the cursor in CSS pixels. It is two device
			// pixels wide, so a corner survives no rounding in the round trip
			// through CSS pixels and back.
			const x = (cursorSection.myTopLeft[0] + cursorSection.size[0] / 2) / win.app.dpiScale;
			const y = (cursorSection.myTopLeft[1] + cursorSection.size[1] / 2) / win.app.dpiScale;

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
		desktopHelper.assertScrollbarPosition('vertical', 390, 430);

		// verify that second view is scrolled to the editor
		cy.cSetActiveFrame('#iframe2');
		desktopHelper.assertScrollbarPosition('vertical', 390, 430);

		// now move cursor a bit in the first view
		cy.cSetActiveFrame('#iframe1');
		helper.typeIntoDocument('{downArrow}{downArrow}{downArrow}{downArrow}{downArrow}{downArrow}');

		// verify that second view is still at the same position (no jump)
		cy.cSetActiveFrame('#iframe2');
		desktopHelper.assertScrollbarPosition('vertical', 390, 430);
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
		waitForAllPages(['#iframe1', '#iframe2']);
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

	it('Leave a multi-page view still while another view edits above it', function() {
		// Two pages fit side by side at this width.
		cy.viewport(1920, 1080);

		// Scroll offset and caret position carried between the queued command
		// callbacks below.
		const before = {};

		// The first view keeps its caret at the very start.
		cy.cSetActiveFrame('#iframe1');
		helper.typeIntoDocument('{ctrl}{home}');

		// The second view sends its caret to the very end and then switches to
		// the multi-page view, which starts at the first page.
		cy.cSetActiveFrame('#iframe2');
		helper.typeIntoDocument('{ctrl}{end}');
		cy.getFrameWindow().then(function(win) {
			win.app.dispatcher.dispatch('multipageview');
		});
		cy.getFrameWindow().then(function(win) {
			return helper.processToIdle(win);
		});
		cy.getFrameWindow().then(function(win) {
			expect(win.app.activeDocument.activeLayout.type).to.equal('ViewLayoutMultiPage');
			before.secondViewY = win.app.activeDocument.activeLayout.scrollProperties.viewY;
			before.secondViewCaretY = win.app.file.textCursor.rectangle.pY1;
		});

		// The first view inserts several paragraphs at the very start. That
		// reflow pushes the second view's caret, at the end, further down the
		// document.
		cy.cSetActiveFrame('#iframe1');
		for (let i = 0; i < 8; i++)
			helper.typeIntoDocument('{enter}');

		cy.cSetActiveFrame('#iframe2');
		cy.getFrameWindow().then(function(win) {
			return helper.processToIdle(win);
		});

		// Wait for the reflow to reach the second view: its own caret is the
		// thing that moves down.
		cy.getFrameWindow().then(function(win) {
			cy.wrap(null).should(function() {
				expect(win.app.file.textCursor.rectangle.pY1,
					'the edit above moved the second view caret down')
					.to.be.greaterThan(before.secondViewCaretY);
			});
		});

		// The pages keep their slots on screen, so the multi-page view shows
		// the same pages at the same place as before the edit.
		cy.getFrameWindow().then(function(win) {
			cy.wrap(null).should(function() {
				expect(win.app.activeDocument.activeLayout.scrollProperties.viewY,
					'multi-page view stays put while another view edits above it')
					.to.equal(before.secondViewY);
			});
		});
	});
});

describe(['tagmultiuser'], 'A user with two connections', function() {

	beforeEach(function() {
		// One user opens the document in iframe2 and in iframe3, so that user has
		// two connections. iframe1 is somebody else and does the following.
		helper.setupAndLoadDocument('writer/cursor_jump.odt', true, false, undefined,
			'userid1=test&userid2=test2&userid3=test2');

		cy.cSetActiveFrame('#iframe3');
		helper.documentChecks(true);
		waitForAllPages(['#iframe1', '#iframe2', '#iframe3']);

		cy.cSetActiveFrame('#iframe1');
		desktopHelper.switchUIToNotebookbar();
	});

	// Moves the caret of the active frame and gives back where it landed in
	// caret.y, for the assertion that follows.
	function moveCaret(keys, caret) {
		helper.typeIntoDocument(keys);
		cy.getFrameWindow().then(function(win) {
			return helper.processToIdle(win);
		});
		cy.getFrameWindow().then(function(win) {
			caret.y = win.app.file.textCursor.rectangle.y1;
		});
	}

	// The follower ends up looking at the place the other user works in.
	function assertFollowerShows(caret) {
		cy.cSetActiveFrame('#iframe1');
		cy.getFrameWindow().should(function(win) {
			const viewed = win.app.activeDocument.activeLayout.viewedRectangle;
			expect(viewed.y1, 'top of the followed area').to.be.at.most(caret.y);
			expect(viewed.y2, 'bottom of the followed area').to.be.at.least(caret.y);
		});
	}

	it('Has one entry in the list and is followed in whichever connection it works', function() {
		// Three views are open, and the two that belong to one user share an
		// entry, so the list and the avatars in the header show two users.
		cy.getFrameWindow().should(function(win) {
			expect(Object.keys(win.app.map._viewInfo)).to.have.length(3);
		});

		cy.cGet('#userListHeader').click();
		cy.cGet('.user-list-item').should('have.length', 2);
		cy.cGet('#userListSummaryButton img').should('have.length', 2);

		// Follow the user with the two connections.
		cy.cGet('.user-list-item').eq(1).click();
		cy.cGet('#followingChip').should('be.visible');

		// The followed user goes to the end of the document in the first of
		// their connections.
		const endOfDocument = {};
		cy.cSetActiveFrame('#iframe2');
		moveCaret('{ctrl}{end}', endOfDocument);
		assertFollowerShows(endOfDocument);

		// The same user carries on near the start in their other connection.
		const nearTheStart = {};
		cy.cSetActiveFrame('#iframe3');
		moveCaret('{downArrow}{downArrow}{downArrow}', nearTheStart);
		assertFollowerShows(nearTheStart);
	});
});
