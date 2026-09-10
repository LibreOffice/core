/* global describe it cy require */

var helper = require('../../common/helper');
var impressHelper = require('../../common/impress_helper');

describe(['tagdesktop'], 'Impress list keyboard shortcuts', function() {

	it('Ctrl Shift 7 and Ctrl Shift 8 apply the two kinds of list', function() {
		helper.setupAndLoadDocument('impress/apply_paragraph_props_text.odp');

		cy.getFrameWindow().then(function(win) {
			impressHelper.triggerNewSVGForShapeInTheCenter();
			impressHelper.selectTextOfShape();
			helper.processToIdle(win);

			cy.realPress(['Control', 'Shift', '8']);
			helper.waitForMapState('.uno:DefaultBullet', 'true');

			cy.realPress(['Control', 'Shift', '7']);
			helper.waitForMapState('.uno:DefaultNumbering', 'true');
		});
	});
});
