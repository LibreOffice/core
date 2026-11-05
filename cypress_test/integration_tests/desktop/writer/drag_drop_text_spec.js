/* global describe it cy beforeEach require expect */

var helper = require('../../common/helper');
var desktopHelper = require('../../common/desktop_helper');

// The caret rectangle in twips, read once the caret has left the given rectangle.
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

// The screen position of a document point given in twips.
function screenPoint(win, x, y) {
	var anchor = win.app.sectionContainer.getDocumentAnchor();
	var viewed = win.app.activeDocument.activeLayout.viewedRectangle;
	var bounds = win.document.getElementById('canvas-container').getBoundingClientRect();
	return {
		x: bounds.left + (anchor[0] + x * win.app.twipsToPixels - viewed.pX1) / win.app.dpiScale,
		y: bounds.top + (anchor[1] + y * win.app.twipsToPixels - viewed.pY1) / win.app.dpiScale,
	};
}

// Measures, in twips, the end of Test1 and the middle of Test2, then selects Test2 with a
// double-click, which shows the context toolbar. A modifier stays held for the rest of the typed
// string, so each key goes alone.
function selectTest2WithMouse(win) {
	var points = {};
	helper.assertCursorAndFocus();
	helper.typeIntoDocument('{ctrl}{home}');
	helper.typeIntoDocument('{end}');
	helper.processToIdle(win);
	var dropTarget = readCaretAfterMove(null);

	helper.typeIntoDocument('{ctrl}{end}');
	helper.processToIdle(win);
	var endOfTest2 = readCaretAfterMove(dropTarget);

	helper.typeIntoDocument('{home}');
	helper.processToIdle(win);
	var startOfTest2 = readCaretAfterMove(endOfTest2);

	// A mouse selection shows the context toolbar, the keyboard one does not.
	cy.then(function() {
		points.endOfTest1 = { x: dropTarget.x1, y: (dropTarget.y1 + dropTarget.y2) / 2 };
		points.insideTest2 = {
			x: (startOfTest2.x1 + endOfTest2.x1) / 2,
			y: (startOfTest2.y1 + startOfTest2.y2) / 2,
		};
		var middle = screenPoint(win, points.insideTest2.x, points.insideTest2.y);
		cy.cGet('body').dblclick(middle.x, middle.y);
	});
	helper.textSelectionShouldExist();
	helper.processToIdle(win);
	helper.waitForTimers(win, 'clicktimer');
	cy.cGet('#context-toolbar').should('not.have.class', 'hidden');
	return points;
}

// Presses the mouse button inside Test2 and drags towards the end of Test1 in five steps, so
// the first steps are too short to start the drag and the later ones start it. The button stays
// down.
function dragTest2TowardsTest1(win, points) {
	cy.then(function() {
		var start = screenPoint(win, points.insideTest2.x, points.insideTest2.y);
		var end = screenPoint(win, points.endOfTest1.x, points.endOfTest1.y);
		cy.cGet('body').realMouseDown({ x: start.x, y: start.y });
		for (let step = 1; step <= 5; ++step) {
			cy.cGet('body').realMouseMove(start.x + (end.x - start.x) * step / 5,
				start.y + (end.y - start.y) * step / 5);
		}
	});
	helper.processToIdle(win);
	cy.cGet('.text-drop-caret').should('be.visible');
	cy.cGet('#context-toolbar').should('have.class', 'hidden');
}

