/* global describe it cy beforeEach require */

var helper = require('../../common/helper');

describe(['tagdesktop', 'tagnextcloud', 'tagproxy'], 'Check mouse cursor type in various interactions', function() {

	beforeEach(function() {
		helper.setupAndLoadDocument('calc/mouse_cursor.ods');
	});

	/*
		NOTE: we need more of these tests for the elements where
		we change the cursor style:
			- the shape resize handles
			- the autofill marker
			- column/row headers
	*/
	it('Cell cursor over the sheet', function() {
		cy.cGet('#document-canvas').trigger('mouseover').then(() => {
			cy.cGet('#document-canvas').should('have.class', 'spreadsheet-cursor');
		});
	});
});
