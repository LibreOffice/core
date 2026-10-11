/* global describe it cy beforeEach require expect*/

var helper = require('../../common/helper');
var mobileHelper = require('../../common/mobile_helper');
var calcHelper = require('../../common/calc_helper');

describe(['tagmobile', 'tagnextcloud', 'tagproxy'], 'Chart tests.', function() {

	beforeEach(function() {
		helper.setupAndLoadDocument('calc/chart.ods');
		mobileHelper.enableEditingMobile();
		calcHelper.selectFirstColumn();
		insertChart();
	});

	function insertChart() {
		mobileHelper.openInsertionWizard();
		cy.cGet('body').contains('.menu-entry-with-icon', 'Chart...').click();
		cy.cGet('.mobile-wizard #ChartWizard').should('exist');
		cy.cGet('#mobile-wizard').invoke('outerHeight').should('be.gt', 500);
	}

	function closeChartDialog() {
		cy.cGet('.mobile-wizard #ChartWizard #ok.ui-pushbutton-wrapper').click();
		cy.cGet('#mobile-wizard').should('not.be.visible');
		cy.cGet('#test-div-shapeHandlesSection').should('exist');
	}

	function stepIntoChartEditing() {
		cy.cGet('#test-div-shapeHandlesSection').should('exist');

		// Double click onto the chart shape
		cy.cGet('svg g svg')
			.then(function(items) {
				expect(items).to.have.length(1);
				var boundingRect = items[0].getBoundingClientRect();
				var XPos = boundingRect.left + 10;
				var YPos = (boundingRect.top + boundingRect.bottom) / 2;

				cy.cGet('body').dblclick(XPos, YPos);
			});

		cy.cGet('#test-div-shapeHandlesSection').should('not.exist');
	}

	function exitChartEditing() {
		calcHelper.typeIntoFormulabar('{enter}');
	}

	function selectChartOnCenter() {
		cy.cGet('#document-container')
			.then(function(items) {
				expect(items).to.have.length(1);
				var boundingRect = items[0].getBoundingClientRect();
				var XPos = (boundingRect.left + boundingRect.right) / 2;
				var YPos = (boundingRect.top + boundingRect.bottom) / 2;
				cy.cGet('body').click(XPos, YPos);
			});

		cy.cGet('#test-div-shapeHandlesSection').should('exist');
	}

	it('Insert a column chart.', function() {
		cy.cGet('.mobile-wizard .ui-tab').contains('Data').click();
		cy.cGet('.mobile-wizard #ED_RANGE-input').should('have.value', '$Sheet1.$A$1:$A$3');
		cy.cGet('.mobile-wizard #IB_RANGE').should('not.be.visible');
		closeChartDialog();
		cy.cGet('svg .OLE2').should('exist');
		cy.cGet('svg .OLE2 g g path').should('have.length', 40);
	});

	it('Cancel removes the chart.', function() {
		cy.cGet('#mobile-wizard-dialog-dismiss').click();
		cy.cGet('#mobile-wizard').should('not.be.visible');
		cy.cGet('#test-div-shapeHandlesSection').should('not.exist');
		cy.cGet('svg .OLE2').should('not.exist');
	});

	it('Pick a pie chart.', function() {
		cy.cGet('.mobile-wizard #charttype .ui-treeview-entry').contains('Pie').click();
		closeChartDialog();
		cy.cGet('svg .OLE2 g g path').should('have.length', 7);
	});

	it('Give the chart a title.', function() {
		cy.cGet('.mobile-wizard .ui-tab').contains('Elements').click();
		cy.cGet('.mobile-wizard #maintitle-input').type('Hello{enter}');
		closeChartDialog();
		cy.cGet('svg .OLE2').should('exist');
		cy.cGet('svg .OLE2 text').contains('Hello').should('exist');
	});

	it.skip('Change chart type.', function() {
		closeChartDialog();
		stepIntoChartEditing();
		mobileHelper.openMobileWizard();
		cy.cGet('#ChartTypePanel').click();
		cy.cGet('#cmb_chartType').click();
		cy.cGet('.ui-combobox-text', 'Pie').click();

		// TODO: this leads to crash?
		//mobileHelper.closeMobileWizard();

		exitChartEditing();
		selectChartOnCenter();
		cy.cGet('svg .OLE2 g g path').should('have.length', 7);
	});
});