describe(['tagdesktop'], 'Writer drag and drop of a text selection.', function() {

	beforeEach(function() {
		// Three paragraphs: Test1, Test3, Test2.
		helper.setupAndLoadDocument('writer/drag_drop_text.fodt');
		cy.getFrameWindow().then((win) => {
			this.win = win;
		});
	});

	it('Dragging the selected Test2 to the end of Test1 moves it there', function() {
		var win = this.win;
		var points = selectTest2WithMouse(win);

		// While the button is held, the drop caret marks where the text will land, and the
		// toolbar of the dragged selection is gone.
		dragTest2TowardsTest1(win, points);

		cy.then(function() {
			var end = screenPoint(win, points.endOfTest1.x, points.endOfTest1.y);
			cy.cGet('body').realMouseUp({ x: end.x, y: end.y });
		});
		helper.processToIdle(win);

		cy.cGet('.text-drop-caret').should('not.be.visible');

		helper.setDummyClipboardForCopy();
		helper.typeIntoDocument('{ctrl}{home}');
		helper.typeIntoDocument('{shift}{end}');
		helper.textSelectionShouldExist();
		helper.copy();
		helper.expectTextForClipboard('Test1Test2');
	});

	it('A short drag with Shift from inside the selection extends it to the release point', function() {
		var win = this.win;

		// The caret after "Te" in the paragraph Test2. A modifier stays held for the rest of the
		// typed string, so each key goes alone.
		helper.assertCursorAndFocus();
		helper.typeIntoDocument('{ctrl}{end}');
		helper.typeIntoDocument('{home}');
		helper.processToIdle(win);
		var startOfTest2 = readCaretAfterMove(null);
		for (let i = 0; i < 2; ++i)
			helper.typeIntoDocument('{rightarrow}');
		helper.processToIdle(win);
		var afterTe = readCaretAfterMove(startOfTest2);

		var points = selectTest2WithMouse(win);

		// When Shift is held, the button is pressed just after "Te" inside the selection, and comes
		// up 8 pixels further on: a drag for the canvas, which starts one at 5 pixels, but closer
		// than a clear drag of the selection. A button held still long enough also makes a clear
		// drag, and a slow test machine could hold it that long, so the test lengthens that time
		// while it drags and measures the distance alone.
		var mouseControlClass;
		var holdTime;
		cy.then(function() {
			mouseControlClass = win.app.activeDocument.mouseControl.constructor;
			holdTime = mouseControlClass.selectionDragHoldTime;
			mouseControlClass.selectionDragHoldTime = 600000;
			var press = screenPoint(win, afterTe.x1, points.insideTest2.y);
			press.x += 1;
			cy.cGet('body').realMouseDown({ x: press.x, y: press.y, shiftKey: true });
			cy.cGet('body').realMouseMove(press.x + 4, press.y, { shiftKey: true });
			cy.cGet('body').realMouseMove(press.x + 8, press.y, { shiftKey: true });
			cy.cGet('body').realMouseUp({ x: press.x + 8, y: press.y, shiftKey: true });
		});
		cy.then(function() {
			mouseControlClass.selectionDragHoldTime = holdTime;
		});
		helper.processToIdle(win);

		// Then the selection ends past the press point, at the release point:
		helper.setDummyClipboardForCopy();
		helper.copy();
		cy.cGet('#copy-paste-container p').should(function($paragraph) {
			expect($paragraph.text()).to.match(/^Test?$/);
		});
	});

	it('A drag that comes back to the selection keeps it and its context toolbar', function() {
		var win = this.win;
		var points = selectTest2WithMouse(win);
		dragTest2TowardsTest1(win, points);

		// When the selection is brought back and released on itself:
		cy.then(function() {
			var start = screenPoint(win, points.insideTest2.x, points.insideTest2.y);
			var end = screenPoint(win, points.endOfTest1.x, points.endOfTest1.y);
			for (let step = 4; step >= 0; --step) {
				cy.cGet('body').realMouseMove(start.x + (end.x - start.x) * step / 5,
					start.y + (end.y - start.y) * step / 5);
			}
			cy.cGet('body').realMouseUp({ x: start.x, y: start.y });
		});
		helper.processToIdle(win);

		// Then Test2 is still selected, with its toolbar:
		helper.textSelectionShouldExist();
		cy.cGet('#context-toolbar').should('not.have.class', 'hidden');
	});

	it('Escape during a drag keeps the selection and gives its context toolbar back', function() {
		var win = this.win;
		var points = selectTest2WithMouse(win);
		dragTest2TowardsTest1(win, points);

		// When Escape is pressed while the button is still down, and the button then comes up:
		cy.realPress('Escape');
		helper.processToIdle(win);
		cy.then(function() {
			var end = screenPoint(win, points.endOfTest1.x, points.endOfTest1.y);
			cy.cGet('body').realMouseUp({ x: end.x, y: end.y });
		});
		helper.processToIdle(win);

		// Then Test2 is still selected, with its toolbar:
		helper.textSelectionShouldExist();
		cy.cGet('#context-toolbar').should('not.have.class', 'hidden');
	});
});

