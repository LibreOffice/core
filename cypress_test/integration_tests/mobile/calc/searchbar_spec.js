/* global describe it cy beforeEach require */

var helper = require('../../common/helper');
var searchHelper = require('../../common/search_helper');
var mobileHelper = require('../../common/mobile_helper');

describe(['tagmobile', 'tagnextcloud', 'tagproxy'], 'Searching via search bar.', function() {

	beforeEach(function() {
		helper.setupAndLoadDocument('calc/search_bar.ods');

		mobileHelper.enableEditingMobile();

		// Make sure the spreadsheet finished loading before starting the search,
		// otherwise the search can start against an empty cell selection.
		cy.cGet(helper.addressInputSelector).should('have.value', 'A2');

		searchHelper.showSearchBar();
	});

	it('Search existing word.', function() {
		helper.setDummyClipboardForCopy();
		searchHelper.typeIntoSearchField('a');

		searchHelper.searchNext();

		// First cell should be selected
		cy.cGet(helper.addressInputSelector).should('have.value', 'A1');
		helper.copy();
		cy.cGet('#copy-paste-container table td').should('have.text', 'a');
	});

	it('Search not existing word.', function() {
		cy.cGet(helper.addressInputSelector).should('have.value', 'A2');

		searchHelper.typeIntoSearchField('q');

		// Should be no new selection
		cy.cGet(helper.addressInputSelector).should('have.value', 'A2');
	});

	it('Search next / prev instance.', function() {
		helper.setDummyClipboardForCopy();
		searchHelper.typeIntoSearchField('a');

		searchHelper.searchNext();

		cy.cGet(helper.addressInputSelector).should('have.value', 'A1');
		helper.copy();
		cy.cGet('#copy-paste-container table td').should('have.text', 'a');

		// Search next instance
		searchHelper.searchNext();

		cy.cGet(helper.addressInputSelector).should('have.value', 'B1');
		helper.copy();
		cy.cGet('#copy-paste-container table td').should('have.text', 'a');

		// Search prev instance
		searchHelper.searchPrev();

		cy.cGet(helper.addressInputSelector).should('have.value', 'A1');
		helper.copy();
		cy.cGet('#copy-paste-container table td').should('have.text', 'a');
	});

	it('Search at the document end.', function() {
		helper.setDummyClipboardForCopy();
		searchHelper.typeIntoSearchField('a');

		searchHelper.searchNext();

		cy.cGet(helper.addressInputSelector).should('have.value', 'A1');
		helper.copy();
		cy.cGet('#copy-paste-container table td').should('have.text', 'a');

		// Search next instance
		searchHelper.searchNext();
		cy.cGet(helper.addressInputSelector).should('have.value', 'B1');
		helper.copy();
		cy.cGet('#copy-paste-container table td').should('have.text', 'a');

		// Search next instance, which is in the beginning of the document.
		searchHelper.searchNext();
		cy.cGet(helper.addressInputSelector).should('have.value', 'A1');
		helper.copy();
		cy.cGet('#copy-paste-container table td').should('have.text', 'a');
	});

	it('Cancel search.', function() {
		helper.setDummyClipboardForCopy();
		searchHelper.typeIntoSearchField('a');

		searchHelper.searchNext();

		cy.cGet(helper.addressInputSelector).should('have.value', 'A1');
		helper.copy();
		cy.cGet('#copy-paste-container table td').should('have.text', 'a');

		// Cancel search -> selection removed
		searchHelper.cancelSearch();
		cy.cGet('input#search-input').should('be.visible');
	});

	it('Close search.', function() {
		helper.setDummyClipboardForCopy();
		searchHelper.typeIntoSearchField('a');
		searchHelper.searchNext();

		cy.cGet(helper.addressInputSelector).should('have.value', 'A1');
		helper.copy();
		cy.cGet('#copy-paste-container table td').should('have.text', 'a');

		// Close search -> search bar is closed
		searchHelper.closeSearchBar();
	});
});
