/* -*- js-indent-level: 8 -*- */

/* global describe it cy require beforeEach */
var helper = require('../../common/helper');

describe(['tagdesktop'], 'Formulabar tests', function() {

	beforeEach(function() {
		helper.setupAndLoadDocument('calc/formulabar.ods');
		cy.cGet('#pos_window-input-address').should('have.value', 'AB106');
	});

	it('Do not hide on notebookbar collapse', function() {
		cy.cGet('#Home-tab-label').click();
		cy.cGet('#sc_input_window.formulabar').should('be.visible');
	});

	// FIXME: need to stabilize
	it.skip('Formula editing shows usage popup for a function', function() {
		// verify default cell selected
		cy.cGet('#sc_input_window.formulabar .ui-custom-textarea-cursor-layer')
			.should('have.text', '=SUM(Z101:Z104,AB101:AB104)');

		cy.cGet('#sc_input_window.formulabar').click().should('have.class', 'focused');

		// type arrow left to enter into function SUM
		cy.cGet('#clipboard-area').type('{leftArrow}');

		// we should see help for a SUM function
		cy.cGet('#formulausagePopup .jsdialog-container').should('be.visible');
		cy.cGet('#formulausagePopup .ui-treeview-cell-text-content')
			.contains('SUM(').should('exist');

		cy.wait(500);
		cy.cGet('#formulausagePopup .jsdialog-container').compareSnapshot('formulausage', 0.03);

		// we should be able to type into formulabar (popup is not interactive)
		cy.cGet('#sc_input_window.formulabar').should('have.class', 'focused');
		const additionalText = ',1000';
		cy.cGet('#clipboard-area').type(additionalText + '{enter}');

		// enter moves one cell down, go back
		cy.cGet('#pos_window-input-address').should('have.value', 'AB107');
		cy.cGet('#pos_window-input-address').type('{selectAll}AB106{enter}');
		cy.cGet('#pos_window-input-address').should('have.value', 'AB106');

		// verify modification
		cy.cGet('#sc_input_window.formulabar .ui-custom-textarea-cursor-layer')
			.should('have.text', '=SUM(Z101:Z104,AB101:AB104' + additionalText + ')');
	});
});
