/* global describe it cy beforeEach require */

var helper = require('../../common/helper');
var calcHelper = require('../../common/calc_helper');
var mobileHelper = require('../../common/mobile_helper');

describe(['tagmobile', 'tagnextcloud', 'tagproxy'], 'Pivot table', function() {

	beforeEach(function() {
		helper.setupAndLoadDocument('calc/autofilter.ods');
		mobileHelper.enableEditingMobile();
		helper.setDummyClipboardForCopy();
	});

	function openPivotLayout() {
		calcHelper.clickOnFirstCell();
		mobileHelper.openInsertionWizard();
		cy.cGet('body').contains('.menu-entry-with-icon', 'Pivot Table...').click();
		cy.cGet('.mobile-wizard #PivotTableLayout').should('exist');
	}

	function selectFieldOption(listboxSelector, item) {
		cy.cGet(listboxSelector + ' > .ui-header').click();
		cy.cGet('.mobile-wizard #PivotTableLayout .ui-button-box').should('not.be.visible');
		cy.cGet(listboxSelector + ' .ui-combobox-text').contains(item).click();
		cy.cGet('.mobile-wizard #PivotTableLayout .ui-button-box').should('be.visible');
	}

	it('Insert a pivot table from the selection', function() {
		openPivotLayout();
		cy.cGet('.mobile-wizard #SelectSourceDialog').should('not.exist');
		cy.cGet('#mobile-wizard').invoke('outerHeight').should('be.gt', 500);
		cy.cGet('.mobile-wizard [id^="name-rows-"]').should('not.exist');

		selectFieldOption('#rows-add', 'Status');
		cy.cGet('.mobile-wizard #name-rows-0').should('have.text', 'Status');
		selectFieldOption('#values-add', 'Cypress Test');
		cy.cGet('#function-values-0 .ui-header-left').should('have.text', 'Sum - Cypress Test');
		selectFieldOption('#function-values-0', 'Count - Cypress Test');
		cy.cGet('#function-values-0 .ui-header-left').should('have.text', 'Count - Cypress Test');

		// A placed column leaves the other pickers.
		cy.cGet('#columns-add > .ui-header').click();
		cy.cGet('#columns-add .ui-combobox-text').should('have.length', 2);
		cy.cGet('#columns-add .ui-combobox-text').contains('Cypress Test').click();
		cy.cGet('.mobile-wizard #name-columns-0').should('have.text', 'Cypress Test');
		cy.cGet('.mobile-wizard #remove-columns-0 button').click();
		cy.cGet('.mobile-wizard [id^="name-columns-"]').should('not.exist');

		cy.cGet('.mobile-wizard #PivotTableLayout #ok.ui-pushbutton-wrapper').click();
		cy.cGet('#mobile-wizard').should('not.be.visible');

		calcHelper.assertSheetContents([
			'Status', 'Count - Cypress Test',
			'Fail', '2',
			'Pass', '2',
			'(empty)', '1',
			'Total Result', '5'
		], true);
	});

	// Long press on A2: inside the table, away from the field button of the header cell.
	function longPressOnSecondCell() {
		cy.getFrameWindow().then(function(win) {
			helper.processToIdle(win);
			var dpiScale = win.app.dpiScale;
			var cellRect = win.app.map._docLayer.sheetGeometry.getCellRect(0, 1);
			var offsetX = (cellRect.min.x + cellRect.max.x) / 2 / dpiScale - 2;
			var offsetY = (cellRect.min.y + cellRect.max.y) / 2 / dpiScale - 2;
			calcHelper.clickAtOffset(offsetX, offsetY, true);
		});
		cy.cGet('#mobile-wizard-content').should('be.visible');
	}

	it('Open the layout of an existing pivot table from the long press menu', function() {
		openPivotLayout();
		selectFieldOption('#rows-add', 'Status');
		selectFieldOption('#values-add', 'Cypress Test');
		cy.cGet('.mobile-wizard #PivotTableLayout #ok.ui-pushbutton-wrapper').click();
		cy.cGet('#mobile-wizard').should('not.be.visible');

		longPressOnSecondCell();
		cy.cGet('body').contains('.context-menu-link', 'Properties...').click();

		cy.cGet('.mobile-wizard #PivotTableLayout').should('exist');
		cy.cGet('.mobile-wizard #name-rows-0').should('have.text', 'Status');
		cy.cGet('#function-values-0 .ui-header-left').should('have.text', 'Sum - Cypress Test');
		cy.cGet('#mobile-wizard-dialog-dismiss').click();
		cy.cGet('#mobile-wizard').should('not.be.visible');
	});

	it('Leave the dialog with the back navigation', function() {
		openPivotLayout();

		cy.getFrameWindow().then(function(win) {
			win.history.back();
		});
		cy.cGet('#mobile-wizard').should('not.be.visible');
		cy.cGet('#toolbar-mobile-back').should('have.class', 'editmode-on');

		// The engine closed it too, so it opens again.
		mobileHelper.openInsertionWizard();
		cy.cGet('body').contains('.menu-entry-with-icon', 'Pivot Table...').click();
		cy.cGet('.mobile-wizard #PivotTableLayout').should('exist');
		cy.cGet('#mobile-wizard-dialog-dismiss').click();
		cy.cGet('#mobile-wizard').should('not.be.visible');
	});

	it('Insert a pivot table at a typed cell', function() {
		openPivotLayout();

		selectFieldOption('#rows-add', 'Status');
		selectFieldOption('#values-add', 'Cypress Test');
		selectFieldOption('#function-values-0', 'Count - Cypress Test');

		cy.cGet('.mobile-wizard .ui-tab').contains('Destination').click();
		cy.cGet('.mobile-wizard #destination-radio-named-range').should('not.be.visible');
		cy.cGet('.mobile-wizard #destination-radio-selection label').click();
		cy.cGet('.mobile-wizard #destination-radio-selection input').should('be.checked');
		cy.cGet('.mobile-wizard #destination-edit-input').clear().type('D1{enter}');

		cy.cGet('.mobile-wizard #PivotTableLayout #ok.ui-pushbutton-wrapper').click();
		cy.cGet('#mobile-wizard').should('not.be.visible');

		cy.cGet('#spreadsheet-tab1').should('not.exist');
		calcHelper.assertSheetContents([
			'Cypress Test', 'Status', '', 'Status', 'Count - Cypress Test',
			'Test 1', 'Pass', '', 'Fail', '2',
			'Test 2', 'Fail', '', 'Pass', '2',
			'Test 3', 'Pass', '', '(empty)', '1',
			'Test 4', '', '', 'Total Result', '5',
			'Test 5', 'Fail', '', '', ''
		], true);
	});
});
