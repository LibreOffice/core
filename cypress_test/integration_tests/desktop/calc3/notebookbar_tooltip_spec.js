/* global describe it cy beforeEach require */

var helper = require('../../common/helper');
var desktopHelper = require('../../common/desktop_helper');
var calcHelper = require('../../common/calc_helper');

describe(['tagdesktop'], 'Notebookbar tooltip tests (ODS).', function() {

	beforeEach(function() {
		helper.setupAndLoadDocument('calc/top_toolbar.ods');
		desktopHelper.switchUIToNotebookbar();
	});

	it.skip('InsertCalcTable button shows ODS tooltip when disabled', function() {
		cy.cGet('#Insert-tab-label').click();

		cy.cGet('[modelid="insert-insert-calc-table"]')
			.should('have.attr', 'disabled');
		cy.cGet('[modelid="insert-insert-calc-table"]')
			.should('have.attr', 'data-cooltip', 'Table styles are only available in .xlsx files');
	});
});

describe(['tagdesktop'], 'Notebookbar tooltip tests (XLSX).', function() {

	beforeEach(function() {
		helper.setupAndLoadDocument('calc/testfile.xlsx');
		desktopHelper.switchUIToNotebookbar();
	});

	it('InsertCalcTable button shows enabled tooltip when cells are selected', function() {
		calcHelper.selectCellsInRange('A1:C3');

		cy.cGet('#Insert-tab-label').click();

		cy.cGet('[modelid="insert-insert-calc-table"]')
			.should('not.have.attr', 'disabled');
		cy.cGet('[modelid="insert-insert-calc-table"]')
			.should('have.attr', 'data-cooltip', 'Insert a styled table');
	});

});
