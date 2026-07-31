/* global describe it cy before beforeEach require */

var helper = require('../../common/helper');
var desktopHelper = require('../../common/desktop_helper');
var writerHelper = require('../../common/writer_helper');

describe(['tagdesktop', 'tagnextcloud', 'tagproxy'], 'Searching via quickfind in navigation panel', { testIsolation: false }, function() {

    desktopHelper.shareDocumentAcrossTests('writer/search_bar.odt');

    // The panel is opened once and stays open, and the tests below search in
    // it. Closing it between tests leaves the client flag and core's own
    // navigator state to catch up with each other, and whether the panel takes
    // the focus when it opens depends on both.
    before(function() {
        writerHelper.openQuickFind();
    });

    it('Search existing word.', function() {
        writerHelper.searchInQuickFind('a');

        // Highlight the first hit
        helper.textSelectionShouldExist();

        writerHelper.assertQuickFindMatches(2);
    });

    it('Search not existing word.', function() {
        writerHelper.selectAllTextOfDoc();
        writerHelper.searchInQuickFind('q');
        helper.textSelectionShouldNotExist();
    });

    it('Search existing word in table.', function() {
        writerHelper.searchInQuickFind('b'); // check character inside table

        // Part of the text should be selected
        helper.textSelectionShouldExist();

        writerHelper.assertQuickFindMatches(5);

    });

    it('Search input should keep the focus after a part change', function() {
        helper.typeIntoDocument('{ctrl}f');

        cy.cGet('body').type('Off');
        cy.wait(1000);
        helper.assertFocus('id', 'navigator-search-input');

        cy.cGet('body').type('i');
        cy.wait(1000);
        helper.assertFocus('id', 'navigator-search-input');
    });

    it('Same-term search re-runs after focusing back into the document', function() {
        writerHelper.searchInQuickFind('a');
        writerHelper.assertQuickFindMatches(2);

        // Focusing the document body invalidates the cached results, so
        // the next Enter on the search field must re-run the search
        // instead of stepping through the existing list.
        cy.cGet('#document-container').click();
        cy.cGet('input#navigator-search-input').type('{enter}');
        cy.getFrameWindow().then(function(win) {
            return helper.processToIdle(win);
        });

        // A re-search keeps the "N results" label. Stepping to the next
        // match would replace it with "Match X of N matches found.".
        writerHelper.assertQuickFindMatches(2);
    });
});

// Both of these read what happens as the panel goes from closed to open, so
// they take a document of their own rather than the shared one, which keeps the
// panel open throughout.
describe(['tagdesktop', 'tagnextcloud', 'tagproxy'], 'Opening quickfind', function() {

    beforeEach(function() {
        helper.setupAndLoadDocument('writer/search_bar.odt');
    });

    it('Ctrl F should open and focus quickfind', function() {
        helper.typeIntoDocument('{ctrl}f');
        cy.cGet('#quickfind-dock-wrapper').should('be.visible');
        helper.assertFocus('id', 'navigator-search-input');
    });

    it('Results tab should be activated after a search', function() {
        helper.typeIntoDocument('{ctrl}f');
        cy.cGet('#quickfind-dock-wrapper').should('be.visible');
        helper.assertFocus('id', 'navigator-search-input');

        writerHelper.searchInQuickFind('a');

        // Results tab should be activated.
        cy.cGet('#tab-quick-find.selected').should('exist');

        cy.cGet('input#navigator-search-input').focus();

        // This should jump to next entry.
        cy.cGet('input#navigator-search-input').type('{enter}'); // Jump to first item.
        cy.cGet('input#navigator-search-input').type('{enter}'); // Jump to second item.

        // Index starts from 1.
        cy.cGet('#QuickFindPanel #searchfinds div:nth-child(3)').should('have.class', 'selected');

        // Search input should still have the focus.
        helper.assertFocus('id', 'navigator-search-input');
    });
});
