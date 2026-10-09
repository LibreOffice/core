/* global describe it cy beforeEach require */

const helper = require('../../common/helper');

describe(['tagdesktop', 'tagnextcloud', 'tagproxy'], 'Socket message types', function() {

	beforeEach(function() {
		helper.setupAndLoadDocument('writer/undo_redo.odt');
		helper.setDummyClipboardForCopy();
	});

	it('Every message type arrives intact and in order', function() {
		cy.getFrameWindow().then(function(win) {
			helper.processToIdle(win);
		});

		// Each message inserts its own token. They are sent in one go, so the proxy socket puts
		// them into a single request, and a message whose size is declared wrong misframes the
		// ones after it as well. The objects come from the document's window, as the socket
		// tells the binary types apart with instanceof.
		cy.getFrameWindow().then(function(win) {
			const encoder = new win.TextEncoder();
			const command = function(token) {
				return 'uno .uno:InsertText {"Text":{"type":"string","value":"' + token + ';"}}';
			};
			const bytes = function(token) {
				return encoder.encode(command(token));
			};
			// A view over the middle of a larger buffer: only the view's bytes are the message.
			const padded = function(token) {
				const inner = bytes(token);
				const outer = new win.Uint8Array(inner.length + 8);
				outer.fill(0x21);
				outer.set(inner, 4);
				return { buffer: outer.buffer, offset: 4, length: inner.length, array: outer };
			};

			const dataView = padded('dataview');
			const subarray = padded('subarray');
			const messages = [
				command('ascii'),
				// Multi-byte UTF-8: the size is in bytes, not in characters.
				command('árvíztűrő'),
				new win.Blob([command('blob')]),
				bytes('arraybuffer').buffer,
				bytes('uint8'),
				new win.Int8Array(bytes('int8').buffer),
				new win.DataView(dataView.buffer, dataView.offset, dataView.length),
				subarray.array.subarray(subarray.offset, subarray.offset + subarray.length),
				command('last'),
			];

			for (const message of messages)
				win.app.socket.sendMessage(message);

			helper.processToIdle(win);
		});

		helper.selectAllText();
		helper.copy();
		helper.expectTextForClipboard(
			'ascii;árvíztűrő;blob;arraybuffer;uint8;int8;dataview;subarray;last;');
	});
});
