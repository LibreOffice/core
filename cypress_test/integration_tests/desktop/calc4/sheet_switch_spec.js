/* global describe it cy beforeEach require */

var calcHelper = require('../../common/calc_helper');
var helper = require('../../common/helper');
var desktopHelper = require('../../common/desktop_helper');

describe(['tagdesktop', 'tagnextcloud', 'tagproxy'], 'Sheet switching tests', function() {
	beforeEach(function() {
		helper.setupAndLoadDocument('calc/calc-zoomed.fods');
		cy.viewport(1920,1080);
	});

	/* calc-zoomed.fods opens with the cell selection in the bottom right corner
	 * which can be later covered by the sidebar - causing it to be invisible */

	it('Check view position on sheet switch', function() {
		// we should be somewhere far from A1
		cy.cGet(helper.addressInputSelector).should('have.prop', 'value', 'CQ1017');
		desktopHelper.assertScrollbarPosition('vertical', 710, 750);
		desktopHelper.assertScrollbarPosition('horizontal', 930, 1050);

		// insert sheet
		cy.cGet('#sheets-buttons-toolbox #insertsheet').click();
		cy.cGet(helper.addressInputSelector).should('have.prop', 'value', 'A1');

		// after switch we should see cursor and A1
		desktopHelper.assertScrollbarPosition('vertical', 20, 100);
		desktopHelper.assertScrollbarPosition('horizontal', 40, 90);
	});

	it.skip('Check the visibility of tile content when rows are hidden', function() {
		// Insert sheet
		cy.cGet(helper.addressInputSelector).should('have.prop', 'value', 'CQ1017');
		cy.cGet('#sheets-buttons-toolbox #insertsheet').click();
		cy.cGet(helper.addressInputSelector).should('have.prop', 'value', 'A1');

		// Step1: Go to B1 and type something
		helper.typeIntoDocument('{rightArrow}Text');
		cy.wait(500);
		cy.cGet('#map').compareSnapshot('b1_text_step1', 0.1);

		// Step2: Hide rows and still see text
		helper.typeIntoDocument('{leftArrow}{downArrow}');
		cy.cGet(helper.addressInputSelector).should('have.prop', 'value', 'A2');
		helper.typeIntoDocument('{shift}{ctrl}{rightArrow}');
		helper.typeIntoDocument('{shift}{ctrl}{downArrow}');
		cy.cGet(helper.addressInputSelector).should('have.prop', 'value', 'A2:XFD1048576');
		calcHelper.hideSelectedRows();
		cy.cGet(helper.addressInputSelector).should('have.prop', 'value', 'A1');
		cy.wait(500);
		cy.cGet('#map').compareSnapshot('b1_text_step2', 0.05);

		// Step3: type and still see text
		helper.typeIntoDocument('Calc is Cool{enter}');
		cy.wait(500);
		cy.cGet('#map').compareSnapshot('b1_text_step3', 0.05);
	});
});

