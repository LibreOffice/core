/* global describe it cy beforeEach require expect */

var helper = require('../../common/helper');
var calcHelper = require('../../common/calc_helper');

describe(['tagdesktop'], 'Pasting a copied shape in Calc', function() {

	beforeEach(function() {
		helper.setupAndLoadDocument('calc/empty-selections.ods');
	});

	// The document coordinates of the shape that is selected right now, in twips. They do not
	// move with the scroll, unlike anything read off the handle overlay.
	function selectedShapeRectangle(win) {
		const rectangle = win.app.definitions.graphicSelection.rectangle;
		expect(rectangle, 'a shape is selected').to.not.be.null;
		return {
			top: rectangle.y1,
			bottom: rectangle.y2,
		};
	}

	it('A copied shape lands at the cell the paste was issued from', function() {
		const source = {};
		const target = {};
		const pasted = {};

		cy.getFrameWindow().then(function(win) {
			// Escape first, so no leftover cell edit or address input state swallows the
			// selection that comes back with the new shape.
			helper.typeIntoDocument('{esc}');
			helper.processToIdle(win);

			// Inserting a basic shape also selects it.
			cy.then(function() {
				win.app.map.sendUnoCommand('.uno:BasicShapes.rectangle');
			});
			helper.processToIdle(win);
			cy.cGet('#test-div-shapeHandlesSection').should('exist');

			cy.then(function() {
				Object.assign(source, selectedShapeRectangle(win));
				// The message the browser sends for Ctrl+C, filling the kit's own clipboard.
				win.app.socket.sendMessage('uno .uno:Copy');
			});
			helper.processToIdle(win);

			// Leave the shape behind and move two thousand rows down. Row 2000 is far enough
			// both to tell a paste that ignores the cell cursor from one that honours it, and
			// to show up a paste position that drifts by a fraction of a row for every row
			// above it.
			helper.typeIntoDocument('{esc}');
			cy.cGet('#test-div-shapeHandlesSection').should('not.exist');
			calcHelper.enterCellAddressAndConfirm(win, 'A2000');

			cy.then(function() {
				const cellCursor = win.app.calc.cellCursorRectangle;
				target.top = cellCursor.y1;
				target.rowHeight = cellCursor.height;

				// The original has to be off screen, or the paste has nowhere wrong to go.
				const viewed = win.app.activeDocument.activeLayout.viewedRectangle;
				expect(viewed.y1, 'the view scrolled past the copied shape')
					.to.be.greaterThan(source.bottom);

				// The message the browser sends for Ctrl+V once it recognises the clipboard
				// content as coming from this same document.
				win.app.socket.sendMessage('uno .uno:Paste');
			});

			// The pasted shape comes back selected. Waiting on the selection rather than on the
			// handle overlay keeps the test honest about placement: the overlay only exists
			// while the shape is on screen, so waiting for it would turn a misplaced paste into
			// a timeout instead of a failed measurement.
			cy.wrap(win, { log: false }).should(function(frameWindow) {
				expect(frameWindow.app.definitions.graphicSelection.rectangle,
					'the pasted shape is selected').to.not.be.null;
			});
			helper.processToIdle(win);

			cy.then(function() {
				Object.assign(pasted, selectedShapeRectangle(win));

				expect(pasted.top, 'the copy is clear of the original')
					.to.be.greaterThan(source.bottom);
				// A pasted object is centred on the paste point, and the paste point is the
				// corner of the cursor cell moved down by half the object, so its top edge and
				// the top of that cell meet. Allow a row for the rounding on the way here.
				expect(pasted.top, 'the copy starts on the row the paste came from')
					.to.be.closeTo(target.top, target.rowHeight);
			});
		});
	});
});
