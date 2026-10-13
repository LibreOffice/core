/* global describe it cy beforeEach require */

var helper = require('../../common/helper');
var mobileHelper = require('../../common/mobile_helper');
var writerHelper = require('../../common/writer_helper');

describe(['tagmobile'], 'Paste special.', function() {

	beforeEach(function() {
		helper.setupAndLoadDocument('writer/hamburger_menu.odt');
		mobileHelper.enableEditingMobile();
	});

	it('Paste as unformatted text.', function() {
		helper.setDummyClipboardForCopy();
		helper.setDummyClipboardForPaste('<p>html content</p>', 'plain text content');

		// replace the misspelled text, it would give the spelling menu
		helper.typeIntoDocument('{ctrl}a');
		helper.typeIntoDocument('hello world');

		cy.cGet('#document-canvas').trigger('contextmenu', 'center');
		cy.cGet('#mobile-wizard-content').should('be.visible');
		cy.cGet('body').contains('.context-menu-link', 'Paste Special').click();

		cy.cGet('#mobile-wizard-title').should('have.text', 'Paste Special');
		cy.cGet('#PasteSpecialDialog .ui-treeview-entry').should('have.length.at.least', 2);
		cy.cGet('#PasteSpecialDialog .ui-treeview-cell-text:contains("HTML")').should('exist');
		cy.cGet('#PasteSpecialDialog .ui-treeview-cell-text:contains("Unformatted text")').click();
		cy.cGet('#PasteSpecialDialog #ok-button').click();

		cy.cGet('#mobile-wizard').should('not.be.visible');
		writerHelper.selectAllTextOfDoc();
		helper.copy();
		cy.cGet('#copy-paste-container p').should('contain.text', 'plain text content');
	});
});
