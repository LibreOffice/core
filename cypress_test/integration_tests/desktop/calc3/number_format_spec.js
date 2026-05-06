/* global describe it cy require beforeEach */

var helper = require('../../common/helper');
var desktopHelper = require('../../common/desktop_helper');

describe(['tagdesktop'], 'Number format Tests', function() {

	beforeEach(function() {
		helper.setupAndLoadDocument('calc/number_format.ods');
		desktopHelper.switchUIToNotebookbar();
		cy.viewport(1920,1080);
		helper.setDummyClipboardForCopy();
	});

	it('Use currency dropdown in notebookbar', function() {
		cy.cGet('.jsdialog-overlay .modalpopup').should('not.exist');

		// go to A3 and reset content
		helper.typeIntoInputField(helper.addressInputSelector, 'A3');
		helper.typeIntoDocument('100');
		cy.cGet(helper.addressInputSelector).should('have.value', 'A3');
		cy.cGet('.notebookbar .unoNumberFormatCurrency').should('not.have.class', 'selected');

		// set USD currency
		cy.cGet('.notebookbar .unoNumberFormatCurrency .arrowbackground').filter(':visible').click();
		cy.cGet('.jsdialog-overlay .modalpopup').should('be.visible');
		cy.cGet('.jsdialog-overlay .modalpopup .ui-treeview-cell-text').contains('USA').click();
		cy.cGet('.jsdialog-overlay .modalpopup').should('not.exist');

		cy.cGet(helper.addressInputSelector).should('have.value', 'A3');
		helper.copy();
		cy.cGet('#copy-paste-container table td').should('have.text', '$100.00');
		cy.cGet('.notebookbar .unoNumberFormatCurrency').should('have.class', 'selected');

		// set GBP currency
		cy.cGet('.notebookbar .unoNumberFormatCurrency .arrowbackground').filter(':visible').click();
		cy.cGet('.jsdialog-overlay .modalpopup').should('be.visible');
		cy.cGet('.jsdialog-overlay .modalpopup .ui-treeview-cell-text').contains('UK').click();
		cy.cGet('.jsdialog-overlay .modalpopup').should('not.exist');

		cy.cGet(helper.addressInputSelector).should('have.value', 'A3');
		helper.copy();
		cy.cGet('#copy-paste-container table td').should('have.text', '£100.00');
		cy.cGet('.notebookbar .unoNumberFormatCurrency').should('have.class', 'selected');

		// turn off currency
		cy.cGet('.notebookbar .unoNumberFormatCurrency').filter(':visible').click();
		helper.copy();
		cy.cGet('#copy-paste-container table td').should('have.text', '100');
		cy.cGet('.notebookbar .unoNumberFormatCurrency').should('not.have.class', 'selected');
	});
});
