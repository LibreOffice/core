/* global describe it cy require beforeEach expect */

var helper = require('../../common/helper');

describe(['tagmultiuser'], 'Multiuser Writer Cross-Session Paste', function() {

	beforeEach(function() {
		// Same document opened by two users (two sessions on one docbroker).
		helper.setupAndLoadDocument('writer/copy_paste.odt', true);
	});

	// Regression test for the copy/paste flow where the copying tab is closed
	// before the paste happens, while the same document stays open in another
	// tab. The copied content must survive in the server's saved-clipboard
	// cache so the surviving tab can still fetch it. Without the rescue on
	// disconnect (DocumentBroker::removeSession), the clipboard GET 400s and no
	// paste is issued.
	it('Paste works after the copying session disconnects', function() {
		var stubHtml;

		// User A selects the text and copies it. The dummy clipboard captures
		// the copy without touching the real system clipboard.
		cy.cSetActiveFrame('#iframe1');
		helper.selectAllText();
		helper.setDummyClipboardForCopy('text/html');
		helper.copy();

		// The copy is async; wait for the captured html (with the meta-origin
		// that carries User A's clipboard tag) to land.
		cy.cGet('#copy-paste-container').should(function($el) {
			expect($el.html()).to.contain('meta-origin');
		});

		// Build a stub payload carrying User A's clipboard origin. A stub has
		// no inline body, so the paste cannot fall back to local html and MUST
		// fetch the content from the server - the path the bug breaks. This is
		// what the app itself puts on the clipboard for a large/complex copy.
		cy.getFrameWindow().then(function(win) {
			stubHtml = win.app.map._clip._getStubHtml();
		});

		// User A closes the tab: its session goes away while User B keeps the
		// document open. close() drops the socket without reconnecting.
		cy.getFrameWindow().then(function(win) {
			win.app.socket.close();
		});

		// User B observes User A's view leave. By the time the view list drops
		// to one, the server has already rescued User A's clipboard into the
		// cache (getclipboard is forwarded before the disconnect handshake).
		cy.cSetActiveFrame('#iframe2');
		cy.getFrameWindow().should(function(win) {
			expect(Object.keys(win.app.map._viewInfo)).to.have.length(1);
		});

		// Give User B a cursor in the document.
		cy.cGet('#document-container').click();

		// Spy on User B's outgoing messages: a paste UNO command is only issued
		// once the cross-session clipboard fetch (and the upload into User B's
		// kit) succeed.
		cy.getFrameWindow().then(function(win) {
			cy.spy(win.app.socket, 'sendMessage').as('iframe2send');
		});

		// User B pastes User A's clipboard.
		cy.getFrameWindow().then(function(win) {
			win.app.map._clip.paste({
				clipboardData: {
					getData: function(t) {
						return t === 'text/html' ? stubHtml : '';
					},
					types: ['text/html'],
				},
				preventDefault: function() {},
			});
		});

		// Primary guard: the paste reached the point of issuing .uno:Paste,
		// which only happens if the saved clipboard was found on the server.
		cy.get('@iframe2send').should(function(spy) {
			var issued = spy.getCalls().some(function(call) {
				return typeof call.args[0] === 'string' &&
					call.args[0].indexOf('.uno:Paste') >= 0;
			});
			expect(issued, 'User B issued a paste UNO command').to.be.true;
		});

		// Confidence check: the text was actually inserted. Copy the whole
		// document from User B (its #copy-paste-container only fills on copy,
		// and the shared helper targets the now-closed iframe1, so wire a dummy
		// clipboard here) and confirm it holds two copies of the original word.
		cy.getFrameWindow().then(function(win) {
			return helper.processToIdle(win);
		});
		cy.getFrameWindow().then(function(win) {
			var clip = win.app.map._clip;
			clip._dummyClipboard = {
				write: function(clipboardItems) {
					clipboardItems[0].getType('text/html')
						.then(function(blob) { return blob.text(); })
						.then(function(text) { clip._dummyDiv.innerHTML = text; });
					return { then: function(resolve) { resolve(); } };
				},
			};
		});
		helper.selectAllText();
		cy.getFrameWindow().then(function(win) {
			win.app.map._clip.filterExecCopyPaste('.uno:Copy');
		});
		// Read only the paragraph text (not the whole container, which also
		// carries the trailing plain-text part of the clipboard payload).
		helper.expectTextForClipboard('texttext');
	});
});
