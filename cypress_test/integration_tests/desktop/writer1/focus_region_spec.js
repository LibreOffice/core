/* global describe it cy beforeEach require */

var helper = require('../../common/helper');
var desktopHelper = require('../../common/desktop_helper');
var a11yHelper = require('../../common/a11y_helper');

// F6 cycles keyboard focus through the writer regions: notebookbar ->
// document -> sidebar -> status bar, wrapping around.
describe(['tagdesktop'], 'Writer F6 region navigation', function () {
	beforeEach(function () {
		helper.setupAndLoadDocument('writer/focus.odt');
		desktopHelper.switchUIToNotebookbar();

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

		// start with the keyboard focus in the document
		helper.typeIntoDocument('{esc}');
		assertDocumentFocused();
	});

	function assertDocumentFocused() {
		helper.assertFocus('className', 'clipboard');
	}

	it('F6 walks notebookbar, document, sidebar and status bar', function () {
		cy.realPress('F6');
		a11yHelper.assertFocusWithin('#sidebar-dock-wrapper');

		cy.realPress('F6');
		a11yHelper.assertFocusWithin('#toolbar-down');

		// wraps from the status bar back to the notebookbar
		cy.realPress('F6');
		a11yHelper.assertFocusWithin('.notebookbar-tabs-container');

		cy.realPress('F6');
		assertDocumentFocused();
	});
});
