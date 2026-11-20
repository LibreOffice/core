/* global describe it cy beforeEach require */

var helper = require('../../common/helper');
var searchHelper = require('../../common/search_helper');
var mobileHelper = require('../../common/mobile_helper');
var writerHelper = require('../../common/writer_helper');

describe.skip('Searching via search bar.', function() {

	beforeEach(function() {
		helper.setupAndLoadDocument('writer/search_bar.odt', false, false, undefined, 'vector=false');
		mobileHelper.enableEditingMobile();
		searchHelper.showSearchBar();
	});

	it('Search existing word.', function() {
		searchHelper.typeIntoSearchField('a');
		// Part of the text should be selected
		helper.textSelectionShouldExist();
		helper.expectTextForClipboard('a');
	});

	it('Search not existing word.', function() {
		writerHelper.selectAllTextOfDoc();
		searchHelper.typeIntoSearchField('q');
		helper.textSelectionShouldNotExist();
	});

	it('Search next / prev instance.', function() {
		searchHelper.typeIntoSearchField('a');
		helper.textSelectionShouldExist();
		helper.expectTextForClipboard('a');
		cy.cGet('#copy-paste-container p b').should('not.exist');
		// Search next instance
		searchHelper.searchNext();
		cy.cGet('#copy-paste-container p b').should('exist');
		helper.textSelectionShouldExist();
		helper.expectTextForClipboard('a');
		// Search prev instance
		searchHelper.searchPrev();
		cy.cGet('#copy-paste-container p b').should('not.exist');
		helper.textSelectionShouldExist();
		helper.expectTextForClipboard('a');
	});

	it('Search at the document end.', function() {
		searchHelper.typeIntoSearchField('a');
		helper.textSelectionShouldExist();
		helper.expectTextForClipboard('a');
		cy.cGet('#copy-paste-container p b').should('not.exist');
		// Search next instance
		searchHelper.searchNext();
		cy.cGet('#copy-paste-container p b').should('exist');
		helper.textSelectionShouldExist();
		helper.expectTextForClipboard('a');
		// Search next instance, which is in the beginning of the document.
		searchHelper.searchNext();
		cy.cGet('#copy-paste-container p b').should('not.exist');
		helper.textSelectionShouldExist();
		helper.expectTextForClipboard('a');
	});

	it('Cancel search.', function() {
		searchHelper.typeIntoSearchField('a');
		// Part of the text should be selected
		helper.textSelectionShouldExist();
		helper.expectTextForClipboard('a');
		// Cancel search -> selection removed
		searchHelper.cancelSearch();
		helper.textSelectionShouldNotExist();
		cy.cGet('input#search-input').should('be.visible');
	});

	it('Close search.', function() {
		searchHelper.typeIntoSearchField('a');
		// Part of the text should be selected
		helper.textSelectionShouldExist();
		helper.expectTextForClipboard('a');
		// Close search -> search bar is closed
		searchHelper.closeSearchBar();
	});
});
