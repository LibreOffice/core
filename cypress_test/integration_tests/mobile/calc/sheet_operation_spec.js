/* global describe it cy beforeEach require */

var helper = require('../../common/helper');
var mobileHelper = require('../../common/mobile_helper');
var calcHelper = require('../../common/calc_helper');

describe(['tagmobile', 'tagnextcloud', 'tagproxy'], 'Sheet Operation', function () {

	beforeEach(function () {
		helper.setupAndLoadDocument('calc/sheet_operation.ods');

		mobileHelper.enableEditingMobile();
	});

	it('Insert sheet', function () {
		calcHelper.assertNumberofSheets(1);

		cy.cGet('#insertsheet').click();

		calcHelper.assertNumberofSheets(2);
	});

	it('Insert sheet before', function () {
		calcHelper.assertNumberofSheets(1);

		cy.cGet('#spreadsheet-tab0').should('have.text', 'Sheet1');

		calcHelper.selectOptionMobileWizard('Insert sheet before this');

		calcHelper.assertNumberofSheets(2);

		cy.cGet('#spreadsheet-tab0').should('have.text', 'Sheet2');

		cy.cGet('#spreadsheet-tab1').should('have.text', 'Sheet1');
	});

	it('Insert sheet after', function () {
		calcHelper.assertNumberofSheets(1);

		cy.cGet('#spreadsheet-tab0').should('have.text', 'Sheet1');

		calcHelper.selectOptionMobileWizard('Insert sheet after this');

		calcHelper.assertNumberofSheets(2);

		cy.cGet('#spreadsheet-tab0').should('have.text', 'Sheet1');

		cy.cGet('#spreadsheet-tab1').should('have.text', 'Sheet2');
	});

	it('Delete sheet', function () {
		calcHelper.assertNumberofSheets(1);

		cy.cGet('#insertsheet').click();

		calcHelper.assertNumberofSheets(2);

		calcHelper.selectOptionMobileWizard('Delete Sheet...');

		cy.cGet('#delete-sheet-modal-response').click();

		calcHelper.assertNumberofSheets(1);
	});

	it('Rename sheet', function () {
		calcHelper.assertNumberofSheets(1);

		cy.cGet('.spreadsheet-tab.spreadsheet-tab-selected').should('have.text', 'Sheet1');

		calcHelper.selectOptionMobileWizard('Rename Sheet...');

		cy.cGet('#mobile-wizard-content-modal-dialog-rename-calc-sheet').should('exist');
		cy.cGet('#input-modal-input').clear().type('renameSheet');
		cy.cGet('#response-ok').click();
		cy.cGet('.spreadsheet-tab.spreadsheet-tab-selected').should('have.text', 'renameSheet');
	});

	it('Hide/Show sheet', function () {
		calcHelper.assertNumberofSheets(1);

		cy.cGet('#insertsheet').click();

		calcHelper.assertNumberofSheets(2);

		//hide sheet
		calcHelper.selectOptionMobileWizard('Hide Sheet');

		calcHelper.assertNumberofSheets(1);

		//show sheet
		calcHelper.selectOptionMobileWizard('Show Sheet');

		cy.cGet('#mobile-wizard-content-modal-dialog-show-sheets-modal').should('exist');
		cy.cGet('input#hidden-part-checkbox-0').check();
		cy.cGet('#show-sheets-modal-response').click();

		calcHelper.assertNumberofSheets(2);
	});

	it('Move sheet left/right', function () {
		calcHelper.assertNumberofSheets(1);

		cy.cGet('#insertsheet').click();

		calcHelper.assertNumberofSheets(2);

		cy.cGet('#spreadsheet-tab0').should('have.text', 'Sheet1');

		//left
		calcHelper.selectOptionMobileWizard('Move Sheet Left');

		cy.cGet('#spreadsheet-tab0').should('have.text', 'Sheet2');

		//right
		calcHelper.selectOptionMobileWizard('Move Sheet Right');

		cy.cGet('#spreadsheet-tab0').should('have.text', 'Sheet1');
	});
});
