/* global describe it cy beforeEach require */

var helper = require('../../common/helper');
var mobileHelper = require('../../common/mobile_helper');
var calcHelper = require('../../common/calc_helper');

describe(['tagmobile'], 'Paste special.', function() {

	beforeEach(function() {
		helper.setupAndLoadDocument('calc/hamburger_menu.ods');
		mobileHelper.enableEditingMobile();
	});

	it('Paste own cells.', function() {
		calcHelper.clickOnFirstCell();
		cy.getFrameWindow().then(function(win) {
			win.app.map.sendUnoCommand('.uno:Copy');
			helper.processToIdle(win);
		});

		cy.cGet('#document-canvas').trigger('contextmenu', 'center');
		cy.cGet('#mobile-wizard-content').should('be.visible');
		cy.cGet('body').contains('.context-menu-link', 'Paste Special').click();

		// no clipboard access under cypress, confirm the paste from this document
		cy.cGet('#modal-dialog-paste_special_dialog-box-yesbutton').click();

		cy.cGet('#mobile-wizard-title').should('have.text', 'Paste Special');
		cy.cGet('#PasteSpecial #paste_values_only-button').should('be.visible');
		cy.cGet('#PasteSpecial input#numbers').should('be.checked');
		cy.cGet('#PasteSpecial input#text').should('be.checked');
		cy.cGet('#PasteSpecial #ok-button').click();

		cy.cGet('#mobile-wizard').should('not.be.visible');
		cy.cGet('#formulabar').should('contain.text', 'Textx');
	});
});
