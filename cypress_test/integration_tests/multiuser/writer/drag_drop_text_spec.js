/* global describe it cy beforeEach require expect */

var helper = require('../../common/helper');

// The caret rectangle of the active frame in twips, read once the caret has left the given
// rectangle.
function readCaretAfterMove(previous) {
	var caret = {};
	cy.getFrameWindow().should(function(frameWindow) {
		var rectangle = frameWindow.app.file.textCursor.rectangle;
		expect(rectangle).to.exist;
		if (previous)
			expect([rectangle.x1, rectangle.y1]).to.not.deep.equal([previous.x1, previous.y1]);
		caret.x1 = rectangle.x1;
		caret.y1 = rectangle.y1;
		caret.y2 = rectangle.y2;
	});
	return caret;
}

// The screen position, inside the given frame window, of a document point given in twips.
function screenPoint(win, x, y) {
	var anchor = win.app.sectionContainer.getDocumentAnchor();
	var viewed = win.app.activeDocument.activeLayout.viewedRectangle;
	var bounds = win.document.getElementById('canvas-container').getBoundingClientRect();
	return {
		x: bounds.left + (anchor[0] + x * win.app.twipsToPixels - viewed.pX1) / win.app.dpiScale,
		y: bounds.top + (anchor[1] + y * win.app.twipsToPixels - viewed.pY1) / win.app.dpiScale,
	};
}

// The middle of a line, in twips, between the two caret rectangles.
function between(start, end) {
	return { x: (start.x1 + end.x1) / 2, y: (start.y1 + start.y2) / 2 };
}

// Measures, in the active frame, the end of the first paragraph and the middle of the second
// and the third. A modifier stays held for the rest of the typed string, so each key goes alone.
function measureParagraphs(win) {
	var points = {};
	helper.typeIntoDocument('{ctrl}{home}');
	helper.typeIntoDocument('{end}');
	helper.processToIdle(win);
	var endOfFirst = readCaretAfterMove(null);
	helper.typeIntoDocument('{downarrow}');
	helper.typeIntoDocument('{end}');
	helper.processToIdle(win);
	var endOfSecond = readCaretAfterMove(endOfFirst);
	helper.typeIntoDocument('{home}');
	helper.processToIdle(win);
	var startOfSecond = readCaretAfterMove(endOfSecond);
	helper.typeIntoDocument('{ctrl}{end}');
	helper.processToIdle(win);
	var endOfThird = readCaretAfterMove(startOfSecond);
	helper.typeIntoDocument('{home}');
	helper.processToIdle(win);
	var startOfThird = readCaretAfterMove(endOfThird);
	cy.then(function() {
		points.endOfFirst = { x: endOfFirst.x1, y: (endOfFirst.y1 + endOfFirst.y2) / 2 };
		points.insideSecond = between(startOfSecond, endOfSecond);
		points.insideThird = between(startOfThird, endOfThird);
	});
	return points;
}

// Selects the second or the last paragraph of the active frame with the keyboard.
function selectParagraph(paragraph) {
	helper.typeIntoDocument(paragraph === 'last' ? '{ctrl}{end}' : '{ctrl}{home}');
	if (paragraph === 'second')
		helper.typeIntoDocument('{downarrow}');
	helper.typeIntoDocument('{home}');
	helper.typeIntoDocument('{shift}{end}');
	helper.textSelectionShouldExist();
}

// Drags with the real mouse inside the active frame, in five steps, so the first steps are too
// short to start the drag and the later ones start it. The points, in twips, come from functions,
// because they are measured by commands that run after this one is queued.
function realDrag(win, getFrom, getTo, release) {
	cy.then(function() {
		var from = getFrom();
		var to = getTo();
		var start = screenPoint(win, from.x, from.y);
		var end = screenPoint(win, to.x, to.y);
		cy.cGet('body').realMouseDown({ x: start.x, y: start.y });
		for (let step = 1; step <= 5; ++step) {
			cy.cGet('body').realMouseMove(start.x + (end.x - start.x) * step / 5,
				start.y + (end.y - start.y) * step / 5);
		}
		if (release)
			cy.cGet('body').realMouseUp({ x: end.x, y: end.y });
	});
	helper.processToIdle(win);
}

