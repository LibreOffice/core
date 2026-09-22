/* global describe it cy beforeEach require expect */

var helper = require('../../common/helper');
var writerHelper = require('../../common/writer_helper');

describe(['tagdesktop'], 'Localized keyboard shortcuts', function() {

	beforeEach(function() {
		cy.viewport(1280, 1024);
	});

	it('English Ctrl+Alt+F still inserts a footnote', function() {
		helper.setupAndLoadDocument('writer/find_replace.odt');

		helper.typeIntoDocument('xxxx');

		// Document-space cursor top in twips, independent of scroll position.
		// Retry until core has sent a real cursor position (the rectangle
		// starts zeroed).
		var cursorOrigTop;
		cy.getFrameWindow().its('app.file.textCursor.rectangle.y1')
			.should('be.greaterThan', 0)
			.then((top) => { cursorOrigTop = top; });

		helper.typeIntoDocument('{ctrl}{alt}f');

		// The footnote moves the cursor down into the footnote area.
		cy.getFrameWindow().its('app.file.textCursor.rectangle.y1')
			.should((top) => { expect(top).to.be.greaterThan(cursorOrigTop); });
		cy.cGet('#FindReplaceDialog').should('not.exist');
	});

	// The menu names the key that German uses, not the one English uses
	// for the same command.
	it('German labels Find and Replace with the key that opens it', function() {
		helper.setupAndLoadDocument('writer/find_replace.odt', false, false, 'de');

		cy.getFrameWindow().then((win) => {
			expect(win.JSDialog.ShortcutsUtil.getShortcutText('.uno:SearchDialog'))
				.to.equal('Strg+Alt+F');
			// Ctrl+Alt+F is taken, so the footnote entry names no key.
			expect(win.JSDialog.ShortcutsUtil.getShortcutText('.uno:InsertFootnote'))
				.to.be.undefined;
		});
	});

	// German LibreOffice gives Ctrl+1 to line spacing 1, Online applies the
	// heading style there in every language.
	it('German Ctrl+1 applies the first heading style', function() {
		helper.setupAndLoadDocument('writer/find_replace.odt', false, false, 'de');
		helper.setDummyClipboardForCopy();

		helper.typeIntoDocument('xxxx');
		helper.typeIntoDocument('{ctrl}1');

		writerHelper.selectAllTextOfDoc();
		helper.copy();
		cy.cGet('#copy-paste-container h1').should('exist');
	});
});
