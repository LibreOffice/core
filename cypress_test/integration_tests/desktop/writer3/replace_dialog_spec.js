/* global describe it cy beforeEach require */

var helper = require('../../common/helper');
var findHelper = require('../../common/find_helper');

describe(['tagdesktop', 'tagnextcloud', 'tagproxy'], 'Replace Dialog Tests', function() {
    beforeEach(function() {
        helper.setupAndLoadDocument('writer/find_replace.odt');
        cy.getFrameWindow().then(function(win) {
            this.win = win;
        });
    });

    it('Ctrl H should open search dialog with replace tab active', function() {
        helper.typeIntoDocument('{ctrl}h');
        findHelper.waitForFindReplaceDialog(this.win);

        // Verify that the replace tab is active
        cy.cGet('#replace_tab_btn').should('have.class', 'checked');
        // Verify that the replace input field is visible
        cy.cGet('#FindReplaceDialog.jsdialog input#replaceterm-input-dialog').should('be.visible');
        // Verify that the focus is on find input field
        cy.cGet('#FindReplaceDialog.jsdialog input#searchterm-input-dialog').should('be.focused');
    });

    it('Replace button should open search dialog with replace tab active', function() {
        cy.viewport(1920,1080);
        // Click the Replace button from the notebookbar
        cy.cGet('#Home-container [id^="home-search-dialog"] button:visible').click();
        findHelper.waitForFindReplaceDialog(this.win);

        // Verify that the replace tab is active
        cy.cGet('#replace_tab_btn').should('have.class', 'checked');
        // Verify that the replace input field is visible
        cy.cGet('#FindReplaceDialog.jsdialog input#replaceterm-input-dialog').should('be.visible');
        // Verify that the focus is on find input field
        cy.cGet('#FindReplaceDialog.jsdialog input#searchterm-input-dialog').should('be.focused');
    });

    it('Enter key in search field triggers search', function() {
        helper.setDummyClipboardForCopy();
        helper.typeIntoDocument('{ctrl}h');
        findHelper.waitForFindReplaceDialog(this.win);

        cy.cGet('#searchterm-input-dialog').type('test').realPress('Enter');

        // Verify text was found and selected
        helper.textSelectionShouldExist();
        helper.copy();
        helper.expectTextForClipboard('test');
    });

    it('Shift plus Enter in search field triggers backward search', function() {
        helper.setDummyClipboardForCopy();
        helper.typeIntoDocument('{ctrl}h');
        findHelper.waitForFindReplaceDialog(this.win);

        // Go to first instance - Not Bold text
        cy.cGet('#searchterm-input-dialog').type('test');
        cy.realPress('Enter');
        helper.textSelectionShouldExist();
        cy.cGet('#copy-paste-container p b').should('not.exist');

        // Search forward again to get to second instance - Bold text
        cy.cGet('#searchterm-input-dialog').focus();
        cy.realPress('Enter');
        helper.copy();
        cy.cGet('#copy-paste-container p b').should('exist'); 

        // Now search backward with Shift+Enter - Not Bold text
        cy.cGet('#searchterm-input-dialog').focus();
        cy.realPress('Enter', {shiftKey: true});
        helper.copy();
        cy.cGet('#copy-paste-container p b').should('not.exist');
    });

    it('Enter key in replace field triggers replace', function() {
        helper.setDummyClipboardForCopy();

        //First make sure that we do not have 'replaced' text in current document
        helper.selectAllText();
        helper.copy();
        cy.cGet('#copy-paste-container').should('not.contain.text', 'replaced');

        helper.typeIntoDocument('{ctrl}h');
        findHelper.waitForFindReplaceDialog(this.win);

        // Search for text first
        cy.cGet('#searchterm-input-dialog').type('test').realPress('Enter');
        helper.textSelectionShouldExist();

        // Type replacement and press Enter
        cy.cGet('#replaceterm-input-dialog').type('replaced').realPress('Enter');

        // The replace triggers a jsdialog update and when the browser-side
        // processes the update the dialog gets rebuilt, and
        // .ui-dialog-titlebar-close temporarily disappears, so wait for the
        // replace operation round-trip and dialog rebuild to complete before
        // closing the dialog.
        helper.processToIdle(this.win);

        findHelper.closeFindDialog();

        // Select all text to verify replacement happened
        helper.selectAllText();
        helper.copy();

        // The clipboard should contain "replaced" somewhere
        cy.cGet('#copy-paste-container').should('contain.text', 'replaced');
    });

    it('Enter key on checkbox toggles it', function() {

        helper.typeIntoDocument('{ctrl}h');
        findHelper.waitForFindReplaceDialog(this.win);

        // Get initial state
        cy.cGet('#matchcase-input').should('not.be.checked');

        // Focus and press Enter
        cy.cGet('#matchcase-input').focus();
        cy.realPress('Enter');

        // Verify it's now checked
        cy.cGet('#matchcase-input').should('be.checked');

        // Press Enter again
        cy.realPress('Enter');

        // Verify it's unchecked
        cy.cGet('#matchcase-input').should('not.be.checked');
    });
});
