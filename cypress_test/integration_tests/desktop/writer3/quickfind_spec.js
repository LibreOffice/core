/* global describe it cy before beforeEach require expect */

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
        cy.getFrameWindow().then(function(win) {
            return helper.processToIdle(win);
        });
        helper.assertFocus('id', 'navigator-search-input');

        cy.cGet('body').type('i');
        cy.getFrameWindow().then(function(win) {
            return helper.processToIdle(win);
        });
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

describe(['tagdesktop', 'tagnextcloud', 'tagproxy'], 'Searching in comments via quickfind', function() {

    beforeEach(function() {
        // Wide enough for the comments to be shown next to the page.
        cy.viewport(1400, 600);
        helper.setupAndLoadDocument('writer/quickfind_comments.fodt');
        writerHelper.openQuickFind();
    });

    it('A match inside a comment is listed and selected in the comment', function() {
        writerHelper.searchInQuickFind('apple');

        // One match is in the document text and one is in the comment.
        writerHelper.assertQuickFindMatches(2);

        cy.cGet('#QuickFindPanel #searchfinds').contains('Comment by Alice:').click();

        // The comment shows the matched word selected.
        cy.cGet('.cool-annotation-content-wrapper').should('be.visible');
        cy.getFrameWindow().should(function(win) {
            const selection = win.getSelection();
            expect(selection.toString()).to.equal('apple');
            const content = win.document.querySelector('.cool-annotation-content');
            expect(content.contains(selection.anchorNode)).to.equal(true);
        });

        // Picking the match in the document text selects it in the document again.
        cy.cGet('#QuickFindPanel #searchfinds div:nth-child(2)').click();
        helper.textSelectionShouldExist();
    });

    it('A match inside a comment is still selected after the comment is edited', function() {
        // The sidebar and the zoom level would otherwise hide the comment menu.
        desktopHelper.switchUIToNotebookbar();
        desktopHelper.sidebarToggle();
        desktopHelper.selectZoomLevel('50', false);

        // Only the author of a comment may edit it, so the test user writes one.
        helper.typeIntoDocument('{ctrl}{end}');
        desktopHelper.insertComment('green pear');
        writerHelper.searchInQuickFind('pear');

        // Put the search word in front of the comment text after the search.
        cy.cGet('.cool-annotation').last().find('.cool-annotation-menu').click();
        cy.cGet('body').contains('.ui-combobox-entry.jsdialog.ui-grid-cell', 'Modify').click();
        cy.cGet('.cool-annotation').last().find('.modify-annotation .cool-annotation-textarea')
            .type('{home}pear ');
        cy.cGet('.cool-annotation').last().find('[value="Save"]').click();
        cy.cGet('.cool-annotation').last().find('.cool-annotation-content')
            .should('have.text', 'pear green pear');

        cy.cGet('#QuickFindPanel #searchfinds').contains('Comment by').click();

        // The word selected in the comment is the match, which moved with the edit.
        cy.getFrameWindow().should(function(win) {
            const selection = win.getSelection();
            expect(selection.toString()).to.equal('pear');
            expect(selection.anchorOffset).to.equal(11);
        });
    });

    it('A comment match that was edited away is not picked', function() {
        // The sidebar and the zoom level would otherwise hide the comment menu.
        desktopHelper.switchUIToNotebookbar();
        desktopHelper.sidebarToggle();
        desktopHelper.selectZoomLevel('50', false);

        // Only the author of a comment may edit it, so the test user writes one.
        helper.typeIntoDocument('{ctrl}{end}');
        desktopHelper.insertComment('green apple');
        writerHelper.searchInQuickFind('apple');
        writerHelper.assertQuickFindMatches(3);

        // Take the search word out of the comment after the search.
        cy.cGet('.cool-annotation').last().find('.cool-annotation-menu').click();
        cy.cGet('body').contains('.ui-combobox-entry.jsdialog.ui-grid-cell', 'Modify').click();
        cy.cGet('.cool-annotation').last().find('.modify-annotation .cool-annotation-textarea')
            .type('{selectall}green plum');
        cy.cGet('.cool-annotation').last().find('[value="Save"]').click();
        cy.cGet('.cool-annotation').last().find('.cool-annotation-content')
            .should('have.text', 'green plum');

        cy.cGet('#QuickFindPanel #searchfinds').contains('green').click();
        cy.getFrameWindow().then(function(win) {
            return helper.processToIdle(win);
        });

        // The label still gives the number of matches instead of naming a picked one. It said
        // the same before the click, so the check waits until the click was handled.
        writerHelper.assertQuickFindMatches(3);
    });

    it('A comment inserted after picking a comment match keeps its text', function() {
        desktopHelper.switchUIToNotebookbar();
        writerHelper.searchInQuickFind('apple');
        cy.cGet('#QuickFindPanel #searchfinds').contains('Comment by Alice:').click();
        cy.getFrameWindow().then(function(win) {
            return helper.processToIdle(win);
        });

        // The helper checks that the new comment shows the text typed into it.
        desktopHelper.insertComment('green apple');

        writerHelper.searchInQuickFind('green');
        cy.cGet('#numberofsearchfinds').should('have.text', 'One match found.');
    });
});