describe(['tagdesktop'], 'Writer drag and drop of a text selection past the visible area.', function() {

	beforeEach(function() {
		// Test1, Test2, 120 filler lines and End: about three pages.
		helper.setupAndLoadDocument('writer/drag_drop_text_long.fodt');
		cy.getFrameWindow().then((win) => {
			this.win = win;
		});
	});

	it('Holding a dragged selection at the bottom edge scrolls the view so it can be dropped further down', function() {
		var win = this.win;

		helper.assertCursorAndFocus();
		// A modifier stays held for the rest of the typed string, so each key goes alone.
		helper.typeIntoDocument('{ctrl}{end}');
		helper.processToIdle(win);
		var endOfDocument = readCaretAfterMove(null);

		helper.typeIntoDocument('{ctrl}{home}');
		helper.processToIdle(win);
		var startOfDocument = readCaretAfterMove(endOfDocument);
		// The view is back at the top, so End is out of sight again.
		cy.getFrameWindow().should(function(frameWindow) {
			var viewed = frameWindow.app.activeDocument.activeLayout.viewedRectangle;
			expect(viewed.y2).to.be.below(endOfDocument.y1);
		});
		helper.typeIntoDocument('{downarrow}');
		helper.typeIntoDocument('{end}');
		helper.processToIdle(win);
		var endOfTest2 = readCaretAfterMove(startOfDocument);
		helper.typeIntoDocument('{home}');
		helper.processToIdle(win);
		var startOfTest2 = readCaretAfterMove(endOfTest2);
		helper.typeIntoDocument('{shift}{end}');
		helper.textSelectionShouldExist();
		helper.processToIdle(win);

		// Drag Test2 to just above the bottom edge of the document area and hold it there, inside
		// the band where the view scrolls by itself.
		cy.then(function() {
			var start = screenPoint(win, (startOfTest2.x1 + endOfTest2.x1) / 2,
				(startOfTest2.y1 + startOfTest2.y2) / 2);
			var bounds = win.document.getElementById('canvas-container').getBoundingClientRect();
			var edge = { x: start.x, y: bounds.bottom - 15 };

			cy.cGet('body').realMouseDown({ x: start.x, y: start.y });
			for (let step = 1; step <= 5; ++step) {
				cy.cGet('body').realMouseMove(start.x, start.y + (edge.y - start.y) * step / 5);
			}
		});

		// The view scrolls while the mouse stands still, until End comes into sight.
		cy.getFrameWindow().should(function(frameWindow) {
			var viewed = frameWindow.app.activeDocument.activeLayout.viewedRectangle;
			expect(viewed.y2).to.be.above(endOfDocument.y2);
		});

		// Move onto the end of End, which stops the scrolling, and drop there.
		cy.then(function() {
			var target = screenPoint(win, endOfDocument.x1, (endOfDocument.y1 + endOfDocument.y2) / 2);
			cy.cGet('body').realMouseMove(target.x, target.y);
			helper.processToIdle(win);
			cy.cGet('.text-drop-caret').should('be.visible');
			cy.cGet('body').realMouseUp({ x: target.x, y: target.y });
		});
		helper.processToIdle(win);

		helper.setDummyClipboardForCopy();
		helper.typeIntoDocument('{ctrl}{end}');
		helper.typeIntoDocument('{shift}{home}');
		helper.textSelectionShouldExist();
		helper.copy();
		helper.expectTextForClipboard('EndTest2');
	});
});

// The screen position of a document point given in twips, as the active view layout shows it.
function layoutScreenPoint(win, x, y) {
	var point = new win.cool.SimplePoint(x, y);
	var bounds = win.app.sectionContainer.getCanvasBoundingClientRect();
	return {
		x: point.vX / win.app.dpiScale + bounds.x,
		y: point.vY / win.app.dpiScale + bounds.y,
	};
}

