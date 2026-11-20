/* global describe it cy beforeEach require */

var helper = require('../../common/helper');

describe.skip(['tagmultiuser'], 'Change paragraph properties', function() {

	beforeEach(function() {
		helper.setupAndLoadDocument('writer/paragraph_prop.odt',true, false, undefined, 'vector=false');
	});

	it('Change paragraph alignment.', function() {
		cy.cSetActiveFrame('#iframe1');
		//user 1 change the paragraph alignment
		cy.cGet('#document-container').click();
		cy.cGet('#rightpara').click();

		helper.selectAllText();

		//assert for user-1
		cy.cGet('#copy-paste-container p').should('have.attr', 'align', 'right');

		//assert for user-2
		cy.cSetActiveFrame('#iframe2');
		cy.cGet('#document-container').click();

		helper.selectAllText();

		cy.cGet('#copy-paste-container p').should('have.attr', 'align', 'right');

		//user-2 changes alignment to left
		cy.cGet('#document-container').click();
		cy.cGet('#leftpara').click();

		helper.selectAllText();

		//assert for user-2
		cy.cSetActiveFrame('#iframe2');
		cy.cGet('#copy-paste-container p').should('have.attr', 'align', 'left');
		//assert for user-1
		cy.cSetActiveFrame('#iframe1');
		cy.cGet('#copy-paste-container p').should('have.attr', 'align', 'left');
	});
});
