/* global describe it cy require expect */

var helper = require('../../common/helper');

function captureKeyEventCalls(win) {
	var calls = [];
	win.app.map._docLayer.postKeyboardEvent = function (type, charCode, unoKeyCode) {
		calls.push({type: type, unoKeyCode: unoKeyCode});
	};
	return calls;
}

function dispatchBackspace(win, modifiers) {
	var target = win.document.querySelector('div.clipboard');
	var evt = new win.KeyboardEvent('keydown', Object.assign({
		key: 'Backspace', code: 'Backspace', keyCode: 8, which: 8,
		bubbles: true, cancelable: true,
	}, modifiers));
	target.dispatchEvent(evt);
}

describe(['tagdesktop'], 'Backspace modifiers on macOS', function () {
	it('sends the same word-delete modifier as Ctrl+Backspace for Option+Backspace', function () {
		helper.setupAndLoadDocument('writer/testfile.fodt');

		cy.getFrameWindow().then(function (win) {
			// Simulate a macOS client: Map.Keyboard reads this to remap Option+Backspace.
			win.L.Browser.mac = true;
			var calls = captureKeyEventCalls(win);

			dispatchBackspace(win, {altKey: true});

			expect(calls.length).to.equal(1);

			// Before the fix, this was sent with the Alt modifier, which the document core
			// has no delete binding for (Alt+Backspace is reserved there as an Undo
			// accelerator this key event never reaches), so Option+Backspace silently did
			// nothing. It must now carry the same Ctrl modifier as Ctrl+Backspace, which the
			// core already deletes the previous word for.
			expect(calls[0].unoKeyCode & win.app.UNOModifier.CTRL).to.equal(win.app.UNOModifier.CTRL);
			expect(calls[0].unoKeyCode & win.app.UNOModifier.ALT).to.equal(0);
		});
	});

	it('sends a delete-to-paragraph-start key for Cmd+Backspace', function () {
		helper.setupAndLoadDocument('writer/testfile.fodt');

		cy.getFrameWindow().then(function (win) {
			win.L.Browser.mac = true;
			var calls = captureKeyEventCalls(win);

			dispatchBackspace(win, {metaKey: true});

			expect(calls.length).to.equal(1);

			// Before the fix, this carried the Ctrl modifier, same as Ctrl+Backspace, which
			// only deletes the previous word. Cmd+Backspace deletes back to the start of the
			// line on macOS, so it must now send the core's dedicated delete-to-paragraph-
			// start key instead.
			expect(calls[0].unoKeyCode).to.equal(win.UNOKey.DELETE_TO_BEGIN_OF_PARAGRAPH);
		});
	});

	it('leaves Backspace modifiers alone on other platforms', function () {
		helper.setupAndLoadDocument('writer/testfile.fodt');

		cy.getFrameWindow().then(function (win) {
			win.L.Browser.mac = false;
			var calls = captureKeyEventCalls(win);

			dispatchBackspace(win, {altKey: true});
			expect(calls.length).to.equal(1);
			expect(calls[0].unoKeyCode & win.app.UNOModifier.ALT).to.equal(win.app.UNOModifier.ALT);
			expect(calls[0].unoKeyCode & win.app.UNOModifier.CTRL).to.equal(0);

			calls.length = 0;
			dispatchBackspace(win, {ctrlKey: true});
			expect(calls.length).to.equal(1);
			expect(calls[0].unoKeyCode).to.not.equal(win.UNOKey.DELETE_TO_BEGIN_OF_PARAGRAPH);
			expect(calls[0].unoKeyCode & win.app.UNOModifier.CTRL).to.equal(win.app.UNOModifier.CTRL);
		});
	});
});