describe(['tagdesktop'], 'Writer drag and drop of a text selection in the multi-page view.', function() {

	beforeEach(function() {
		cy.viewport(1920, 1080);
		// Six pages, one paragraph each: Page one to Page six.
		helper.setupAndLoadDocument('writer/six_pages.fodt');
		cy.getFrameWindow().then((win) => {
			this.win = win;
			win.app.dispatcher.dispatch('multipageview');
			return helper.processToIdle(win);
		});
	});

	it('Dragging the selected word of the last page to the start of its line moves it there', function() {
		var win = this.win;
		cy.getFrameWindow().should(function(frameWindow) {
			expect(frameWindow.app.activeDocument.activeLayout.type).to.equal('ViewLayoutMultiPage');
		});

		// A modifier stays held for the rest of the typed string, so each key goes alone.
		helper.assertCursorAndFocus();
		helper.typeIntoDocument('{ctrl}{end}');
		helper.processToIdle(win);
		var endOfSix = readCaretAfterMove(null);
		helper.typeIntoDocument('{ctrl}{leftarrow}');
		helper.processToIdle(win);
		var startOfSix = readCaretAfterMove(endOfSix);
		helper.typeIntoDocument('{home}');
		helper.processToIdle(win);
		var startOfLine = readCaretAfterMove(startOfSix);

		// A mouse selection shows the context toolbar.
		var inside;
		cy.then(function() {
			inside = layoutScreenPoint(win, (startOfSix.x1 + endOfSix.x1) / 2,
				(startOfSix.y1 + startOfSix.y2) / 2);
			cy.cGet('body').dblclick(inside.x, inside.y);
		});
		helper.textSelectionShouldExist();
		helper.processToIdle(win);
		helper.waitForTimers(win, 'clicktimer');
		cy.cGet('#context-toolbar').should('not.have.class', 'hidden');

		// When the selection is dragged from inside it to the start of the line:
		cy.then(function() {
			var end = layoutScreenPoint(win, startOfLine.x1, (startOfLine.y1 + startOfLine.y2) / 2);
			cy.cGet('body').realMouseDown({ x: inside.x, y: inside.y });
			for (let step = 1; step <= 5; ++step) {
				cy.cGet('body').realMouseMove(inside.x + (end.x - inside.x) * step / 5,
					inside.y + (end.y - inside.y) * step / 5);
			}
			helper.processToIdle(win);

			// Then the drag is one of the selection: the drop caret shows, and the toolbar of the
			// dragged selection is gone.
			cy.cGet('.text-drop-caret').should('be.visible');
			cy.cGet('#context-toolbar').should('have.class', 'hidden');

			cy.cGet('body').realMouseUp({ x: end.x, y: end.y });
		});
		helper.processToIdle(win);

		// And the word now starts the line:
		helper.setDummyClipboardForCopy();
		helper.typeIntoDocument('{ctrl}{end}');
		helper.typeIntoDocument('{shift}{home}');
		helper.textSelectionShouldExist();
		helper.copy();
		helper.expectTextForClipboard('sixPage ');
	});
});

