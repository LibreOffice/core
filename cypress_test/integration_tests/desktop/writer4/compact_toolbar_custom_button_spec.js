/* global describe it cy beforeEach require expect */

var helper = require('../../common/helper');
var desktopHelper = require('../../common/desktop_helper');

describe(['tagdesktop'], 'Compact toolbar custom button', function() {

	beforeEach(function() {
		cy.viewport(1920, 1080);
		helper.setupAndLoadDocument('writer/top_toolbar.odt');
		desktopHelper.switchUIToCompact();
		cy.cGet('#toolbar-up').should('be.visible');
		cy.getFrameWindow().then((win) => {
			this.win = win;
		});
	});

	it('keeps the style list after inserting a custom button', function() {
		helper.processToIdle(this.win);

		cy.cGet('#styles .ui-combobox-button').click();
		cy.cGet('[id^="styles-dropdown"].modalpopup .ui-combobox-entry')
			.should('have.length.greaterThan', 1);
		cy.cGet('[id^="styles-dropdown"].modalpopup')
			.should('contain.text', 'Default Paragraph Style');

		let entryCountBefore = 0;
		cy.cGet('[id^="styles-dropdown"].modalpopup .ui-combobox-entry')
			.then(($entries) => { entryCountBefore = $entries.length; });

		cy.cGet('#styles-dropdown-overlay').click();
		cy.cGet('[id^="styles-dropdown"].modalpopup').should('not.exist');

		// Insert a custom button the way a WOPI host does
		cy.then(() => {
			this.win.app.map.uiManager.insertButton({
				id: 'CypressCustomButton',
				hint: 'Cypress Custom Button',
				unoCommand: '.uno:Bold',
			});
		});

		cy.cGet('#toolbar-up .w2ui-icon.CypressCustomButton').should('exist');

		helper.processToIdle(this.win);

		// The style list still has its entries after the rebuild.
		cy.cGet('#styles .ui-combobox-button').click();
		cy.cGet('[id^="styles-dropdown"].modalpopup .ui-combobox-entry')
			.should('have.length.greaterThan', 1);
		cy.cGet('[id^="styles-dropdown"].modalpopup')
			.should('contain.text', 'Default Paragraph Style');

		cy.cGet('[id^="styles-dropdown"].modalpopup .ui-combobox-entry')
			.then(($entries) => {
				expect($entries.length).to.equal(entryCountBefore);
			});
	});
});