describe(['tagdesktop', 'tagnextcloud', 'tagproxy'], 'Searching in resolved comments via quickfind', function() {

    beforeEach(function() {
        // Wide enough for the comments to be shown next to the page.
        cy.viewport(1400, 600);
        helper.setupAndLoadDocument('writer/quickfind_resolved_comment.fodt');
        writerHelper.openQuickFind();
    });

    it('A match in a long resolved reply shows the resolved comments', function() {
        // The resolved thread is hidden when the document opens.
        cy.cGet('.cool-annotation-content-wrapper').should('not.be.visible');

        writerHelper.searchInQuickFind('plum');
        cy.cGet('#numberofsearchfinds').should('have.text', 'One match found.');

        cy.cGet('#QuickFindPanel #searchfinds').contains('Comment by Eve:').click();

        // The reply is shown with the matched word selected. The word is on the last line of the
        // long reply, and both the comment text and the document are scrolled to show it.
        cy.getFrameWindow().should(function(win) {
            const selection = win.getSelection();
            expect(selection.toString()).to.equal('plum');
            const wordRect = selection.getRangeAt(0).getBoundingClientRect();
            const contentRect = selection.anchorNode.parentElement.closest('.cool-annotation-content')
                .getBoundingClientRect();
            const documentRect = win.document.getElementById('document-container').getBoundingClientRect();
            expect(wordRect.height).to.be.above(0);
            expect(wordRect.top).to.be.at.least(Math.max(contentRect.top, documentRect.top));
            expect(wordRect.bottom).to.be.at.most(Math.min(contentRect.bottom, documentRect.bottom));
        });
    });
});

describe(['tagdesktop', 'tagnextcloud', 'tagproxy'], 'Searching in long replies via quickfind', function() {

    beforeEach(function() {
        // Wide enough for the comments to be shown next to the page.
        cy.viewport(1400, 600);
        helper.setupAndLoadDocument('writer/quickfind_long_comment.fodt');
        writerHelper.openQuickFind();
    });

    it('A match beyond the visible lines of a long reply is scrolled into view', function() {
        writerHelper.searchInQuickFind('kiwi');
        cy.cGet('#QuickFindPanel #searchfinds').contains('Comment by Carol:').click();

        // The thread is far down the page, below the part that shows when the document opens.
        // Picking the reply opens its thread, and a later comment leaves room for only part of
        // the long reply, so its text scrolls inside its card. The selected word is inside the
        // part of the reply that is shown, and inside the document area.
        cy.getFrameWindow().should(function(win) {
            const selection = win.getSelection();
            expect(selection.toString()).to.equal('kiwi');
            const wordRect = selection.getRangeAt(0).getBoundingClientRect();
            const contentRect = selection.anchorNode.parentElement.closest('.cool-annotation-content')
                .getBoundingClientRect();
            const documentRect = win.document.getElementById('document-container').getBoundingClientRect();
            expect(wordRect.height).to.be.above(0);
            expect(wordRect.top).to.be.at.least(Math.max(contentRect.top, documentRect.top));
            expect(wordRect.bottom).to.be.at.most(Math.min(contentRect.bottom, documentRect.bottom));
        });
    });
});

describe(['tagdesktop', 'tagnextcloud', 'tagproxy'], 'Searching in comments with links via quickfind', function() {

    beforeEach(function() {
        // Wide enough for the comments to be shown next to the page.
        cy.viewport(1400, 600);
        helper.setupAndLoadDocument('writer/quickfind_link_comment.fodt');
        writerHelper.openQuickFind();
    });

    it('A match after links in a comment is selected', function() {
        // The comment has a link and a web address written as plain text before the match.
        writerHelper.searchInQuickFind('kiwi');
        cy.cGet('#QuickFindPanel #searchfinds').contains('Comment by Dave:').click();

        cy.getFrameWindow().should(function(win) {
            expect(win.getSelection().toString()).to.equal('kiwi');
        });
    });
});
