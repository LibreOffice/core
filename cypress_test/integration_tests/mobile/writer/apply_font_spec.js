/* global describe it cy beforeEach require */

var helper = require('../../common/helper');
var mobileHelper = require('../../common/mobile_helper');
var writerHelper = require('../../common/writer_helper');

describe(['tagmobile', 'tagnextcloud', 'tagproxy'], 'Apply font changes.', function() {

	beforeEach(function() {
		helper.setupAndLoadDocument('writer/apply_font.odt');
		mobileHelper.enableEditingMobile();
		writerHelper.selectAllTextOfDoc();
		mobileHelper.openMobileWizard();
	});

	it('Apply font name.', function() {
		helper.setDummyClipboardForCopy();

		cy.cGet('#fontnamecombobox .ui-header-right .entry-value')
			.should('not.have.text', 'Linux Libertine G');

		cy.cGet('#fontnamecombobox').click();
		cy.cGet('#font').contains('.mobile-wizard.ui-combobox-text', 'Linux Libertine G').click();

		cy.cGet('#fontnamecombobox .ui-header-right .entry-value')
			.should('have.text', 'Linux Libertine G');

		writerHelper.selectAllTextOfDoc();
		helper.copy();
		cy.cGet('#copy-paste-container p font').should('have.attr', 'face', 'Linux Libertine G');
	});

	it('Apply font size.', function() {
		helper.setDummyClipboardForCopy();
		cy.cGet('#fontsizecombobox').click();
		cy.cGet('#fontsizecombobox').contains('.mobile-wizard.ui-combobox-text', '36 pt').click();
		writerHelper.selectAllTextOfDoc();
		helper.copy();
		cy.cGet('#copy-paste-container p font').should('have.attr', 'style', 'font-size: 36pt');
	});

	it('Apply bold font.', function() {
		helper.setDummyClipboardForCopy();
		cy.cGet('#mobile-wizard .unoBold').scrollIntoView().click();
		writerHelper.selectAllTextOfDoc();
		helper.copy();
		cy.cGet('#copy-paste-container p b').should('exist');
	});

	it('Apply italic font.', function() {
		helper.setDummyClipboardForCopy();
		cy.cGet('#mobile-wizard .unoItalic').scrollIntoView().click();
		writerHelper.selectAllTextOfDoc();
		helper.copy();
		cy.cGet('#copy-paste-container p i').should('exist');
	});

	it('Apply underline.', function() {
		helper.setDummyClipboardForCopy();
		cy.cGet('#mobile-wizard .unoUnderline').scrollIntoView().click();
		writerHelper.selectAllTextOfDoc();
		helper.copy();
		cy.cGet('#copy-paste-container p u').should('exist');
	});

	it('Apply strikeout.', function() {
		helper.setDummyClipboardForCopy();
		cy.cGet('.unoStrikeout').click();
		writerHelper.selectAllTextOfDoc();
		helper.copy();
		cy.cGet('#copy-paste-container p strike').should('exist');
	});

	it('Apply shadowed.', function() {
		cy.cGet('.unoShadowed').click();
		writerHelper.selectAllTextOfDoc();
		// TODO: Shadowed is not in the clipboard content.
	});

	it('Apply font color.', function() {
		helper.setDummyClipboardForCopy();
		cy.cGet('#Color').contains('.ui-header','Font Color').click();
		cy.cGet('#Color [id$=-basic-color-5]').click();
		writerHelper.selectAllTextOfDoc();
		helper.copy();
		cy.cGet('#copy-paste-container p font').should('have.attr', 'color', '#00ff00');
	});

	it('Apply automatic font color.', function() {
		helper.setDummyClipboardForCopy();
		cy.cGet('#Color').contains('.ui-header','Font Color').click();
		cy.cGet('#Color [id$=-basic-color-2]').click();
		mobileHelper.closeMobileWizard();
		writerHelper.selectAllTextOfDoc();
		helper.copy();
		cy.cGet('#copy-paste-container p font').should('have.attr', 'color', '#ff0000');
		mobileHelper.openMobileWizard();
		cy.cGet('#Color').contains('.ui-header','Font Color').click();
		cy.cGet('.colors-container-auto-color-row:visible').click();
		writerHelper.selectAllTextOfDoc();
		helper.copy();
		cy.cGet('#copy-paste-container p font').should('have.attr', 'color', '#000000');
	});

	it('Apply highlight color.', function() {
		helper.setDummyClipboardForCopy();
		cy.cGet('#CharBackColor .ui-header').click();
		cy.cGet('#CharBackColor [id$=-basic-color-2]').click();
		writerHelper.selectAllTextOfDoc();
		helper.copy();
		cy.cGet('#copy-paste-container p font span').should('have.attr', 'style', 'background: #ff0000');
	});

	it('Apply superscript.', function() {
		helper.setDummyClipboardForCopy();
		cy.cGet('.unoSuperScript').click();
		writerHelper.selectAllTextOfDoc();
		helper.copy();
		cy.cGet('#copy-paste-container p sup').should('exist');
	});

	it('Apply subscript.', function() {
		helper.setDummyClipboardForCopy();
		cy.cGet('.unoSubScript').click();
		writerHelper.selectAllTextOfDoc();
		helper.copy();
		cy.cGet('#copy-paste-container p sub').should('exist');
	});

	it('Character spacing item is hidden.', function() {
		// Check that mobile wizard is opened
		cy.cGet('.unoSubScript').scrollIntoView().should('be.visible');
		// Character spacing item triggers the character dialog
		// So better to hide it.
		cy.cGet('.unoSpacing').should('not.exist');
	});

	it('Apply style.', function() {
		helper.setDummyClipboardForCopy();

		cy.cGet('#applystyle').click();
		cy.cGet('body').contains('#fontstyletoolbox .ui-combobox-text', 'Title').click();

		writerHelper.selectAllTextOfDoc();
		helper.copy();
		cy.cGet('#copy-paste-container p font').should('have.attr', 'face', 'Liberation Sans, sans-serif');
		cy.cGet('#copy-paste-container p font font').should('have.attr', 'style', 'font-size: 28pt');

		// Clear formatting
		mobileHelper.openMobileWizard();
		cy.cGet('#applystyle').click();
		cy.cGet('body').contains('#fontstyletoolbox .ui-combobox-text', 'Clear formatting').click();

		writerHelper.selectAllTextOfDoc();
		helper.copy();
		cy.cGet('#copy-paste-container p font font').should('not.exist');
	});

	it.skip('New style and update style items are hidden.', function() {
		cy.cGet('#applystyle').should('exist');
		cy.cGet('#StyleUpdateByExample').should('not.exist');
		cy.cGet('#StyleNewByExample').should('not.exist');
	});
});

