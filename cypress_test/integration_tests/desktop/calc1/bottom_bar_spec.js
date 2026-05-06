/* global describe it cy beforeEach require */

var helper = require('../../common/helper');
var calcHelper = require('../../common/calc_helper');

describe(['tagdesktop', 'tagnextcloud', 'tagproxy'], 'Calc bottom bar tests.', function() {

	beforeEach(function() {
		// Ensure the viewport is large enough to show the whole status bar
		// In Calc #StateTableCellMenu has a high data-priority
		// and will be hidden if the bottom bar doesn't have enough space
		cy.viewport(1280, 720);
		helper.setupAndLoadDocument('calc/BottomBar.ods');
	});

	it('Bottom tool bar.', function() {
		cy.cGet('#map').focus();
		calcHelper.clickOnFirstCell();
		cy.cGet('#StateTableCellMenu:visible').click();
		cy.cGet('body').contains('.ui-combobox-entry', 'CountA').click();
	});
});
