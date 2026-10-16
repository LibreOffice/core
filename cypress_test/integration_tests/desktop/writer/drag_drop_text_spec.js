/* global describe it cy beforeEach require expect */

var helper = require('../../common/helper');

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

		helper.assertCursorAndFocus();
		// A modifier stays held for the rest of the typed string, so each key goes alone.
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
			var middle = screenPoint(win, (startOfTest2.x1 + endOfTest2.x1) / 2,
				(startOfTest2.y1 + startOfTest2.y2) / 2);
			cy.cGet('body').dblclick(middle.x, middle.y);
		});
		helper.textSelectionShouldExist();
		helper.processToIdle(win);
		helper.waitForTimers(win, 'clicktimer');
		cy.cGet('#context-toolbar').should('not.have.class', 'hidden');

		// Five steps from the middle of Test2, so the first steps are too short to start the
		// drag and the later ones start it.
		const STEPS = 5;
		cy.then(function() {
			var start = screenPoint(win, (startOfTest2.x1 + endOfTest2.x1) / 2,
				(startOfTest2.y1 + startOfTest2.y2) / 2);
			var end = screenPoint(win, dropTarget.x1, (dropTarget.y1 + dropTarget.y2) / 2);

			cy.cGet('body').realMouseDown({ x: start.x, y: start.y });
			for (let step = 1; step <= STEPS; ++step) {
				cy.cGet('body').realMouseMove(start.x + (end.x - start.x) * step / STEPS,
					start.y + (end.y - start.y) * step / STEPS);
			}
			helper.processToIdle(win);

			// While the button is held, the drop caret marks where the text will land, and the
			// toolbar of the dragged selection is gone.
			cy.cGet('.text-drop-caret').should('be.visible');
			cy.cGet('#context-toolbar').should('have.class', 'hidden');

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
});
