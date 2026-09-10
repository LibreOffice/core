/* global describe it cy expect require */

var helper = require('../../common/helper');

describe(['tagdesktop'], 'Calc: frozen panes of a saved document', function() {

	function checkFrozenPanes() {
		cy.cGet('#document-canvas').should('be.visible');
		cy.getFrameWindow().then(function(win) { return helper.processToIdle(win); });
		cy.getFrameWindow().should(function(win) {
			expect(win.app.calc.splitCoordinate.pX, 'frozen column split').to.be.greaterThan(0);
			expect(win.app.calc.splitCoordinate.pY, 'frozen row split').to.be.greaterThan(0);
		});
	}

	it('are applied in an editable view', function() {
		helper.setupAndLoadDocument('calc/frozen_panes.ods');
		checkFrozenPanes();
	});

	it('are applied in a read-only view', function() {
		var filePath = helper.setupDocument('calc/frozen_panes.ods');
		helper.loadDocument(filePath, true, undefined, undefined, 'permission=readonly');
		checkFrozenPanes();
	});
});
