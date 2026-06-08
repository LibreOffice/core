/* global describe it cy beforeEach require */

var helper = require('../../common/helper');

// Regression test for issue 6959: a data validation input message must stay
// visible while the cursor sits on its cell, instead of flashing once and
// disappearing as soon as the kit resends the cellcursor for the same cell.

describe(['tagdesktop', 'tagnextcloud', 'tagproxy'], 'Validity input help', function() {
	beforeEach(function() {
		helper.setupAndLoadDocument('calc/validity_input_help.fods');
	});

	it('Input help stays visible after the document loads on its cell', function() {
		// The document opens with the cursor on A1, which carries the help.
		cy.cGet('.input-help').should('be.visible');
		cy.cGet('.input-help h4').should('have.text', 'Budget help');

		// Let the load settle so the kit cellcursor resends arrive. The help
		// must survive them rather than vanish.
		helper.typeIntoDocument('{esc}');
		cy.wait(1000);
		cy.cGet('.input-help').should('be.visible');
	});

	it('Input help disappears once the cursor leaves its cell', function() {
		cy.cGet('.input-help').should('be.visible');

		// Move to B1, which has no input help.
		helper.typeIntoDocument('{rightArrow}');
		cy.cGet(helper.addressInputSelector).should('have.prop', 'value', 'B1');
		cy.cGet('.input-help').should('not.exist');
	});

	it('Input help is hidden on another sheet and shown again on its own', function() {
		cy.cGet('.input-help').should('be.visible');

		// Sheet2 has no input help on its first cell.
		cy.cGet('#spreadsheet-tab1').click();
		cy.cGet('.input-help').should('not.exist');

		// Coming back to Sheet1 lands on A1 again and the help returns.
		cy.cGet('#spreadsheet-tab0').click();
		cy.cGet('.input-help').should('be.visible');
	});
});
