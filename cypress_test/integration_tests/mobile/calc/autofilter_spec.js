/* global describe it cy beforeEach require */

var helper = require('../../common/helper');
var calcHelper = require('../../common/calc_helper');
var mobileHelper = require('../../common/mobile_helper');

describe(['tagmobile', 'tagnextcloud', 'tagproxy'], 'AutoFilter', function() {

	beforeEach(function() {
		helper.setupAndLoadDocument('calc/autofilter.ods', false, false, undefined, 'vector=false');
		// Click on edit button
		mobileHelper.enableEditingMobile();
		helper.setDummyClipboardForCopy();
		calcHelper.assertSheetContents(['Cypress Test', 'Status', 'Test 1', 'Pass', 'Test 2', 'Fail', 'Test 3', 'Pass', 'Test 4', '', 'Test 5', 'Fail'], true);
	});

	it('Sort by ascending/descending', function() {
		calcHelper.openAutoFilterMenu();

		//sort by descending order
		cy.cGet('body').contains('.mobile-wizard', 'Sort Descending').click();
		// Wait for autofilter dialog to close
		cy.cGet('.mobile-wizard').should('not.exist');

		calcHelper.assertSheetContents(['Cypress Test', 'Status', 'Test 5', 'Fail', 'Test 4', '', 'Test 3', 'Pass', 'Test 2', 'Fail', 'Test 1', 'Pass'], true);

		//sort by ascending order
		calcHelper.openAutoFilterMenu();

		cy.cGet('body').contains('.mobile-wizard', 'Sort Ascending').click();
		// Wait for autofilter dialog to close
		cy.cGet('.mobile-wizard').should('not.exist');

		calcHelper.assertSheetContents(['Cypress Test', 'Status', 'Test 1', 'Pass', 'Test 2', 'Fail', 'Test 3', 'Pass', 'Test 4', '', 'Test 5', 'Fail'], true);
	});
});
