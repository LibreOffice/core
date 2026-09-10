/* global describe it cy require */

var helper = require('../../common/helper');
var calcHelper = require('../../common/calc_helper');
var findHelper = require('../../common/find_helper');

describe(['tagdesktop'], 'Calc: searching without diacritics', function() {

	it('finds an accented word once Diacritic-sensitive is off', function() {
		helper.setupAndLoadDocument('calc/diacritic_search.fods');
		cy.getFrameWindow().then(function(win) {
			findHelper.openFindDialog(win);

			cy.cGet('#OptionsExpander-button').click();
			cy.cGet('#includediacritics-input').should('be.visible').click();
			cy.cGet('#includediacritics-input').should('not.be.checked');

			findHelper.typeIntoSearchField('aa');
			findHelper.findNext(win);

			// A3 holds "a" followed by an a with an acute accent.
			calcHelper.assertAddressAfterIdle(win, 'A3');
		});
	});
});
