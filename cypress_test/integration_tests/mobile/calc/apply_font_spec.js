/* global describe it cy beforeEach require */

var helper = require('../../common/helper');
var calcHelper = require('../../common/calc_helper');
var mobileHelper = require('../../common/mobile_helper');

describe(['tagmobile', 'tagnextcloud', 'tagproxy'], 'Apply font changes.', function() {

	beforeEach(function() {
		helper.setupAndLoadDocument('calc/apply_font.ods');

		// Click on edit button
		mobileHelper.enableEditingMobile();

		calcHelper.clickOnFirstCell();

		// Open character properties
		mobileHelper.openTextPropertiesPanel();
	});

	it('Apply bold.', function() {
		helper.setDummyClipboardForCopy();
		cy.cGet('#mobile-wizard .unoBold').click();
		calcHelper.selectEntireSheet();
		helper.copy();
		cy.cGet('#copy-paste-container table td b').should('exist');
	});

	it('Apply italic.', function() {
		helper.setDummyClipboardForCopy();
		cy.cGet('#mobile-wizard .unoItalic').click();
		calcHelper.selectEntireSheet();
		helper.copy();
		cy.cGet('#copy-paste-container table td i').should('exist');
	});

	it('Apply underline.', function() {
		helper.setDummyClipboardForCopy();
		cy.cGet('#mobile-wizard .unoUnderline').click();

		calcHelper.selectEntireSheet();
		helper.copy();

		cy.cGet('#copy-paste-container table td u')
			.should('exist');
	});

	it('Apply strikeout.', function() {
		helper.setDummyClipboardForCopy();
		cy.cGet('#mobile-wizard .unoStrikeout').click();

		calcHelper.selectEntireSheet();
		helper.copy();

		cy.cGet('#copy-paste-container table td s')
			.should('exist');
	});

	it('Apply shadowed.', function() {
		cy.cGet('#mobile-wizard .unoShadowed').click();

		calcHelper.selectEntireSheet();

		// TODO: Shadowed is not in the clipboard content.
	});

	it('Apply font name.', function() {
		helper.setDummyClipboardForCopy();
		cy.cGet('#TextPropertyPanel, .TextPropertyPanel').click();
		cy.cGet('#fontnamecombobox').click();
		cy.cGet('#fontnamecombobox').contains('.mobile-wizard.ui-combobox-text', 'Linux Libertine G').click();

		calcHelper.selectEntireSheet();
		helper.copy();

		cy.cGet('#copy-paste-container table td font')
			.should('have.attr', 'face', 'Linux Libertine G');
	});

	it('Apply font size.', function() {
		helper.setDummyClipboardForCopy();
		cy.cGet('#TextPropertyPanel, .TextPropertyPanel').click();
		cy.cGet('#fontsizecombobox').click();
		cy.cGet('#fontsizecombobox').contains('.mobile-wizard.ui-combobox-text', '14 pt').click();

		calcHelper.selectEntireSheet();
		helper.copy();

		cy.cGet('#copy-paste-container table td font')
			.should('have.attr', 'size', '4');
	});

	it('Apply font color.', function() {
		helper.setDummyClipboardForCopy();
		cy.cGet('#Color .ui-header').click();

		mobileHelper.selectFromColorPicker('#Color', 5);

		calcHelper.selectEntireSheet();
		helper.copy();

		cy.cGet('#copy-paste-container table td font')
			.should('have.attr', 'color', '#00FF00');
	});
});
