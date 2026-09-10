/* global describe it cy require */

var helper = require('../../common/helper');

describe(['tagdesktop'], 'Draw list keyboard shortcuts', function() {

	it('Ctrl Shift 7 and Ctrl Shift 8 apply the two kinds of list', function() {
		helper.setupAndLoadDocument('draw/list_shortcut.fodg');

		// Tab selects the shape that holds the text.
		cy.realPress('Tab');
		cy.cGet('#test-div-shape-handle-0').should('exist');

		// Enter opens the text of the shape for editing.
		cy.realPress('Enter');
		cy.getFrameWindow().then(function(win) { return helper.processToIdle(win); });

		cy.realPress(['Control', 'Shift', '8']);
		helper.waitForMapState('.uno:DefaultBullet', 'true');

		cy.realPress(['Control', 'Shift', '7']);
		helper.waitForMapState('.uno:DefaultNumbering', 'true');
	});
});
