/* global describe it cy beforeEach require */

var helper = require('../../common/helper');
var desktopHelper = require('../../common/desktop_helper');
var a11yHelper = require('../../common/a11y_helper');

// F6 cycles keyboard focus through the impress regions in this order:
// notebookbar -> slide panel -> document -> sidebar -> status bar, wrapping
// around. Shift+F6 goes the other way.
describe(['tagdesktop'], 'Impress F6 region navigation', function () {
	beforeEach(function () {
		helper.setupAndLoadDocument('impress/empty-placeholder.fodp');
		desktopHelper.switchUIToNotebookbar();

		// the slide panel is shown by default
		cy.cGet('#slide-sorter').should('be.visible');

		// make sure the sidebar is part of the ring
		cy.cGet('#sidebar-dock-wrapper').then(function ($dock) {
			if (!$dock.is(':visible')) desktopHelper.sidebarToggle();
		});
		cy.cGet('#sidebar-dock-wrapper').should('be.visible');

		// let the slide panel and sidebar focus-grabs settle before we anchor
		// focus in the document, so neither can steal it back mid-test
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

	it('F6 walks every region in turn and wraps around', function () {
		cy.realPress('F6');
		a11yHelper.assertFocusWithin('#sidebar-dock-wrapper');

		cy.realPress('F6');
		a11yHelper.assertFocusWithin('#toolbar-down');

		// wraps from the last region back to the notebookbar
		cy.realPress('F6');
		a11yHelper.assertFocusWithin('.notebookbar-tabs-container');

		cy.realPress('F6');
		a11yHelper.assertFocusWithin('#navigation-sidebar');

		cy.realPress('F6');
		assertDocumentFocused();
	});

	it('Shift+F6 steps back from the document into the slide panel', function () {
		cy.realPress(['Shift', 'F6']);
		a11yHelper.assertFocusWithin('#navigation-sidebar');
	});
});
