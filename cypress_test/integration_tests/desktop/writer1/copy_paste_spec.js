/* global describe it cy require expect */

var helper = require('../../common/helper');

describe(['tagdesktop', 'tagnextcloud', 'tagproxy'], 'Clipboard operations.', function() {

	it('Copy and Paste text.', function() {
		helper.setupAndLoadDocument('writer/copy_paste.odt');
		// Select some text
		helper.selectAllText();

		cy.getFrameWindow().then(win => {
			const selectionStart = win.TextSelections.getStartRectangle();
			cy.cGet('#document-container').rightclick(selectionStart.pX1, selectionStart.pY1);
		});

		helper.setDummyClipboardForCopy();

		helper.getContextMenuItem('Copy').click();

		cy.cGet('#copy-paste-container div p').should('have.text', 'text');
	});

	it('Copy plain text.', function() {
		helper.setupAndLoadDocument('writer/copy_paste_simple.odt');

		helper.setDummyClipboardForCopy('text/plain');
		helper.selectAllText();
		helper.copy();

		let expected = '    • first\n    • second\n    • third';
		cy.cGet('#copy-plain-container').should('have.text', expected);
	});

	it('Copy text as markdown.', function() {
		// Given a document with 3 words: middle word is italic:
		helper.setupAndLoadDocument('writer/copy_markdown.odt');
		cy.getFrameWindow().then(function(win) {
			cy.stub(win.parent, 'postMessage').as('postMessage');
		});

		// When copying the document text as markdown:
		helper.selectAllText();
		cy.getFrameWindow().then(function(win) {
			// Same as using framed.doc.html's "Send a message" frame:
			// - message set to 'Action_Copy'
			// - values set to '{"Mimetype": "text/markdown;charset=utf-8"}'
			const message = {
				'MessageId': 'Action_Copy',
				'Values': {
					'Mimetype': 'text/markdown;charset=utf-8'
				}
			};
			win.postMessage(JSON.stringify(message), '*');
		});

		// Then make sure we get markdown:
		// Without the accompanying fix in place, this test would have failed with:
		// expected postMessage to have been called at least once, but it was never called
		cy.get('@postMessage').should('be.called');
		cy.get('@postMessage').should(stub => {
			const json = JSON.parse(stub.firstCall.args[0]);
			expect(json.MessageId).to.equal('Action_Copy_Resp');
			expect(json.Values.content).to.equal('foo *bar* baz\n');
		});
	});

	it('Copy and Paste text with DisableCopy', function () {
		helper.setupAndLoadDocument('writer/copy_paste.odt', /* isMultiUser */ false, /* copy .wopi.json */ true);
		// Select some text
		helper.selectAllText();

		cy.getFrameWindow().then(win => {
			const selectionStart = win.TextSelections.getStartRectangle();
			cy.cGet('#document-container').rightclick(selectionStart.pX1, selectionStart.pY1);
		});

		helper.setDummyClipboardForCopy();

		const copyEntry = helper.getContextMenuItem('Copy');
		copyEntry.should('be.visible');
		copyEntry.click();

		// With DisableCopy active we should not copy to clipboard
		cy.cGet('#copy-paste-container div p').should('not.have.text', 'text');

		// But paste should still work properly
		helper.typeIntoDocument('{end}');
		helper.getCursorPos('left', 'beforePaste');
		cy.getFrameWindow().then(win => {
			win.app.map.sendUnoCommand('.uno:Paste');
			helper.processToIdle(win);
		});
		helper.getCursorPos('left', 'afterPaste');
		cy.get('@beforePaste').then(beforeValue => {
			cy.get('@afterPaste').should('be.gt', beforeValue);
		});

	});
});
