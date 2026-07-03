/* global describe it cy beforeEach require expect */

var helper = require('../../common/helper');
var desktopHelper = require('../../common/desktop_helper');
var ceHelper = require('../../common/contenteditable_helper');

// A predictive-text, autocorrect or IME engine rewrites the whole editable
// area in a single input event: it changes text in the middle of the buffer
// while leaving the caret at the end. Core's caret is also at the end, because
// every earlier key was sent there.
//
// TextInput._onInput (used when accessibility is disabled) trims the common
// suffix from the update. If it trims when the caret is at the end rather than
// just before the suffix, the backward removal count is measured from the wrong
// place and the wrong characters are deleted, so the tail is corrupted.
describe(['taga11yenabled'], 'TextInput - middle edit with caret at the end', function() {

	beforeEach(function () {
		helper.setupAndLoadDocument('writer/undo_redo.odt');
		cy.cGet('div.clipboard').as('clipboard');
	});

	// Rewrite the whole editable-area content in one shot and fire a single
	// 'input' event, with the caret left at the end of the text, exactly as an
	// autocorrect or predictive-text engine does. This bypasses per-key caret
	// updates, so core's caret stays where the last real keystroke left it.
	function autocorrectKeepingCaretAtEnd(newContent) {
		cy.get('@clipboard').then(($c) => {
			const clip = $c[0];
			const doc = clip.ownerDocument;
			const win = doc.defaultView;

			clip.innerHTML = clip._wrapContent(newContent);

			// collapse the caret at the very end of the text
			const walker = doc.createTreeWalker(clip, win.NodeFilter.SHOW_TEXT, null);
			let node = null;
			let lastTextNode = null;
			while ((node = walker.nextNode()) !== null)
				lastTextNode = node;
			const sel = win.getSelection();
			sel.removeAllRanges();
			const range = doc.createRange();
			range.setStart(lastTextNode, lastTextNode.textContent.length);
			range.collapse(true);
			sel.addRange(range);
			expect(clip._getSelectionStart(), 'caret is at the end of the buffer').to.eq(newContent.length);

			clip.dispatchEvent(new win.InputEvent('input', {
				inputType: 'insertReplacementText',
				data: newContent,
				bubbles: true,
				cancelable: false,
			}));
		});
	}

	it('autocorrecting a word leaves the following text intact', function () {
		helper.setDummyClipboardForCopy();

		// Exercise the non-accessibility input path, which reconstructs edits by
		// matching a common prefix and suffix.
		desktopHelper.setAccessibilityState(false);
		cy.cGet('div.clipboard').as('clipboard');

		// Type a misspelled word followed by more text. Both the browser caret
		// and core's caret end up after the last character.
		ceHelper.type('teh dog');
		ceHelper.checkPlainContent('teh dog');

		// Autocorrect rewrites "teh" to "the" without moving the caret.
		autocorrectKeepingCaretAtEnd('the dog');

		// The document must read "the dog". If the common suffix is trimmed while
		// the caret is at the end, core deletes "og" and inserts "he", producing
		// "teh dhe" instead.
		helper.selectAllText();
		helper.copy();
		cy.wait(500);
		helper.expectTextForClipboard('The dog');
	});
});
