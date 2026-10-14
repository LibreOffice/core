/* global describe it cy expect beforeEach require */

var helper = require('../../common/helper');
var findHelper = require('../../common/find_helper');
var impressHelper = require('../../common/impress_helper');

describe(['tagdesktop'], 'Searching via find dialog' ,function() {

	beforeEach(function() {
		helper.setupAndLoadDocument('impress/search_bar.odp');
		cy.getFrameWindow().then((win) => {
			this.win = win;
		});
	});

	it('Search existing word.', function() {
		helper.setDummyClipboardForCopy();
		findHelper.openFindDialog(this.win);
		findHelper.typeIntoSearchField('a');

		findHelper.findNext(this.win);
		findHelper.closeFindDialog(); // we cant copy the document text while the dialog is still open
		// A shape and some text should be selected
		//cy.get('.transform-handler--rotate')
		//	.should('be.not.visible');

		helper.textSelectionShouldExist();

		helper.copy();
		helper.expectTextForClipboard('a');
	});

	it('Search not existing word.', function() {
		cy.cGet('#document-container').dblclick('center');
		impressHelper.assertInTextEditMode();
		helper.selectAllText();
		helper.textSelectionShouldExist();

		findHelper.openFindDialog(this.win);
		findHelper.typeIntoSearchField('q');
		findHelper.findNext(this.win);

		helper.textSelectionShouldNotExist();
	});

	it('Search next / prev instance.', function() {
		helper.setDummyClipboardForCopy();
		findHelper.openFindDialog(this.win);
		findHelper.typeIntoSearchField('a');

		findHelper.findNext(this.win);
		findHelper.closeFindDialog();

		// A shape and some text should be selected
		//cy.get('.transform-handler--rotate')
		//	.should('be.not.visible');

		helper.getCursorPos('left', 'cursorOrigLeft');

		helper.textSelectionShouldExist();
		helper.copy();
		helper.expectTextForClipboard('a');

		// Search next instance
		findHelper.openFindDialog(this.win);
		findHelper.typeIntoSearchField('a');
		findHelper.findNext(this.win);
		findHelper.closeFindDialog();

		//cy.get('.transform-handler--rotate')
		//	.should('be.not.visible');

		helper.textSelectionShouldExist();
		helper.copy();
		helper.expectTextForClipboard('a');

		cy.get('@cursorOrigLeft')
			.then(function(cursorOrigLeft) {
				cy.cGet('.blinking-cursor')
					.should(function(cursor) {
						expect(cursor.offset().left).to.be.greaterThan(cursorOrigLeft);
					});
			});

		// Search prev instance
		findHelper.openFindDialog(this.win);
		findHelper.typeIntoSearchField('a');
		findHelper.findPrev(this.win);
		findHelper.closeFindDialog();

		//cy.get('.transform-handler--rotate')
		//	.should('be.not.visible');

		helper.textSelectionShouldExist();
		helper.copy();
		helper.expectTextForClipboard('a');

		cy.get('@cursorOrigLeft')
			.then(function(cursorOrigLeft) {
				cy.cGet('.blinking-cursor')
					.should(function(cursor) {
						expect(cursor.offset().left).to.be.equal(cursorOrigLeft);
					});
			});
	});

	it('Search wrap at the document end.', function() {
		helper.setDummyClipboardForCopy();
		findHelper.openFindDialog(this.win);
		findHelper.typeIntoSearchField('a');

		findHelper.findNext(this.win);
		findHelper.closeFindDialog();

		// A shape and some text should be selected
		//cy.get('.transform-handler--rotate')
		//	.should('be.not.visible');
		helper.textSelectionShouldExist();
		helper.copy();
		helper.expectTextForClipboard('a');

		helper.getCursorPos('left', 'cursorOrigLeft');

		// Search next instance
		findHelper.openFindDialog(this.win);
		findHelper.typeIntoSearchField('a');
		findHelper.findNext(this.win);
		findHelper.closeFindDialog();

		//cy.get('.transform-handler--rotate')
		//	.should('be.not.visible');
		helper.textSelectionShouldExist();
		helper.copy();
		helper.expectTextForClipboard('a');

		cy.get('@cursorOrigLeft')
			.then(function(cursorOrigLeft) {
				cy.cGet('.blinking-cursor')
					.should(function(cursor) {
						expect(cursor.offset().left).to.be.greaterThan(cursorOrigLeft);
					});
			});

		// Search next instance, which is in the beginning of the document.
		findHelper.openFindDialog(this.win);
		findHelper.typeIntoSearchField('a');
		findHelper.findNext(this.win);
		findHelper.closeFindDialog();

		//cy.get('.transform-handler--rotate')
		//	.should('be.not.visible');
		helper.textSelectionShouldExist();
		helper.copy();
		helper.expectTextForClipboard('a');

		cy.get('@cursorOrigLeft')
			.then(function(cursorOrigLeft) {
				cy.cGet('.blinking-cursor')
					.should(function(cursor) {
						expect(cursor.offset().left).to.be.equal(cursorOrigLeft);
					});
			});
	});
});
