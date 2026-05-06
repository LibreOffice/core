/* global describe it cy beforeEach require */

var helper = require('../../common/helper');
var desktopHelper = require('../../common/desktop_helper');
var ceHelper = require('../../common/contenteditable_helper');
var writerHelper = require('../../common/writer_helper');

describe(['tagdesktop', 'tagnextcloud', 'tagproxy'], 'Preset tests.', function() {

	beforeEach(function() {
		helper.setupAndLoadDocument('writer/presets.odt');
		cy.cGet('div.clipboard').as('clipboard');
		helper.setDummyClipboardForCopy();
		desktopHelper.switchUIToNotebookbar();
	});

	// Ensure that shared autotext was installed.
	it('Verify shared autotext install.', function() {
		// Add some text and expand via autotext
		ceHelper.type('Flu');

		// F3
		cy.get('@clipboard').trigger('keydown', { key: "F3", code: "F3", which: 114, force: true })

		writerHelper.selectAllTextOfDoc();
		helper.copy();
		cy.cGet('#copy-paste-container p').should('contain.text', 'Flu is an infection caused by a virus.');
	});

	// Ensure that shared wordbook was installed.
	it.skip('Verify shared wordbook install.', function() {
		// Add some text and check spelling
		ceHelper.type('collaboraonline');

		cy.cGet('#Review-tab-label').click();

                cy.cGet('.notebookbar > .unoSpellingAndGrammarDialog > button').click();

		// we should end up with the "there are no misspelling information dialog"
		cy.cGet('.ui-dialog-title').should('have.text', 'Spelling: Information');

		cy.cGet('body').type('{esc}');
		cy.cGet('body').type('{esc}');
	});

	// Ensure that user autotext was installed.
	it('Verify user autotext install.', function() {
		// Add some text and expand via autotext
		ceHelper.type('Cold');

		// F3
		cy.get('@clipboard').trigger('keydown', { key: "F3", code: "F3", which: 114, force: true })

		writerHelper.selectAllTextOfDoc();
		helper.copy();
		cy.cGet('#copy-paste-container p').should('contain.text', 'The first symptom of a cold');
	});

	// Ensure that user wordbook was installed.
	it.skip('Verify user wordbook install.', function() {
		// Add some text and check spelling
		ceHelper.type('sneha');

		cy.cGet('#Review-tab-label').click();

                cy.cGet('.notebookbar > .unoSpellingAndGrammarDialog > button').click();

		// we should end up with the "there are no misspelling information dialog"
		cy.cGet('.ui-dialog-title').should('have.text', 'Spelling: Information');

		cy.cGet('body').type('{esc}');
		cy.cGet('body').type('{esc}');
	});
});
