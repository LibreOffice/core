/* global describe it cy beforeEach require */

var calcHelper = require('../../common/calc_helper');
var helper = require('../../common/helper');

describe(['tagdesktop', 'tagnextcloud', 'tagproxy'], 'Chart wizard data series dialog tests', function() {
	beforeEach(function() {
		helper.setupAndLoadDocument('calc/chart_wizard.ods');
		cy.viewport(1920, 720);
	});

	it('Chart wizard data series dialog visual test', function() {
		// Select some data first
		calcHelper.clickOnFirstCell();

		// Open chart wizard via notebookbar Insert tab
		cy.cGet('#Insert-tab-label').click();
		// do not click the hidden overflow button
		cy.cGet('#Insert .unoInsertObjectChart').eq(0).click();

		// Wait for Chart Type dialog to appear
		cy.cGet('.lokdialog_container').should('be.visible');

		// Navigate to the Data series/Data ranges dialog
        // Click "Next" to go from Chart type to Date range
		cy.cGet('.lokdialog_container #next').click();
		cy.wait(500);

        // Click "Next" again to go from Date range to Data series/ranges
		cy.cGet('.lokdialog_container #next').click();
		cy.wait(500);

		// Capture screenshot of the Data series/Data ranges dialog
		cy.cGet('.lokdialog_container').compareSnapshot('chart_wizard_data_series', 0.1);
	});
});