describe(['tagdesktop'], 'Writer drag and drop of a text selection over a shape.', function() {

	beforeEach(function() {
		// Three paragraphs: Test1 followed by a rectangle in the same line, Test3, Test2.
		helper.setupAndLoadDocument('writer/drag_drop_text_shape.fodt');
		cy.getFrameWindow().then((win) => {
			this.win = win;
		});
	});

	it('Over the shape the drop caret goes away, and a release there moves nothing', function() {
		var win = this.win;

		// The rectangle lies between the end of "Test1" and the end of the first paragraph. A
		// modifier stays held for the rest of the typed string, so each key goes alone.
		helper.assertCursorAndFocus();
		helper.typeIntoDocument('{ctrl}{home}');
		helper.processToIdle(win);
		var startOfTest1 = readCaretAfterMove(null);
		for (let i = 0; i < 5; ++i)
			helper.typeIntoDocument('{rightarrow}');
		helper.processToIdle(win);
		var endOfTest1Text = readCaretAfterMove(startOfTest1);
		helper.typeIntoDocument('{end}');
		helper.processToIdle(win);
		var endOfFirstParagraph = readCaretAfterMove(endOfTest1Text);
		helper.typeIntoDocument('{downarrow}');
		helper.typeIntoDocument('{end}');
		helper.processToIdle(win);
		var endOfTest3 = readCaretAfterMove(endOfFirstParagraph);
		helper.typeIntoDocument('{ctrl}{end}');
		helper.processToIdle(win);
		var endOfTest2 = readCaretAfterMove(endOfTest3);
		helper.typeIntoDocument('{home}');
		helper.processToIdle(win);
		var startOfTest2 = readCaretAfterMove(endOfTest2);

		cy.then(function() {
			var middle = screenPoint(win, (startOfTest2.x1 + endOfTest2.x1) / 2,
				(startOfTest2.y1 + startOfTest2.y2) / 2);
			cy.cGet('body').dblclick(middle.x, middle.y);
		});
		helper.textSelectionShouldExist();
		helper.processToIdle(win);
		helper.waitForTimers(win, 'clicktimer');

		// When Test2 is dragged from inside it to the end of Test3, where the drop caret shows,
		// and on to the rectangle, with the button held:
		var onShape;
		cy.then(function() {
			var start = screenPoint(win, (startOfTest2.x1 + endOfTest2.x1) / 2,
				(startOfTest2.y1 + startOfTest2.y2) / 2);
			var overText = screenPoint(win, endOfTest3.x1, (endOfTest3.y1 + endOfTest3.y2) / 2);
			onShape = screenPoint(win, (endOfTest1Text.x1 + endOfFirstParagraph.x1) / 2,
				(endOfFirstParagraph.y1 + endOfFirstParagraph.y2) / 2);
			cy.cGet('body').realMouseDown({ x: start.x, y: start.y });
			for (let step = 1; step <= 5; ++step) {
				cy.cGet('body').realMouseMove(start.x + (overText.x - start.x) * step / 5,
					start.y + (overText.y - start.y) * step / 5);
			}
			helper.processToIdle(win);
			cy.cGet('.text-drop-caret').should('be.visible');
			for (let step = 1; step <= 5; ++step) {
				cy.cGet('body').realMouseMove(overText.x + (onShape.x - overText.x) * step / 5,
					overText.y + (onShape.y - overText.y) * step / 5);
			}
		});
		helper.processToIdle(win);

		// Then the drop caret goes away, as the text would land nowhere there:
		cy.cGet('.text-drop-caret').should('not.be.visible');

		// And a release there moves nothing:
		cy.then(function() {
			cy.cGet('body').realMouseUp({ x: onShape.x, y: onShape.y });
		});
		helper.processToIdle(win);
		helper.setDummyClipboardForCopy();
		helper.typeIntoDocument('{ctrl}{end}');
		helper.typeIntoDocument('{shift}{home}');
		helper.textSelectionShouldExist();
		helper.copy();
		helper.expectTextForClipboard('Test2');
	});
});

describe(['tagdesktop'], 'Writer drag and drop of a text selection in the compare view.', function() {

	beforeEach(function() {
		cy.viewport(1400, 600);
		// Three paragraphs: Test1, Test3, Test2.
		helper.setupAndLoadDocument('writer/drag_drop_text.fodt');
		desktopHelper.switchUIToNotebookbar();
		cy.cGet('#Review-tab-label').click();
		cy.getFrameWindow().then((win) => {
			this.win = win;
		});
	});

	it('A press inside the selection starts its drag where the compare view shows it', function() {
		var win = this.win;

		// Given the compare view, with Test2 selected:
		desktopHelper.getNbIconArrow('TrackChanges', 'Review').click();
		cy.cGet('#compare-tracked-change-button').filter(':visible').click();
		cy.cGet('#compare-tracked-change-entry-1').click();
		cy.cGet('.compare-changes-labels').should('not.have.css', 'display', 'none');
		helper.processToIdle(win);
		cy.getFrameWindow().should(function(frameWindow) {
			expect(frameWindow.app.activeDocument.activeLayout.type).to.equal('ViewLayoutCompareChanges');
		});
		helper.typeIntoDocument('{ctrl}{end}');
		helper.typeIntoDocument('{shift}{home}');
		helper.textSelectionShouldExist();
		helper.processToIdle(win);

		// When the button is pressed in the middle of the selection, where the view shows it:
		var press;
		cy.getFrameWindow().should(function(frameWindow) {
			var rectangles = frameWindow.app.activeDocument.activeView.selectionRectangles;
			expect(rectangles.length).to.be.above(0);
			var middle = layoutScreenPoint(frameWindow, rectangles[0][0] + rectangles[0][2] / 2,
				rectangles[0][1] + rectangles[0][3] / 2);
			press = middle;
		});
		cy.then(function() {
			cy.cGet('body').realMouseDown({ x: press.x, y: press.y });
		});

		// Then the press counts as one inside the selection, which a drag of it starts from:
		cy.getFrameWindow().should(function(frameWindow) {
			expect(frameWindow.app.activeDocument.mouseControl.pressedInSelection).to.be.true;
		});

		cy.then(function() {
			cy.cGet('body').realMouseUp({ x: press.x, y: press.y });
		});
	});
});
