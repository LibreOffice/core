/* global describe it cy beforeEach require */

var helper = require('../../common/helper');
var calcHelper = require('../../common/calc_helper');
var findHelper = require('../../common/find_helper');

describe(['tagdesktop'], 'Calc: searching without diacritics', function() {

	beforeEach(function() {
		helper.setupAndLoadDocument('calc/diacritic_search.fods');
		cy.getFrameWindow().then((win) => {
			this.win = win;
		});
	});

	it('finds an accented word once Diacritic-sensitive is off', function() {
		findHelper.openFindDialog(this.win);

		// The core answers the toggle with a new copy of the options, which replaces the
		// checkbox below, so wait for the core to go idle before using it.
		cy.cGet('#OptionsExpander-button').click();
		helper.processToIdle(this.win);

		cy.cGet('#includediacritics-input').should('be.visible').click();
		cy.cGet('#includediacritics-input').should('not.be.checked');

		findHelper.typeIntoSearchField('aa');
		findHelper.findNext(this.win);

		// A3 holds "a" followed by an a with an acute accent.
		calcHelper.assertAddressAfterIdle(this.win, 'A3');
	});
});
