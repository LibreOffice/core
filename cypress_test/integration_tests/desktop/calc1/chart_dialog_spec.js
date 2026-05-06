/* global describe it cy beforeEach require */

var calcHelper = require('../../common/calc_helper');
var desktopHelper = require('../../common/desktop_helper')
var helper = require('../../common/helper');

describe(['tagdesktop', 'tagnextcloud', 'tagproxy'], 'Chart dialog tests', function() {
	beforeEach(function() {
		helper.setupAndLoadDocument('calc/chart_dialog.ods');
		cy.viewport(1920, 720);
	});

	it('Chart format axis(scale) manual min test', function() {
		const XPos = 10;
		const YPos = 10;
		const minValue = 2.1;
		// Select chart.
		calcHelper.clickAtOffset(XPos, YPos);
		// Enter chart edit mode.
		helper.typeIntoDocument('{enter}');
		// Right-click on the thick y-axis and click 'Format Axis'.
		calcHelper.clickAtOffset(XPos, YPos, true);
		helper.getContextMenuItem('Format Axis...').click();

		cy.cGet('.lokdialog_container').should('be.visible');

		// Auto min must be ON.
		cy.cGet('#CBX_AUTO_MIN-input').should('be.checked');
		// Manual min input must be disabled.
		cy.cGet('#EDT_MIN-input').should('not.be.enabled');
		// Turn off auto min.
		cy.cGet('#CBX_AUTO_MIN-input').uncheck();
		cy.cGet('#CBX_AUTO_MIN-input').should('not.be.checked');
		cy.cGet('#EDT_MIN-input').should('be.enabled');
		// Set min to 2.
		cy.cGet('#EDT_MIN-input').type('{backspace}' + minValue);
		cy.cGet('#ok-button').click();
		cy.cGet('.lokdialog_container').should('not.exist');

		// See if the above changes persisted.
		// Again right-click on the thick y-axis and select 'Format Axis'.
		calcHelper.clickAtOffset(XPos, YPos, true);
		helper.getContextMenuItem('Format Axis...').click();
		cy.cGet('.lokdialog_container').should('be.visible');
		// Auto min must be OFF.
		cy.cGet('#CBX_AUTO_MIN-input').should('not.be.checked');
		// Manual min input must be enabled.
		cy.cGet('#EDT_MIN-input').should('be.enabled').should('have.value', '' + minValue);
		cy.cGet('#ok-button').click();
		cy.cGet('.lokdialog_container').should('not.exist');
	});

	/*
	 * tests if the width of the 'chart wizard' is larger than a "reasonable"
	 * width and if it's larger that means something is obviously wrong, probably
	 * some css property.
	 * `reasonableWidth` = width at the time of writing this test +- 15px ;)
	 */
	it('Chart Wizard width', function() {
		cy.cGet('#Insert-tab-label').click();
		desktopHelper.getNbIcon('InsertObjectChart', 'Insert').click();

		cy.cGet('#CHART2_HID_SCH_WIZARD_ROADMAP')
			.should('be.visible')
			.invoke('width')
			.should('be.greaterThan', 380).and('be.lessThan', 450);
	});
});
