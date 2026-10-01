/* global describe it cy beforeEach require expect */

var helper = require('../../common/helper');
var impressHelper = require('../../common/impress_helper');

// The notes that one user edits in the speaker notes pane are held by that user. The other users
// see those notes with the name of that user, and can edit them once that user leaves them.
describe(['tagmultiuser'], 'Impress speaker notes pane with two users', function () {
	var win1, win2;
	var editEngine = '#notespanel-container .ui-editengine';
	var paragraph = editEngine + ' .ui-editengine-paragraph';

	beforeEach(function () {
		helper.setupAndLoadDocument('impress/empty-placeholder.fodp', true);
		cy.getFrameWindow('#iframe1').then(function (win) { win1 = win; });
		cy.getFrameWindow('#iframe2').then(function (win) { win2 = win; });
	});

	// Both frames share one browser window. Each edit of the notes changes the selection of the
	// documents, and a document whose selection changes sets the focus on itself, which takes the
	// focus from the notes of the other frame. A frame whose document nobody types into keeps its
	// document away from the focus.
	function keepDocumentFromFocus(win) {
		cy.then(function () {
			cy.stub(win.app.map, 'focus');
		});
	}

	// Each view is idle before its pane opens, so the pane opens after the join of the other user.
	// The engine creates the pane of a view while that view is the active one, and the view of the
	// user who sent the last message is the active one. Each idle check is a message of this user,
	// so the checks repeat until the pane is there.
	function openNotesPane(win) {
		helper.processToIdle(win);
		cy.then(function () {
			win.app.dispatcher.dispatch('notespanel');
		});
		helper.retryUntil(
			function () { helper.processToIdle(win); },
			function () { return win.app.map.notesPanel.isVisible(); });
		cy.cGet('#notespanel-dock-wrapper').should('be.visible');
		cy.cGet(editEngine).should('exist');
		helper.processToIdle(win);
	}

	// Cypress sees the document iframe as the focused element, so its blur() refuses to act on
	// the editor. The element itself is blurred instead.
	function leaveNotes() {
		cy.cGet(editEngine).then(function (element) {
			element[0].blur();
		});
	}

	it('notes one user edits are read-only for the other until the first user leaves', function () {
		keepDocumentFromFocus(win1);
		keepDocumentFromFocus(win2);

		cy.cSetActiveFrame('#iframe1');
		openNotesPane(win1);
		cy.cSetActiveFrame('#iframe2');
		openNotesPane(win2);
		cy.cGet(editEngine).should('have.attr', 'contenteditable', 'true');

		// User A types into the notes.
		cy.cSetActiveFrame('#iframe1');
		cy.cGet(editEngine).click();
		helper.processToIdle(win1);
		cy.cGet(editEngine).type('Alpha');
		cy.cGet(paragraph).should('have.text', 'Alpha');

		// User B sees the text of user A, cannot edit it, and is told who is editing.
		cy.cSetActiveFrame('#iframe2');
		cy.cGet(paragraph).should('have.text', 'Alpha');
		cy.cGet(editEngine).should('have.attr', 'contenteditable', 'false');
		cy.cGet(editEngine).should(function (element) {
			var nameOfA = win2.app.map.getViewName(win1.app.map._docLayer._viewId);
			expect(nameOfA).to.be.a('string').and.not.be.empty;
			expect(element[0].getAttribute('data-locked-by')).to.equal(nameOfA + ' is editing');
		});

		// Once user A leaves the notes, user B can add to them.
		cy.cSetActiveFrame('#iframe1');
		leaveNotes();

		cy.cSetActiveFrame('#iframe2');
		cy.cGet(editEngine).should('have.attr', 'contenteditable', 'true');
		cy.cGet(editEngine).should('not.have.attr', 'data-locked-by');
		cy.cGet(editEngine).click();
		helper.processToIdle(win2);
		cy.cGet(editEngine).type('{end} beta');
		cy.cGet(paragraph).should('have.text', 'Alpha beta');

		// Now user A sees the text of user B and cannot edit it.
		cy.cSetActiveFrame('#iframe1');
		cy.cGet(paragraph).should('have.text', 'Alpha beta');
		cy.cGet(editEngine).should('have.attr', 'contenteditable', 'false');
		cy.cGet(editEngine).should('have.attr', 'data-locked-by');
	});

	it('notes edited on the handout page are read-only in the pane of the other user', function () {
		keepDocumentFromFocus(win2);

		cy.cSetActiveFrame('#iframe2');
		openNotesPane(win2);

		// User A edits the notes text on the handout page.
		cy.cSetActiveFrame('#iframe1');
		helper.processToIdle(win1);
		cy.then(function () {
			win1.app.map.sendUnoCommand('.uno:NotesMode');
		});
		helper.processToIdle(win1);
		cy.cGet('#document-container').then(function (items) {
			expect(items).to.have.length(1);
			var rect = items[0].getBoundingClientRect();
			var x = (rect.left + rect.right) / 2;
			var y = rect.top + (rect.bottom - rect.top) * 0.75;
			cy.cGet('body').dblclick(x, y);
		});
		impressHelper.assertInTextEditMode();
		helper.typeIntoDocument('Handout');

		// The pane of user B cannot edit those notes, and says who is editing them.
		cy.cSetActiveFrame('#iframe2');
		cy.cGet(editEngine).should('have.attr', 'contenteditable', 'false');
		cy.cGet(editEngine).should(function (element) {
			var nameOfA = win2.app.map.getViewName(win1.app.map._docLayer._viewId);
			expect(nameOfA).to.be.a('string').and.not.be.empty;
			expect(element[0].getAttribute('data-locked-by')).to.equal(nameOfA + ' is editing');
		});

		// Once user A leaves the text, user B gets the text and can edit it.
		cy.cSetActiveFrame('#iframe1');
		helper.typeIntoDocument('{esc}');

		cy.cSetActiveFrame('#iframe2');
		cy.cGet(editEngine).should('have.attr', 'contenteditable', 'true');
		cy.cGet(editEngine).should('not.have.attr', 'data-locked-by');
		cy.cGet(paragraph).should('have.text', 'Handout');
	});
});
