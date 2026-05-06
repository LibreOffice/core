/* global describe it cy beforeEach require */

var helper = require('../../common/helper');
var desktopHelper = require('../../common/desktop_helper');

describe(['tagdesktop'], 'Context toolbar tests.', function() {

	beforeEach(function() {
		cy.viewport(1920, 1080);
		helper.setupAndLoadDocument('writer/top_toolbar.odt');
		desktopHelper.switchUIToNotebookbar();
	});

	it('Font name combobox has previews.', function() {
		// Double-click on text to select a word and trigger context toolbar
		helper.getBlinkingCursorPosition('cursorPos');
		helper.clickAt('cursorPos', true);

		// Context toolbar should appear
		cy.cGet('#context-toolbar').should('not.have.class', 'hidden');

		// Open font name combobox in context toolbar
		cy.cGet('#context-toolbar #fontnamecombobox .ui-combobox-button').click();

		// Dropdown should appear with font preview images
		cy.cGet('[id$="-dropdown"].modalpopup').should('be.visible');
		cy.wait(1000); // Wait for custom entry rendering
		cy.cGet('[id$="-dropdown"].modalpopup img').should('have.length.greaterThan', 0);
	});
});
