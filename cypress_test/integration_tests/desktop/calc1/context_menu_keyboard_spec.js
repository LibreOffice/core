/* global describe it cy beforeEach require */

var helper = require('../../common/helper');
var calcHelper = require('../../common/calc_helper');
var a11yHelper = require('../../common/a11y_helper');

// Opening the context menu from the keyboard must place it at the active cell
// and move keyboard focus into it, so it can be driven without the mouse.
describe(['tagdesktop'], 'Calc keyboard context menu', function () {
	beforeEach(function () {
		helper.setupAndLoadDocument('calc/context_menu.ods');

		// start with the keyboard focus in the sheet grid on a known cell
		calcHelper.clickOnFirstCell();
		helper.assertFocus('className', 'clipboard');
	});

	it('Shift+F10 opens the cell context menu and focuses its first entry', function () {
		cy.realPress(['Shift', 'F10']);

		// the full cell menu, not the shorter edit menu
		helper.getContextMenuItemList().its('length').should('be.greaterThan', 10);
		// focus lands inside the menu instead of staying on the sheet grid
		a11yHelper.assertFocusWithin('#jsd-context-menu-dropdown-overlay');
	});

	it('Arrow keys move focus between entries within the menu', function () {
		cy.realPress(['Shift', 'F10']);
		a11yHelper.assertFocusWithin('#jsd-context-menu-dropdown-overlay');

		// the first entry opens focused; arrowing down lands on a later entry,
		// still inside the menu. The first press confirms the selection on the
		// already-focused entry, the second moves focus on to the next one.
		// The focused entry lives inside the document iframe, so read the
		// active element from there rather than from the top document.
		cy.cGet().its('activeElement.textContent').then(function (firstEntryText) {
			cy.realPress('ArrowDown');
			cy.realPress('ArrowDown');
			a11yHelper.assertFocusWithin('#jsd-context-menu-dropdown-overlay');
			cy.cGet()
				.its('activeElement.textContent')
				.should('not.equal', firstEntryText);
		});
	});
});
