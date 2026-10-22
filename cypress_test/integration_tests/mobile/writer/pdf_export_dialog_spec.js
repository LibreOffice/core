/* global describe it cy beforeEach require */

var helper = require('../../common/helper');
var mobileHelper = require('../../common/mobile_helper');

describe(['tagmobile', 'tagnextcloud', 'tagproxy'], 'PDF export dialog.', function() {

	beforeEach(function() {
		helper.setupAndLoadDocument('writer/hamburger_menu.odt');
		mobileHelper.enableEditingMobile();
		mobileHelper.selectHamburgerMenuItem(['Download as', 'PDF Document (.pdf) as...']);
		cy.cGet('.mobile-wizard #PdfOptionsDialog').should('exist');
	});

	it('Show the General page full screen', function() {
		cy.cGet('#mobile-wizard').invoke('outerHeight').should('be.gt', 500);
		cy.cGet('.mobile-wizard .ui-tab').should('have.length', 1);
		cy.cGet('.mobile-wizard .ui-tab').should('have.text', 'General');
		cy.cGet('.mobile-wizard #all.radiobutton').should('be.visible');
		cy.cGet('.mobile-wizard #losslesscompress.radiobutton').should('be.visible');
		cy.cGet('.mobile-wizard #tagged.checkbutton').scrollIntoView().should('be.visible');
		cy.cGet('.mobile-wizard #sheetrange.radiobutton').should('not.be.visible');
	});

	it('Close the dialog with the dismiss control', function() {
		cy.cGet('#mobile-wizard-dialog-dismiss').click();
		cy.cGet('#mobile-wizard').should('not.be.visible');
		mobileHelper.selectHamburgerMenuItem(['Download as', 'PDF Document (.pdf) as...']);
		cy.cGet('.mobile-wizard #PdfOptionsDialog').should('exist');
	});

	it('Export the PDF', function() {
		cy.cGet('.mobile-wizard #PdfOptionsDialog #ok.ui-pushbutton-wrapper').click();
		cy.cGet('iframe').should('have.attr', 'data-src').should('contain', 'download');
		cy.cGet('#mobile-wizard').should('not.be.visible');
	});
});
