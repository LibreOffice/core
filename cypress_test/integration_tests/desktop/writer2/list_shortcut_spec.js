/* global describe it cy require */

var helper = require('../../common/helper');

describe(['tagdesktop'], 'Writer list keyboard shortcuts', function() {

	it('Ctrl Shift 8 turns the paragraph into an unordered list and back', function() {
		helper.setupAndLoadDocument('writer/notebookbar.odt');

		cy.realPress(['Control', 'Shift', '8']);
		helper.waitForMapState('.uno:DefaultBullet', 'true');

		cy.realPress(['Control', 'Shift', '8']);
		helper.waitForMapState('.uno:DefaultBullet', 'false');
	});

	it('Ctrl Shift 7 turns the paragraph into an ordered list and back', function() {
		helper.setupAndLoadDocument('writer/notebookbar.odt');

		cy.realPress(['Control', 'Shift', '7']);
		helper.waitForMapState('.uno:DefaultNumbering', 'true');

		cy.realPress(['Control', 'Shift', '7']);
		helper.waitForMapState('.uno:DefaultNumbering', 'false');
	});
});
