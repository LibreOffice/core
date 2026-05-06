/* global describe it cy beforeEach require */

var helper = require('../../common/helper');

describe(['tagdesktop', 'tagnextcloud', 'tagproxy'], 'Writer text selection deselect redraw.', function() {

	beforeEach(function() {
		helper.setupAndLoadDocument('writer/copy_paste.odt');
		cy.viewport(1920, 1080);
	});

	// Regression test for: https://github.com/CollaboraOnline/online/pull/15068
	// Bug: When the cursor is at the end of the document and select-all
	// is performed, deselecting (click or arrow key) clears the selection
	// but the screen fails to redraw, leaving stale selection visuals.
	it('Deselect after select-all with cursor at end of document.', function() {
		// Move cursor to the end of the document.
		helper.typeIntoDocument('{ctrl}{end}');

		// Select all text.
		helper.selectAllText();

		// Verify text selection div exists (drawn on canvas).
		cy.cGet('[id^="test-div-"][id$="-text-selections"]').should('exist');

		// Press down arrow to deselect.
		helper.typeIntoDocument('{downarrow}');

		cy.getFrameWindow().then(function(win) {
			helper.processToIdle(win);
		});

		// After deselection, the text selection div should be removed.
		cy.cGet('[id^="test-div-"][id$="-text-selections"]').should('not.exist');
	});

	it('Deselect with End key after select-all with cursor at end of document.', function() {
		// Move cursor to the end of the document.
		helper.typeIntoDocument('{ctrl}{end}');

		// Select all text.
		helper.selectAllText();

		// Verify text selection div exists (drawn on canvas).
		cy.cGet('[id^="test-div-"][id$="-text-selections"]').should('exist');

		// Press End key to deselect and move cursor to end.
		helper.typeIntoDocument('{end}');

		cy.getFrameWindow().then(function(win) {
			helper.processToIdle(win);
		});

		// After deselection, the text selection div should be removed.
		cy.cGet('[id^="test-div-"][id$="-text-selections"]').should('not.exist');
	});
});
