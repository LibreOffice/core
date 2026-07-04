/* global describe it cy beforeEach require */

var helper = require('../../common/helper');
var desktopHelper = require('../../common/desktop_helper');
var calcHelper = require('../../common/calc_helper');
var a11yHelper = require('../../common/a11y_helper');

// F6 cycles keyboard focus through the spreadsheet regions in this order:
// notebookbar -> formula bar -> sheet grid -> sidebar -> sheet-name tabs ->
// status bar, wrapping around. Shift+F6 goes the other way.
describe(['tagdesktop'], 'Calc F6 region navigation', function () {
	beforeEach(function () {
		helper.setupAndLoadDocument('calc/focus.ods');
		desktopHelper.switchUIToNotebookbar();

		// make sure the sidebar is part of the ring
		cy.cGet('#sidebar-dock-wrapper').then(function ($dock) {
			if (!$dock.is(':visible')) desktopHelper.sidebarToggle();
		});
		cy.cGet('#sidebar-dock-wrapper').should('be.visible');

		// let the sidebar's own focus-grab settle before we anchor focus
		// elsewhere, so it cannot steal it back mid-test
		cy.getFrameWindow().then(function (win) {
			helper.waitUntilLayoutingIsIdle(win);
			helper.waitForTimers(win, 'sidebarstealfocus');
		});

		// start with the keyboard focus in the sheet grid
		calcHelper.clickOnFirstCell();
		assertGridFocused();
	});

	function assertGridFocused() {
		helper.assertFocus('className', 'clipboard');
	}

	function assertFormulaBarFocused() {
		cy.cGet('#sc_input_window').should('have.class', 'focused');
	}

	function assertSheetTabFocused() {
		cy.cGet().its('activeElement.className').should('contain', 'spreadsheet-tab');
	}

	it('F6 walks every region in turn and wraps around', function () {
		cy.realPress('F6');
		a11yHelper.assertFocusWithin('#sidebar-dock-wrapper');

		cy.realPress('F6');
		assertSheetTabFocused();

		cy.realPress('F6');
		a11yHelper.assertFocusWithin('#toolbar-down');

		// wraps from the last region back to the notebookbar
		cy.realPress('F6');
		a11yHelper.assertFocusWithin('.notebookbar-tabs-container');

		cy.realPress('F6');
		assertFormulaBarFocused();

		cy.realPress('F6');
		assertGridFocused();
	});

	it('Shift+F6 walks the regions in reverse', function () {
		cy.realPress(['Shift', 'F6']);
		assertFormulaBarFocused();

		cy.realPress(['Shift', 'F6']);
		a11yHelper.assertFocusWithin('.notebookbar-tabs-container');

		// wraps from the notebookbar back to the last region
		cy.realPress(['Shift', 'F6']);
		a11yHelper.assertFocusWithin('#toolbar-down');

		cy.realPress(['Shift', 'F6']);
		assertSheetTabFocused();

		cy.realPress(['Shift', 'F6']);
		a11yHelper.assertFocusWithin('#sidebar-dock-wrapper');

		cy.realPress(['Shift', 'F6']);
		assertGridFocused();
	});
});
