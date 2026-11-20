/* global describe it cy beforeEach require */

var helper = require('../../common/helper');
var mobileHelper = require('../../common/mobile_helper');

describe(['tagmobile'], 'Toolbar tests', function() {
	beforeEach(function() {
		helper.setupAndLoadDocument('writer/search_bar.odt', false, false, undefined, 'vector=false');
	});

	it('Hides the FAB on search.', function() {
		cy.cGet('#mobile-edit-button').should('be.visible');

		mobileHelper.selectHamburgerMenuItem(['Search']);

		cy.cGet('#mobile-edit-button').should('not.be.visible');

		// can't use closeSearchBar since as it checks if #bold is visible - and it isn't in read mode
		cy.cGet('#hidesearchbar').click();
		cy.cGet('input#search-input').should('not.be.visible');

		cy.cGet('#mobile-edit-button').should('be.visible');
	});
});
