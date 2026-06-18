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