describe(['tagdesktop', 'tagnextcloud', 'tagproxy'], 'Sheet switching tests', function() {

	beforeEach(function() {
		helper.setupAndLoadDocument('calc/switch.ods');
	});

	/* switch.ods has 2 sheets, 1st with data in G45, 2nd with data in F720*/

	it('Check view position on sheet switch', function() {
		// go to sheet 1
		cy.cGet('#spreadsheet-tab0').click();

		desktopHelper.assertScrollbarPosition('vertical', 35, 45);
		cy.cGet(helper.addressInputSelector).should('have.prop', 'value', 'G45');

		// go to sheet 2
		cy.cGet('#spreadsheet-tab1').click();
		cy.cGet(helper.addressInputSelector).should('have.prop', 'value', 'F720');
		desktopHelper.assertScrollbarPosition('vertical', 300, 350);

		cy.cGet(helper.addressInputSelector).type('{selectAll}A2{enter}');
		desktopHelper.assertScrollbarPosition('vertical', 15, 25);
	});

	it('Check view position on repeated selection of currently selected sheet', function() {
		// initially we are on sheet 2 tab
		cy.cGet(helper.addressInputSelector).should('have.prop', 'value', 'F720');
		desktopHelper.assertScrollbarPosition('vertical', 300, 350);

		// click on sheet 2 tab (yes, current one)
		cy.cGet('#spreadsheet-tab1').click();
		cy.cGet(helper.addressInputSelector).should('have.prop', 'value', 'F720');
		desktopHelper.assertScrollbarPosition('vertical', 320, 380);

		// go to different place in the spreadsheet
		cy.cGet(helper.addressInputSelector).type('{selectAll}A2{enter}');
		desktopHelper.assertScrollbarPosition('vertical', 15, 25);

		// validate we didn't jump back after some time
		cy.wait(1000);
		desktopHelper.assertScrollbarPosition('vertical', 15, 25);
	});

	// TODO: remove if multiple sheet selection feature will be implemented
	it('Check if multiple sheet selection is disabled', function() {
		// go to sheet 1
		cy.cGet('#spreadsheet-tab0').click();

		desktopHelper.assertScrollbarPosition('vertical', 35, 45);
		cy.cGet(helper.addressInputSelector).should('have.prop', 'value', 'G45');
		cy.cGet('#spreadsheet-tab0').should('have.class', 'spreadsheet-tab-selected');
		cy.cGet('#spreadsheet-tab1').should('not.have.class', 'spreadsheet-tab-selected');

		// try to add sheet 2 to sheets selection
		helper.typeIntoDocument('{ctrl}{shift}{pageDown}');

		// we expect no effect
		desktopHelper.assertScrollbarPosition('vertical', 35, 45);
		cy.cGet(helper.addressInputSelector).should('have.prop', 'value', 'G45');
		cy.cGet('#spreadsheet-tab0').should('have.class', 'spreadsheet-tab-selected');
		cy.cGet('#spreadsheet-tab1').should('not.have.class', 'spreadsheet-tab-selected');
	});

	// TODO: enable if multiple sheet selection feature will be implemented
	//       this tests serious regression we had, so be sure it works properly
	it.skip('Check view position when having multiple sheet selection', function() {
		// go to sheet 1
		cy.cGet('#spreadsheet-tab0').click();

		desktopHelper.assertScrollbarPosition('vertical', 35, 45);
		cy.cGet(helper.addressInputSelector).should('have.prop', 'value', 'G45');
		cy.cGet('#spreadsheet-tab0').should('have.class', 'spreadsheet-tab-selected');
		cy.cGet('#spreadsheet-tab1').should('not.have.class', 'spreadsheet-tab-selected');

		// add sheet 2 to sheets selection
		helper.typeIntoDocument('{ctrl}{shift}{pageDown}');

		desktopHelper.assertScrollbarPosition('vertical', 300, 330);
		cy.cGet(helper.addressInputSelector).should('have.prop', 'value', 'F720');
		cy.cGet('#spreadsheet-tab0').should('have.class', 'spreadsheet-tab-selected');
		cy.cGet('#spreadsheet-tab1').should('have.class', 'spreadsheet-tab-selected');

		// try to go to sheet 1 using keyboard shortcut - it is not allowed in the core
		helper.typeIntoDocument('{ctrl}{alt}{pageUp}');

		// we still have selected two sheets so we see cell data from sheet 2
		cy.cGet(helper.addressInputSelector).should('have.prop', 'value', 'F720');

		// go to sheet 2 using tab
		cy.cGet('#spreadsheet-tab1').click();
		cy.cGet(helper.addressInputSelector).should('have.prop', 'value', 'F720');

		// go to different place in the spreadsheet
		cy.cGet(helper.addressInputSelector).type('{selectAll}A2{enter}');
		helper.typeIntoDocument('some text');

		// validate we didn't jump back after some time
		cy.wait(1000);
		desktopHelper.assertScrollbarPosition('vertical', 15, 25);
	});
});

describe(['tagdesktop', 'tagnextcloud', 'tagproxy'], 'Test sheet switching with split panes', function() {

	it('Check view position on sheet switch', function() {
		helper.setupAndLoadDocument('calc/switch-split.ods');
		// Sheet 1 has split panes
		desktopHelper.assertScrollbarPosition('horizontal', 300, 340);
		// Switch to another sheet with differently sized columns
		cy.cGet('#spreadsheet-tab1').click();
		// Switch back to the split sheet
		cy.cGet('#spreadsheet-tab0').click();
		// The panes positions shouldn't have changed
		desktopHelper.assertScrollbarPosition('horizontal', 300, 340);
	});
});
