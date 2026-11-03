/* -*- js-indent-level: 8 -*- */

/* global describe it cy require beforeEach expect */
var helper = require('../../common/helper');

describe(['tagdesktop'], 'Formula dialog tests', function() {

	beforeEach(function() {
		helper.withoutBrandTheme();
		helper.setupAndLoadDocument('calc/formula_dialog.ods');
		cy.getFrameWindow().then((win) => {
			this.win = win;
		});
	});

	it('Formula dialog visual regression test', function() {
		helper.processToIdle(this.win);

		cy.cGet('.unoFunctionDialog.formulabar').click();
		cy.cGet('#FormulaDialog').should('be.visible');

		cy.cGet('#FormulaDialog #function .ui-treeview-expander:nth(2)').click();
		cy.cGet('#FormulaDialog #function span').contains('ACCRINTM').dblclick();
		cy.cGet('#FormulaDialog #function .ui-treeview-entry.selected span').contains('ACCRINTM').should('exist');
		cy.cGet('#FormulaDialog label').contains('Settlement').should('be.visible');

		helper.processToIdle(this.win);

		cy.cGet('.ui-dialog').compareSnapshot('formula_dialog_accrintm', 0.15);

		helper.processToIdle(this.win);

		// collapse to selection mode
		cy.cGet('#FormulaDialog #RB_ARG1-button').click();

		cy.cGet('#FormulaDialog #function').should('not.be.visible');
		helper.processToIdle(this.win);

		cy.cGet('.ui-dialog').compareSnapshot('formula_dialog_accrintm_collapsed', 0.15);
	});

	it('Lists get shorter so the dialog fits a short window', function() {
		function dialogContent() {
			return cy.cGet('#FormulaDialog').closest('.ui-dialog-content');
		}

		function checkDialogFits() {
			dialogContent().should(function ($content) {
				const content = $content[0];
				expect(content.scrollHeight, 'dialog content height').to.be.at.most(content.clientHeight + 1);
			});
		}

		function checkFunctionListHeight(check) {
			cy.cGet('#FormulaDialog #function').should(function ($list) {
				check($list[0].offsetHeight);
			});
		}

		// In this window the dialog only fits when the function list is shorter than the rows it
		// asks for.
		cy.viewport(1000, 660);
		helper.processToIdle(this.win);
		cy.cGet('.unoFunctionDialog.formulabar').click();
		cy.cGet('#FormulaDialog').should('be.visible');
		helper.processToIdle(this.win);
		checkDialogFits();
		checkFunctionListHeight(function (height) {
			expect(height, 'function list height').to.be.within(150, 250);
		});

		// In a tall window the list shows the rows it asks for.
		cy.viewport(1000, 1000);
		helper.processToIdle(this.win);
		checkDialogFits();
		checkFunctionListHeight(function (height) {
			expect(height, 'function list height').to.be.above(300);
		});

		// When the window gets short again while the dialog is open, the list gets shorter again.
		cy.viewport(1000, 660);
		helper.processToIdle(this.win);
		checkDialogFits();
		checkFunctionListHeight(function (height) {
			expect(height, 'function list height').to.be.within(150, 250);
		});
	});
});
