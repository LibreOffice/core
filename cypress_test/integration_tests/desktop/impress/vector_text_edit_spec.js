/* -*- js-indent-level: 8 -*- */
/* global describe it cy beforeEach require expect */

var helper = require('../../common/helper');
var desktopHelper = require('../../common/desktop_helper');

// Text edits in the text box of a slide. Each case runs in a view drawn from tiles, where the
// server answers, and in one drawn from vector primitives, where the client answers from the
// cached text, and both reach the same result.
[
	{ name: 'tiles', vector: false },
	{ name: 'vector primitives', vector: true },
].forEach(function(view) {
	describe(['tagdesktop'], 'Text edits on a slide drawn from ' + view.name, function() {
		var win;
		// The text messages sent to the server, oldest first.
		var sent;

		beforeEach(function() {
			var path = helper.setupDocument('impress/apply_paragraph_props_text.odp');
			if (view.vector)
				helper.loadDocument(path, true, undefined, undefined, 'vector=true');
			else
				helper.loadDocument(path);
			cy.getFrameWindow().then(function(w) {
				win = w;
				// The stock load checks wait for tiles that a vector view lacks.
				cy.wrap(null).should(function() {
					expect(win.app.impress.partList).to.have.length.greaterThan(0);
				});
				return helper.processToIdle(win);
			});
			// With accessibility support the browser moves the caret of the text area
			// on the arrow keys.
			desktopHelper.setAccessibilityState(false);
			cy.then(function() {
				sent = [];
				var socket = win.app.socket;
				var doSend = socket._doSend;
				socket._doSend = function(message) {
					if (typeof message === 'string')
						sent.push(message);
					return doSend.apply(this, arguments);
				};
			});

			// A double click on the text box, at twips 794..15080 by 2160..7268, starts
			// the edit.
			cy.then(function() {
				var center = onCanvas(7937, 3000);
				cy.cGet('#document-canvas').dblclick(center.x, center.y);
			});
			cy.wrap(null).should(function() {
				expect(win.app.file.textCursor.visible).to.be.true;
			});
			keysToServer('{ctrl}{home}');
		});

		// The CSS pixel position on the document canvas of a point in document twips.
		function onCanvas(x, y) {
			var visible = win.app.activeDocument.activeLayout.viewedRectangle;
			var scale = win.app.twipsToPixels / win.app.dpiScale;
			return {
				x: x * scale - visible.pX1 / win.app.dpiScale,
				y: y * scale - visible.pY1 / win.app.dpiScale,
			};
		}

		// The caret once it rests and the server has answered for it.
		function settledCaret() {
			cy.then(function() { return helper.waitForTimers(win, 'clicktimer'); });
			cy.then(function() { return helper.waitForTimers(win, 'localcaretrest'); });
			cy.then(function() { return helper.processToIdle(win); });
			return cy.wrap(null).then(function() {
				var cursor = win.app.file.textCursor;
				return {
					x: cursor.rectangle.x1,
					y: (cursor.rectangle.y1 + cursor.rectangle.y2) / 2,
					logical: cursor.logical,
					anchor: cursor.anchor,
				};
			});
		}

		// Waits for the caret and the other end of the selection to settle where expected.
		function expectCaret(logical, anchor) {
			settledCaret();
			cy.wrap(null).should(function() {
				var cursor = win.app.file.textCursor;
				expect(cursor.logical).to.deep.equal(logical);
				expect(cursor.anchor || null).to.deep.equal(anchor || null);
			});
		}

		// Types keys that the server answers in both views.
		function keysToServer(keys) {
			helper.typeIntoDocument(keys);
			return settledCaret();
		}

		// True for a key message of an arrow key, whose UNO key code is 1024 to 1027 below
		// the modifier bits.
		function isArrowKey(message) {
			var match = /^key .*key=(\d+)/.exec(message);
			var code = match ? Number(match[1]) & 4095 : 0;
			return code >= 1024 && code <= 1027;
		}

		// Types arrow keys and checks the caret and selection they lead to. In a vector
		// view the client answers them, so no arrow key goes to the server.
		function expectKeys(keys, logical, anchor) {
			cy.then(function() { sent.length = 0; });
			helper.typeIntoDocument(keys);
			expectCaret(logical, anchor);
			cy.then(function() {
				if (view.vector)
					expect(sent.filter(isArrowKey)).to.deep.equal([]);
			});
		}

		function isPress(message) {
			return message.startsWith('mouse type=buttondown');
		}

		// Checks that a press in a vector view went to the server as no button message.
		function expectNoPressSent() {
			cy.then(function() {
				if (view.vector)
					expect(sent.filter(isPress)).to.deep.equal([]);
			});
		}

		// Checks the selected text through a copy of it.
		function expectSelectedText(text) {
			helper.copy();
			helper.expectTextForClipboard(text);
		}

		// Checks the text of the paragraph through a copy of it, after the keys.
		function expectParagraph(keys, text) {
			keysToServer(keys + '{home}{shift}{end}');
			expectSelectedText(text);
		}

		it('moves the caret by character', function() {
			expectKeys('{rightarrow}{rightarrow}{rightarrow}', { paragraph: 0, index: 3 });
			expectKeys('{leftarrow}', { paragraph: 0, index: 2 });
		});

		it('puts the caret where a click lands', function() {
			var before, after;
			keysToServer('{rightarrow}{rightarrow}').then(function(caret) { before = caret; });
			keysToServer('{rightarrow}').then(function(caret) { after = caret; });
			keysToServer('{end}');

			// A third of the way from offset 2 to offset 3, so offset 2 is the closest.
			cy.then(function() {
				sent.length = 0;
				var at = onCanvas(before.x + (after.x - before.x) / 3, before.y);
				cy.cGet('#document-canvas').click(at.x, at.y);
			});
			expectCaret({ paragraph: 0, index: 2 });
			expectNoPressSent();
		});

		it('moves the caret by line and across paragraphs', function() {
			expectKeys('{rightarrow}{rightarrow}', { paragraph: 0, index: 2 });
			expectKeys('{downarrow}', { paragraph: 1, index: 2 });
			expectKeys('{uparrow}', { paragraph: 0, index: 2 });
			keysToServer('{end}');
			expectKeys('{rightarrow}', { paragraph: 1, index: 0 });
			expectKeys('{leftarrow}', { paragraph: 0, index: 6 });
		});

		it('selects text with Shift and an arrow', function() {
			expectKeys('{shift}{rightarrow}{rightarrow}', { paragraph: 0, index: 2 }, { paragraph: 0, index: 0 });
			expectSelectedText('Te');
		});

		it('selects the text a drag goes over', function() {
			var start, end;
			settledCaret().then(function(caret) { start = caret; });
			keysToServer('{end}').then(function(caret) { end = caret; });
			cy.then(function() {
				sent.length = 0;
				var from = onCanvas(start.x, start.y);
				var to = onCanvas(end.x, end.y);
				cy.cGet('#document-canvas')
					.trigger('mousedown', from.x, from.y, { button: 0, buttons: 1 })
					.trigger('mousemove', (from.x + to.x) / 2, to.y, { button: 0, buttons: 1 })
					.trigger('mousemove', to.x, to.y, { button: 0, buttons: 1 })
					.trigger('mouseup', to.x, to.y, { button: 0, buttons: 0 });
			});
			expectCaret({ paragraph: 0, index: 6 }, { paragraph: 0, index: 0 });
			expectNoPressSent();
			expectSelectedText('Text 1');
		});

		it('moves the caret by word', function() {
			expectKeys('{ctrl}{rightarrow}', { paragraph: 0, index: 5 });
			expectKeys('{ctrl}{rightarrow}', { paragraph: 1, index: 0 });
			expectKeys('{ctrl}{leftarrow}', { paragraph: 0, index: 6 });
			expectKeys('{ctrl}{leftarrow}', { paragraph: 0, index: 5 });
			keysToServer('{home}');
			expectKeys('{ctrl}{shift}{rightarrow}', { paragraph: 0, index: 5 }, { paragraph: 0, index: 0 });
			expectSelectedText('Text ');
		});

		// Clicks the document canvas count times at (x, y), in CSS pixels, with no time
		// between the clicks, so they count together however busy the machine is.
		function clickTimes(x, y, count) {
			cy.cGet('#document-canvas').then(function(canvas) {
				var element = canvas[0];
				var frame = element.ownerDocument.defaultView;
				var bounds = element.getBoundingClientRect();
				for (var detail = 1; detail <= count; detail++) {
					['mousedown', 'mouseup', 'click'].forEach(function(type) {
						element.dispatchEvent(new frame.MouseEvent(type, {
							bubbles: true,
							cancelable: true,
							view: frame,
							clientX: bounds.left + x,
							clientY: bounds.top + y,
							button: 0,
							buttons: type === 'mousedown' ? 1 : 0,
							detail: detail,
						}));
					});
				}
			});
		}

		it('selects a word on a double click and a paragraph on a triple click', function() {
			var middle;
			keysToServer('{rightarrow}{rightarrow}').then(function(caret) { middle = caret; });
			keysToServer('{end}');

			cy.then(function() {
				sent.length = 0;
				var at = onCanvas(middle.x, middle.y);
				cy.cGet('#document-canvas').dblclick(at.x, at.y);
			});
			expectCaret({ paragraph: 0, index: 4 }, { paragraph: 0, index: 0 });
			expectNoPressSent();

			cy.then(function() {
				sent.length = 0;
				var at = onCanvas(middle.x, middle.y);
				clickTimes(at.x, at.y, 3);
			});
			expectCaret({ paragraph: 0, index: 6 }, { paragraph: 0, index: 0 });
			expectNoPressSent();
		});

		it('replaces or deletes the selected text', function() {
			keysToServer('{shift}{rightarrow}{rightarrow}{rightarrow}{rightarrow}');
			helper.typeIntoDocument('Word');
			expectParagraph('', 'Word 1');

			keysToServer('{downarrow}{home}{shift}{rightarrow}{rightarrow}');
			helper.typeIntoDocument('{backspace}');
			expectParagraph('', 'xt 2');
		});

		// Holds the messages from the server until the keys are typed, so the answer to the
		// first key arrives after the last one.
		function typeWithAnswersHeld(keys) {
			var held = [];
			var onMessage;
			cy.then(function() {
				var socket = win.app.socket;
				onMessage = socket._onMessage;
				socket._onMessage = function() { held.push(arguments); };
			});
			helper.typeIntoDocument(keys);
			cy.then(function() {
				var socket = win.app.socket;
				socket._onMessage = onMessage;
				held.forEach(function(message) { onMessage.apply(socket, message); });
			});
		}

		it('keeps typed text, deletions and caret moves in the order they were made', function() {
			// The outbox keeps four messages in flight, so the "e" waits to be sent
			// while the caret moves back over all five letters.
			keysToServer('{end}');
			cy.then(function() { sent.length = 0; });
			typeWithAnswersHeld('abcde{leftarrow}{leftarrow}{leftarrow}{leftarrow}{leftarrow}');
			expectParagraph('', 'Text 1abcde');
			cy.then(function() {
				if (!view.vector)
					return;
				var e = sent.findIndex(function(message) {
					return /^textinput id=0 text=\w*e /.test(message);
				});
				var cursor = sent.findIndex(function(message) {
					return message.startsWith('textcursor ');
				});
				expect(e).to.be.at.least(0);
				expect(cursor).to.be.greaterThan(e);
			});

			keysToServer('{downarrow}{end}');
			typeWithAnswersHeld('xyz{leftarrow}{del}{backspace}');
			expectParagraph('', 'Text 2x');
		});
	});
});
