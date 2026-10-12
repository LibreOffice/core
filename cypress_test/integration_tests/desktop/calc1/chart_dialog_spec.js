/* global describe it cy beforeEach require */

var calcHelper = require('../../common/calc_helper');
var desktopHelper = require('../../common/desktop_helper')
var helper = require('../../common/helper');

describe(['tagdesktop', 'tagnextcloud', 'tagproxy'], 'Chart dialog tests', function() {
	beforeEach(function() {
		helper.setupAndLoadDocument('calc/chart_dialog.ods');
		cy.viewport(1920, 720);
		cy.getFrameWindow().then(function(win) {
			this.win = win;
		}.bind(this));
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

	it('Data series icon menu button opens from its whole box', function() {
		// Select the chart and enter chart edit mode.
		calcHelper.clickAtOffset(10, 10);
		helper.typeIntoDocument('{enter}');
		helper.processToIdle(this.win);

		// Walk the chart object hierarchy with the keyboard: Tab selects the
		// first top-level object, the diagram, and Enter goes one level down
		// to its first child, the data series.
		helper.assertFocus('className', 'clipboard');
		cy.realPress('Tab');
		helper.processToIdle(this.win);
		cy.realPress('Enter');
		helper.processToIdle(this.win);

		cy.getFrameWindow().then(function(win) {
			win.app.map.sendUnoCommand('.uno:FormatDataSeries');
		});
		cy.cGet('#AttributeDialog').should('be.visible');
		helper.processToIdle(this.win);

		// The Line tab is the tab page named border.
		cy.cGet('#AttributeDialog #tabcontrol button#border').click();
		cy.cGet('#AttributeDialog #tabcontrol button#border').should('have.attr', 'aria-selected', 'true');
		helper.processToIdle(this.win);

		// The menu button under Icon shows the hand cursor over its whole box.
		cy.cGet('#MB_SYMBOL_BITMAP').should('be.visible').and('not.have.attr', 'disabled');
		cy.cGet('#MB_SYMBOL_BITMAP').should('have.css', 'cursor', 'pointer');

		// A click 2px inside the top-left corner lands on the border and
		// padding of the box, outside the inner button, and opens the menu.
		cy.cGet('#MB_SYMBOL_BITMAP').click(2, 2);
		cy.cGet('#MB_SYMBOL_BITMAP-dropdown').should('be.visible');
		cy.cGet('body').type('{esc}');
		cy.cGet('#MB_SYMBOL_BITMAP-dropdown').should('not.exist');

		cy.cGet('#AttributeDialog #cancel-button').click();
		cy.cGet('.lokdialog_container').should('not.exist');
	});
});
