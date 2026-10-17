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
	});
});
