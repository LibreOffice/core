/* global describe it cy beforeEach require */

var helper = require('../../common/helper');
var desktopHelper = require('../../common/desktop_helper');

var dimDialogSelector = '#modal-dialog-inactive_user_message-overlay';

// A comment is written in a text area that floats above the document. These tests cover what
// the idle handler makes of the time an author spends in there.
describe(['tagdesktop'], 'Idle while a comment is open', function() {
	beforeEach(function() {
		helper.setupAndLoadDocument('writer/annotation.odt');
		// The document already comes up in the notebookbar. This tells the helpers which
		// path to take when they insert the comment.
		desktopHelper.switchUIToNotebookbar();
	});

	function commentTextArea() {
		return cy.cGet('.cool-annotation').last({log: false})
			.find('.modify-annotation .cool-annotation-textarea');
	}

	it('Typing in a comment keeps the document awake', function() {
		desktopHelper.insertComment('Comment', false);

		// The idle timeout is 7 seconds in this suite, so five rounds of this carry us
		// well past it without ever touching the document itself.
		for (var i = 0; i < 5; i++) {
			cy.wait(2000);
			commentTextArea().type(String(i));
			cy.cGet(dimDialogSelector).should('not.exist');
		}

		commentTextArea().should('contain', 'Comment01234');
	});

	it('A comment left alone still lets the document go idle', function() {
		desktopHelper.insertComment('Comment', false);

		cy.cGet(dimDialogSelector).should('not.exist');
		cy.wait(7100);
		cy.cGet(dimDialogSelector).should('exist');
	});
});
