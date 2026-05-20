/* global describe it cy require beforeEach expect */

var helper = require('../../common/helper');

describe(['tagmultiuser'], 'Multiuser Slide Cross-Document Paste', function() {

	beforeEach(function() {
		helper.setupAndLoadTwoDocuments(
			'impress/slide_operations.odp',
			'impress/slide_operations.odp');
	});

	it('Right-click copy in file A and Ctrl+V paste in file B inserts the slide', function() {
		// File B starts with a single slide.
		cy.cSetActiveFrame('#iframe2');
		cy.cGet('#preview-frame-part-0').should('exist');
		cy.cGet('#preview-frame-part-1').should('not.exist');

		// Install a dummy clipboard in file A's Clipboard.js that captures
		// the html written by the right-click "Copy" path. We later replay
		// that html into file B's paste handler, the same flow Ctrl+V uses
		// through document.onpaste.
		cy.cSetActiveFrame('#iframe1');
		helper.setDummyClipboardForCopy('text/html');

		// Right-click the slide preview and pick "Copy" from the menu.
		cy.cGet('#preview-frame-part-0 img').rightclick();
		cy.cGet('[id$="-dropdown"]:visible')
			.contains('.ui-combobox-entry', 'Copy').click();

		// The copy is async (it fetches /clipboard, builds a ClipboardItem,
		// then writes). Wait for the html to land in the dummy div.
		cy.cGet('#copy-paste-container').should(function($el) {
			expect($el.html()).to.contain('meta-origin');
		});

		// Read the captured html out of file A's iframe.
		cy.cSetActiveFrame('#iframe1');
		cy.cGet('#copy-paste-container').invoke('html').then(function(html) {
			// Simulate Ctrl+V in file B: dispatch the same paste event the
			// browser would deliver, with file A's clipboard html attached.
			cy.cSetActiveFrame('#iframe2');
			cy.getFrameWindow().then(function(win) {
				win.app.map._clip.paste({
					clipboardData: {
						getData: function(t) {
							return t === 'text/html' ? html : '';
						},
						types: ['text/html'],
					},
					preventDefault: function() {},
				});
			});
		});

		// After paste, file B must have a second slide preview.
		cy.cSetActiveFrame('#iframe2');
		cy.cGet('#preview-frame-part-1', { timeout: 20000 }).should('exist');
	});
});
