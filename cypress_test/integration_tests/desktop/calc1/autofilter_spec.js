/* global describe it cy beforeEach require */

var helper = require('../../common/helper');
var calcHelper = require('../../common/calc_helper');
var desktopHelper = require('../../common/desktop_helper');

function toggleAutofilter() {
	//enable/disable autofilter
	cy.cGet('#menu-data').click();
	cy.cGet('body').contains('#menu-data li', 'AutoFilter').click();
}

describe(['tagdesktop', 'tagnextcloud', 'tagproxy'], 'AutoFilter Complex', function() {

	beforeEach(function() {
		helper.setupAndLoadDocument('calc/autofilter-complex.ods');
		desktopHelper.switchUIToCompact();

		// make deterministic jump, so in retry we have similar scrollbar values
		helper.typeIntoInputField(helper.addressInputSelector, 'A1');
		cy.wait(1000);

		helper.typeIntoInputField(helper.addressInputSelector, 'U126');
		cy.cGet('#map').focus();
		cy.wait(1000);

		desktopHelper.assertScrollbarPosition('vertical', 230, 270);
		desktopHelper.assertScrollbarPosition('horizontal', 260, 290);
	});

	it('Check checkbox status in the date tree', function() {
		helper.typeIntoInputField(helper.addressInputSelector, 'P100');

		cy.cGet('#test-div-OwnCellCursor').then((div) => {
			const rect = div[0].getBoundingClientRect();
			const x = rect.right - 5;
			const y = rect.bottom - 10;
			cy.cGet('body').click(x, y);
		});

		cy.cGet('.autofilter .vertical').should('be.visible');
		cy.cGet('#toggle_all-input').should('not.be.checked');

		cy.cGet('.autofilter .ui-treeview-expander-column').eq(0).click(); // open 2022
		cy.cGet('.autofilter .ui-treeview-expander-column').eq(1).click(); // open January

		cy.cGet('#toggle_all-input').should('not.be.checked');
		cy.cGet('.autofilter input[type="checkbox"]').eq(4).should('not.be.checked');
	});
});


