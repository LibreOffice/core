/* global describe it cy require beforeEach */
var helper = require('../../common/helper');

describe(['tagdesktop', 'tagnextcloud', 'tagproxy'], 'Debug tools', function() {

	beforeEach(function() {
		helper.setupAndLoadDocument('calc/debug_tools.ods');
	});

	it('Enter and exit debug mode', function() {
		cy.cGet('#debug-panel').should('not.exist');
		helper.typeIntoDocument('{ctrl}{alt}{shift}D');
		cy.cGet('#debug-panel').should('be.visible');
		helper.typeIntoDocument('{ctrl}{alt}{shift}D');
		cy.cGet('#debug-panel').should('not.exist');
	});

});
