/* global describe it cy beforeEach require */

var helper = require('../../common/helper');
var ceHelper = require('../../common/contenteditable_helper');

describe(['tagdesktop'], 'Wordbook upload.', function() {

	var documentFilePath;

	// The dev WOPI server only keeps a per-user preset store for a test that asks
	// for one with an explicit &userid; the default user ("test") is deliberately
	// not persisted (see skipPersist in test/TestWopiFileServer.hpp).
	var userId = 'wordbook-upload';
	var userQuery = 'userid=' + userId;

	// Where the dev WOPI server files a userconfig upload away. Mirrors
	// userPresetDir() in test/TestWopiFileServer.hpp. Relative paths resolve
	// against the cypress project root, which is cypress_test.
	var uploadedWordbook = '../test/data/presets/user/u-' + userId + '/standard.dic';

	var emptyWordbook = 'OOoUserDict1\nlang: <none>\ntype: positive\n---\n';

	// A word no dictionary knows, so the spelling checker is sure to flag it.
	var unknownWord = 'zzquixotry';

	beforeEach(function() {
		// Start from an empty dictionary in the store, so the assertion below
		// cannot pass on a file an earlier run left behind.
		cy.writeFile(uploadedWordbook, emptyWordbook);

		documentFilePath = helper.setupAndLoadDocument('writer/presets.odt', false, false,
			undefined, userQuery);
		cy.cGet('div.clipboard').as('clipboard');
	});

	// The upload runs while the last session is being removed, off the document's
	// own thread, so the reply can land after the document is already gone.
	it('A word added to the dictionary reaches the host after the document closes.', function() {
		ceHelper.type(unknownWord);

		// The word is only offered to a dictionary once the spelling checker has
		// flagged it, so let the document settle before opening the menu.
		cy.getFrameWindow().then(function(win) {
			return helper.processToIdle(win);
		});

		// Right-click inside the word. The cursor sits just after the last
		// letter, so step back into it.
		cy.cGet('.cursor-overlay .blinking-cursor').then(function(cursor) {
			var rect = cursor[0].getBoundingClientRect();
			cy.cGet('body').rightclick(rect.left - 20, rect.top + 5);
		});

		cy.cGet('#jsd-context-menu-dropdown-overlay').should('be.visible');
		cy.cGet('#jsd-context-menu-dropdown-overlay .ui-combobox-entry.ui-has-menu')
			.contains('span', 'Add to Dictionary')
			.trigger('mouseover');

		cy.cGet('#jsd-context-menu-dropdown-overlay .ui-combobox-entry')
			.contains('span', 'standard.dic')
			.click();

		helper.closeDocument(documentFilePath);

		// readFile retries, so this waits for the upload to arrive rather than
		// assuming it had finished by the time the view closed.
		cy.readFile(uploadedWordbook).should('contain', unknownWord);
	});
});