describe(['tagdesktop', 'tagnextcloud', 'tagproxy'], 'AutoFilter', function() {

	beforeEach(function() {
		helper.setupAndLoadDocument('calc/autofilter.ods');
		desktopHelper.switchUIToCompact();
		toggleAutofilter();
		helper.setDummyClipboardForCopy();
		calcHelper.assertSheetContents(['Cypress Test', 'Status', 'Test 1', 'Pass', 'Test 2', 'Fail', 'Test 3', 'Pass', 'Test 4', '', 'Test 5', 'Fail'], true);
	});

	it.skip('Enable/Disable autofilter', function() {
		//filter by pass
		calcHelper.openAutoFilterMenu(true);
		cy.cGet('.autofilter .vertical').should('be.visible');
		cy.cGet('.autofilter  .ui-treeview-entry-checkbox').eq(0).uncheck();
		cy.cGet('.autofilter  .ui-treeview-entry-checkbox').eq(1).uncheck();
		cy.cGet('.autofilter .ui-button-box-right #ok').click();
		// Wait for autofilter dialog to close
		cy.cGet('div.autofilter').should('not.exist');

		calcHelper.assertSheetContents(['Cypress Test', 'Status', 'Test 1', 'Pass', 'Test 3', 'Pass']);

		// Disable autofilter
		// First toggle fails when whole sheet is selected, as it is after assertSheetContents
		toggleAutofilter();
		toggleAutofilter();

		calcHelper.assertSheetContents(['Cypress Test', 'Status', 'Test 1', 'Pass', 'Test 2', 'Fail', 'Test 3', 'Pass', 'Test 4', '', 'Test 5', 'Fail']);
	});

	it('Sort by ascending/descending', function() {
		calcHelper.openAutoFilterMenu();

		//sort by descending order
		cy.cGet('.autofilter').contains('.ui-treeview-entry', 'Sort Descending').click();
		// Wait for autofilter dialog to close
		cy.cGet('div.autofilter').should('not.exist');

		calcHelper.assertSheetContents(['Cypress Test', 'Status', 'Test 5', 'Fail', 'Test 4', '', 'Test 3', 'Pass', 'Test 2', 'Fail', 'Test 1', 'Pass'], true);

		//sort by ascending order
		calcHelper.openAutoFilterMenu();
		cy.cGet('.autofilter').contains('.ui-treeview-entry', 'Sort Ascending').click();
		// Wait for autofilter dialog to close
		cy.cGet('div.autofilter').should('not.exist');

		calcHelper.assertSheetContents(['Cypress Test', 'Status', 'Test 1', 'Pass', 'Test 2', 'Fail', 'Test 3', 'Pass', 'Test 4', '', 'Test 5', 'Fail'], true);
	});

	it('Filter empty/non-empty cells', function() {
		//empty
		calcHelper.openAutoFilterMenu(true);
		cy.cGet('#check_list_box :nth-child(1 of .ui-treeview-entry) > div > input').click();
		cy.cGet('#ok').click();
		// Wait for autofilter dialog to close
		cy.cGet('div.autofilter').should('not.exist');

		calcHelper.assertSheetContents(['Cypress Test', 'Status', 'Test 1', 'Pass', 'Test 2', 'Fail', 'Test 3', 'Pass', 'Test 5', 'Fail'], true);
	});

	it('Close autofilter popup by click outside', function() {
		// Test sometimes fails without this wait, no idea why.
		cy.wait(1000);

		calcHelper.openAutoFilterMenu();

		cy.cGet('.autofilter .vertical').should('be.visible');
		cy.cGet('div.jsdialog-overlay').should('be.visible');
		cy.cGet('div.jsdialog-overlay').click();

		// Wait for autofilter dialog to close
		cy.cGet('div.autofilter').should('not.exist');
		cy.wait(500);

		calcHelper.dblClickOnFirstCell();
		// Position of the double click determines the cursor position. So press home button in order to go to start of the cell.
		helper.typeIntoDocument('{home}');
		// Type new content to verify that cell is in edit mode
		helper.typeIntoDocument('New content{enter}');

		calcHelper.assertSheetContents(['New contentCypress Test', 'Status', 'Test 1', 'Pass', 'Test 2', 'Fail', 'Test 3', 'Pass', 'Test 4', '', 'Test 5', 'Fail'], true);
	});

	// check if filter by color applied or not
	it('Filter by color', function() {
		// apply background color to some cells
		calcHelper.selectCellsInRange('A2:A2');
		cy.cGet('#sidebar-dock-wrapper .unoBackgroundColor .arrowbackground').click();
		desktopHelper.selectColorFromPalette('3FAF46');

		calcHelper.openAutoFilterMenu();

		//Click on `Filter by Color`
		cy.cGet('.autofilter').contains('.ui-treeview-entry', 'Filter by Color').click();

		// Find the table element with ID "background"
		cy.cGet('#background')
		.find('input') // Find all input elements inside the table
		.each(($input) => { // Iterate through each input element
			// Assert that each input is of type radio
			cy.wrap($input).should('have.attr', 'type', 'radio');
		});

		// Find the table element with ID "background"
		cy.cGet('#background')
		.find('img') // Find all input elements inside the table
		.first() // Select the first input element
		.click(); // Click on the first input element

		calcHelper.assertSheetContents(['Cypress Test', 'Status', 'Test 1', 'Pass'], true);
	});

	it('Disable already filtered', function () {
		// Filter row with ['Test 4', ''] on the first column
		calcHelper.openAutoFilterMenu();
		cy.cGet('#check_list_box :nth-child(4 of .ui-treeview-entry) > div > input').click();
		cy.cGet('#ok').click();
		// Wait for autofilter dialog to close
		cy.cGet('div.autofilter').should('not.exist');

		// Open autofilter menu on the second column
		calcHelper.openAutoFilterMenu(true);
		// Check that '(empty)' option is disabled
		cy.cGet('#check_list_box :nth-child(3 of .ui-treeview-entry) > div').should('contain.text', '(empty)');
		cy.cGet('#check_list_box :nth-child(3 of .ui-treeview-entry) > div > input').should('be.disabled');

	});
});
