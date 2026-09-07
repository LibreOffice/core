/* global describe before beforeEach it cy expect require */

var helper = require('../../common/helper');
var desktopHelper = require('../../common/desktop_helper');
var a11yHelper = require('../../common/a11y_helper');

// The checks are the ones the property deck runs, read from the DOM.
describe(['tagdesktop'], 'Impress shapes deck keyboard navigation',
	{ testIsolation: false }, function () {
	let win;

	function getWin() {
		return win;
	}

	before(function () {
		cy.viewport(1920, 1080);
		helper.setupAndLoadDocument('impress/help_dialog.odp');

		cy.getFrameWindow().then(function (frameWindow) {
			win = frameWindow;
		});

		desktopHelper.switchUIToNotebookbar();
		cy.cGet('.notebookbar-tabs-container').should('be.visible');

		// The View tab entry is the only way the interface offers to this deck.
		cy.cGet('#View-tab-label').click();
		cy.cGet('#View-container').should('be.visible');
		cy.cGet('#View-container [modelId="view-shapes-deck"] button').click();

		cy.cGet('#sidebar-dock-wrapper').should('be.visible');
		cy.cGet('#DefaultShapesPanel').should('exist');
		// An entry redrawn while the focus walks would move the ground under it.
		cy.cGet('#DefaultShapesPanel .ui-iconview-entry img').should('exist');

		cy.then(function () {
			helper.waitUntilLayoutingIsIdle(win);
		});
	});

	beforeEach(function () {
		helper.typeIntoDocument('{esc}');
		helper.assertFocus('className', 'clipboard');
	});

	it('every gallery of the deck is one tab stop', function () {
		cy.then(function () {
			const panel = win.document.querySelector('#DefaultShapesPanel');
			const galleries = panel.querySelectorAll('.ui-iconview');
			const entries = panel.querySelectorAll('.ui-iconview-entry');
			const tabbable = panel.querySelectorAll('.ui-iconview-entry[tabindex="0"]');

			expect(galleries.length, 'galleries in the panel').to.not.equal(0);
			expect(entries.length, 'shapes across the galleries')
				.to.be.greaterThan(galleries.length);
			// A gallery is one stop, not one per shape: the arrows move inside it.
			expect(tabbable.length, 'shapes that Tab stops on')
				.to.equal(galleries.length);
		});
	});

	it('no widget of the deck forces its own tab position', function () {
		a11yHelper.sidebarKeyboard.assertNoForcedTabPosition(getWin);
	});

	it('the focusable helper reports the tab order of the deck', function () {
		a11yHelper.sidebarKeyboard.assertHelperReportsTabOrder(getWin);
	});

	it('F6 enters the deck on its first widget', function () {
		a11yHelper.sidebarKeyboard.assertRingEntersOnFirstWidget(getWin);
	});

	it('Tab walks the deck in order and Shift+Tab comes back', function () {
		a11yHelper.sidebarKeyboard.assertTabWalksTheDeck(getWin);
	});

	it('F6 leaves the deck from a widget inside it', function () {
		a11yHelper.sidebarKeyboard.assertRingLeavesTheDeck(getWin);
	});
});
