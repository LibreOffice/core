/* global describe it cy beforeEach require */

var helper = require('../../common/helper');
var calcHelper = require('../../common/calc_helper');

describe(['tagdesktop'], 'Calc focus tests', function() {

	beforeEach(function() {
		helper.setupAndLoadDocument('calc/focus.ods');
		cy.viewport(1920,1080);
	});

	it('Formula-bar focus', function() {
		helper.setDummyClipboardForCopy();
		// Select first cell
		calcHelper.clickOnFirstCell();
		cy.wait(200);

		// Type some text.
		var text1 = 'Hello from Calc';
		calcHelper.typeIntoFormulabar(text1);
		calcHelper.typeIntoFormulabar('{enter}');

		// Unselect formulabar and reselect cell
		calcHelper.clickOnFirstCell();
		cy.wait(200);

		// Check text in formulabar
		calcHelper.typeIntoFormulabar('{ctrl}a');
		helper.copy();
		helper.expectTextForClipboard(text1);
		// Clear selection
		calcHelper.typeIntoFormulabar('{enter}');


		// Type some more text, at the end.
		calcHelper.clickOnFirstCell();
		cy.wait(200);
		var text2 = ', this is a test.';
		calcHelper.typeIntoFormulabar('{end}'+text2);
		calcHelper.typeIntoFormulabar('{enter}');

		// Check text in formulabar
		calcHelper.clickOnFirstCell();
		cy.wait(200);
		calcHelper.typeIntoFormulabar('{ctrl}a');
		helper.copy();
		helper.expectTextForClipboard(text1+text2);
		calcHelper.typeIntoFormulabar('{enter}');
	});

	it('On Rename-sheet modal dialog open', function() {
		cy.cGet('.spreadsheet-tab.spreadsheet-tab-selected').dblclick();
		cy.cGet('#input-modal-input').should('have.focus');
	});

	it('On color palette dialog open', function() {
		cy.cGet('#Home').click();
		cy.cGet('#Home-container .unoBackgroundColor .arrowbackground').click();
		// focus should be on first element which is Automatic color button
		cy.cGet('#transparent-color-button').should('have.focus');
	});

	it('On Tabcontrol dialog open', function() {
		cy.cGet('#Layout-tab-label').click();
		cy.cGet('#layout-page-more-button').filter(':visible').click();
		// focus should be on selected tab if current dialog is tabcontrol dialog
		cy.cGet('.ui-tab.jsdialog.selected').should('have.focus');
	});

});
