/* global describe it cy beforeEach require */

var helper = require('../../common/helper');
var impressHelper = require('../../common/impress_helper');

describe(['tagdesktop'], 'Arrow styles of a reloaded shape', function() {

	beforeEach(function() {
		helper.setupAndLoadDocument('impress/arrow_style_reload.fodp', false, false, undefined, 'vector=false');
	});

	it('Line dialog lists the arrow styles of the reloaded shape', function() {
		impressHelper.selectTextShapeInTheCenter();

		cy.getFrameWindow().then(function(win) {
			win.app.map.sendUnoCommand('.uno:FormatLine');
		});
		cy.cGet('#LineDialog').should('be.visible');

		cy.cGet('#LineDialog #LB_START_STYLE input')
			.should('have.value', 'CF Zero Many');
		cy.cGet('#LineDialog #LB_END_STYLE input')
			.should('have.value', 'Arrow');

		cy.cGet('#LineDialog #cancel').click();
		cy.cGet('#LineDialog').should('not.exist');
	});
});