// The text of the first paragraph of the active frame, through a copy.
function expectFirstParagraph(text) {
	helper.setDummyClipboardForCopy();
	helper.typeIntoDocument('{ctrl}{home}');
	helper.typeIntoDocument('{shift}{end}');
	helper.textSelectionShouldExist();
	helper.copy();
	helper.expectTextForClipboard(text);
}

describe(['tagmultiuser'], 'Multiuser Writer drag and drop of a text selection', function() {

	beforeEach(function() {
		// Three paragraphs: Test1, Test3, Test2, opened by two users.
		helper.setupAndLoadDocument('writer/drag_drop_text.fodt', true);
		cy.cSetActiveFrame('#iframe1');
		cy.getFrameWindow().then((win) => {
			this.win1 = win;
		});
		cy.cSetActiveFrame('#iframe2');
		cy.getFrameWindow().then((win) => {
			this.win2 = win;
		});
	});

	it('A drag that waits for another user\'s drag keeps the selection and says why', function() {
		var win1 = this.win1;
		var win2 = this.win2;

		cy.cSetActiveFrame('#iframe1');
		var points = measureParagraphs(win1);
		selectParagraph('last');

		cy.cSetActiveFrame('#iframe2');
		selectParagraph('second');

		// The first user drags Test2 towards the end of Test1 and holds the button.
		cy.cSetActiveFrame('#iframe1');
		realDrag(win1, () => points.insideThird, () => points.endOfFirst, false);
		cy.cGet('.text-drop-caret').should('be.visible');

		// The second user drags Test3 meanwhile. Its mouse messages go to the document the way its
		// browser sends them, because the test has only one real mouse, which the first user holds.
		cy.cSetActiveFrame('#iframe2');
		cy.then(function() {
			var send = function(type, x, y) {
				win2.app.socket.sendMessage('mouse type=' + type + ' x=' + Math.round(x) +
					' y=' + Math.round(y) + ' count=1 buttons=1 modifier=0');
			};
			var from = points.insideSecond;
			var to = points.endOfFirst;
			send('buttondown', from.x, from.y);
			for (let step = 1; step <= 5; ++step)
				send('move', from.x + (to.x - from.x) * step / 5, from.y + (to.y - from.y) * step / 5);
			send('buttonup', to.x, to.y);
		});
		helper.processToIdle(win2);

		// Then the second user is told why nothing moves, and keeps the selection of Test3, the
		// second paragraph.
		cy.cGet('#snackbar').should('be.visible')
			.and('contain.text', 'Another user is moving text in this document.');
		cy.getFrameWindow().should(function(win) {
			var view = win.app.activeDocument.activeView;
			var inside = function(point) {
				return view.selectionContains(new win.cool.SimplePoint(point.x, point.y));
			};
			expect(inside(points.insideSecond)).to.be.true;
			expect(inside(points.insideThird)).to.be.false;
		});

		// When the first user drops, Test2 lands at the end of Test1.
		cy.cSetActiveFrame('#iframe1');
		cy.then(function() {
			var end = screenPoint(win1, points.endOfFirst.x, points.endOfFirst.y);
			cy.cGet('body').realMouseUp({ x: end.x, y: end.y });
		});
		helper.processToIdle(win1);
		expectFirstParagraph('Test1Test2');
	});

	it('Two users drag one after the other', function() {
		var win1 = this.win1;
		var win2 = this.win2;

		cy.cSetActiveFrame('#iframe1');
		var points = measureParagraphs(win1);

		// The first user moves Test2 to the end of Test1.
		selectParagraph('last');
		realDrag(win1, () => points.insideThird, () => points.endOfFirst, true);
		expectFirstParagraph('Test1Test2');

		// Then the second user moves Test3 to the end of the first paragraph, which now ends after
		// Test2.
		cy.cSetActiveFrame('#iframe2');
		helper.typeIntoDocument('{ctrl}{home}');
		helper.typeIntoDocument('{end}');
		helper.processToIdle(win2);
		var endOfFirst = readCaretAfterMove(null);
		selectParagraph('second');
		realDrag(win2, () => points.insideSecond,
			() => ({ x: endOfFirst.x1, y: (endOfFirst.y1 + endOfFirst.y2) / 2 }), true);

		// Both users see the same document. A copy in the second frame of this harness comes back
		// empty, so the first frame reads the result.
		cy.cSetActiveFrame('#iframe1');
		expectFirstParagraph('Test1Test2Test3');
	});
});
