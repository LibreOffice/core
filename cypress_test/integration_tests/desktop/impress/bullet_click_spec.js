/* -*- js-indent-level: 8 -*- */
/* global describe it cy beforeEach require expect */

var helper = require('../../common/helper');
var desktopHelper = require('../../common/desktop_helper');

// Clicks on the bullet of an item in the outline of a slide, and on the space beside it. Each
// case runs in a view drawn from tiles, where the server answers, and in one drawn from vector
// primitives, where the client answers what it can, and both reach the same result.
[
	{ name: 'tiles', vector: false },
	{ name: 'vector primitives', vector: true },
].forEach(function(view) {
	describe(['tagdesktop'], 'Bullet clicks on a slide drawn from ' + view.name, function() {
		var win;
		// The text messages sent to the server, oldest first.
		var sent;
		// The caret in front of the text "xaxax" of the only item, in document twips.
		var start;

		beforeEach(function() {
			var path = helper.setupDocument('impress/search_bar.odp');
			if (view.vector) {
				helper.loadDocument(path, true, undefined, undefined, 'vector=true');
				helper.documentChecks(true);
			} else
				helper.loadDocument(path);
			cy.getFrameWindow().then(function(w) {
				win = w;
				cy.wrap(null).should(function() {
					expect(win.app.impress.partList).to.have.length.greaterThan(0);
				});
				return helper.processToIdle(win);
			});
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

			// A double click on the outline, at twips 794..15080 by 2089..7267, starts the
			// edit.
			cy.then(function() {
				var center = onCanvas(7937, 5000);
				cy.cGet('#document-canvas').dblclick(center.x, center.y);
			});
			cy.wrap(null).should(function() {
				expect(win.app.file.textCursor.visible).to.be.true;
			});
			helper.typeIntoDocument('{ctrl}{home}');
			settledCaret().then(function(caret) { start = caret; });
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
				var rectangle = win.app.file.textCursor.rectangle;
				return {
					x: rectangle.x1,
					y: (rectangle.y1 + rectangle.y2) / 2,
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

		// Waits for the whole text "xaxax" of the item to be selected, whichever end the caret
		// is at.
		function expectItemSelected() {
			settledCaret();
			cy.wrap(null).should(function() {
				var cursor = win.app.file.textCursor;
				var ends = [cursor.logical, cursor.anchor].map(function(end) {
					return end ? end.paragraph + ':' + end.index : null;
				}).sort();
				expect(ends).to.deep.equal(['0:0', '0:5']);
			});
		}

		/*
			The label in front of the text is 0.9 cm, 510 twips, wide and the bullet stands at
			its left end. The bullet is drawn at 45 percent of the size of the text, so it ends
			about 300 twips before the text, and the strip that selects the item a tenth of the
			line height after that.
		*/
		function onBullet() {
			return onCanvas(start.x - 420, start.y);
		}

		function besideBullet() {
			return onCanvas(start.x - 100, start.y);
		}

		// Sends the events of one click at (x, y), in CSS pixels on the document canvas, with
		// detail as the count the system gives it.
		function clickWithDetail(at, detail) {
			cy.cGet('#document-canvas').then(function(canvas) {
				var element = canvas[0];
				var frame = element.ownerDocument.defaultView;
				var bounds = element.getBoundingClientRect();
				['mousedown', 'mouseup', 'click'].forEach(function(type) {
					element.dispatchEvent(new frame.MouseEvent(type, {
						bubbles: true,
						cancelable: true,
						view: frame,
						clientX: bounds.left + at.x,
						clientY: bounds.top + at.y,
						button: 0,
						buttons: type === 'mousedown' ? 1 : 0,
						detail: detail,
					}));
				});
			});
		}

		// Moves the mouse to (x, y), in CSS pixels on the document canvas. The canvas follows
		// the mouse only once the mouse has entered it, so the move is a real one, which the
		// browser starts with an enter.
		function moveTo(at) {
			cy.cGet('#document-canvas').realMouseMove(at.x, at.y);
		}

		function expectPointer(pointer) {
			cy.cGet('#document-canvas').should(function($canvas) {
				expect($canvas[0].style.cursor).to.equal(pointer);
			});
		}

		it('selects the item on a click on its bullet', function() {
			helper.typeIntoDocument('{end}');
			expectCaret({ paragraph: 0, index: 5 });

			cy.then(function() { clickWithDetail(onBullet(), 1); });
			expectItemSelected();
		});

		it('puts the caret in front of the text on a click beside the bullet', function() {
			helper.typeIntoDocument('{end}');
			expectCaret({ paragraph: 0, index: 5 });

			cy.then(function() {
				sent.length = 0;
				clickWithDetail(besideBullet(), 1);
			});
			expectCaret({ paragraph: 0, index: 0 });
			cy.then(function() {
				// The client answers the click itself.
				if (view.vector)
					expect(sent.filter(function(message) {
						return message.startsWith('mouse type=buttondown');
					})).to.deep.equal([]);
			});
		});

		it('keeps the item selected on a triple click on its bullet', function() {
			helper.typeIntoDocument('{end}');
			expectCaret({ paragraph: 0, index: 5 });

			cy.then(function() {
				var at = onBullet();
				for (var detail = 1; detail <= 3; detail++)
					clickWithDetail(at, detail);
			});
			expectItemSelected();
		});

		it('shows the move pointer on the bullet and the text pointer beside it', function() {
			// The client sets the pointer only when it draws from the vector primitives.
			if (!view.vector)
				return;

			cy.then(function() { moveTo(onBullet()); });
			expectPointer('move');
			cy.then(function() { moveTo(besideBullet()); });
			expectPointer('text');
		});
	});
});
