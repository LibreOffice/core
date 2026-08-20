/* global describe it cy beforeEach require */

var helper = require('../../common/helper');

describe(['tagdesktop'], 'Sidebar tests', function() {

	beforeEach(function() {
		helper.setupAndLoadDocument('calc/cell_cursor.ods');
		cy.viewport(1920,1080);
	});

	it('Style panel more button switches to the styles deck', function() {
		cy.cGet('#StyleListDeck').should('not.exist');

		cy.cGet('.StylesPropertyPanel .ui-expander-icon-right button').click();

		cy.cGet('#StyleListDeck').should('be.visible');
	});
});
