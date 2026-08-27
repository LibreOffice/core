/* global describe it cy require beforeEach expect */

// The five cases from https://github.com/CollaboraOnline/online/issues/16190,
// one test each. They all navigate onto a cell that is bigger than the pane
// past the freeze, so no scroll position can show the whole of it. What the
// view should then do depends on where the cell already sits: the start edge
// of the cell is the one worth showing, but only when moving the view towards
// it actually gains something.

const helper = require('../../common/helper');

// The cell cursor and the pane past the freeze, both in twips, as the scroll
// code itself sees them.
function cursorAndFreePane(win) {
	const panes = win.app.getViewRectangles();
	return {
		cursor: win.app.calc.cellCursorRectangle,
		pane: panes[panes.length - 1],
		scrollX: win.app.activeDocument.activeLayout.viewedRectangle.pX1,
		scrollY: win.app.activeDocument.activeLayout.viewedRectangle.pY1,
	};
}

// Wait for the cell cursor to arrive on the given column and row, and for the
// scroll that the arrival may trigger to have been applied, so that a test
// asserting the view did not move cannot pass by running too early.
function afterCursorReaches(column, row) {
	cy.getFrameWindow().should(function(win) {
		expect(win.app.calc.cellAddress.x, 'cursor column').to.equal(column);
		expect(win.app.calc.cellAddress.y, 'cursor row').to.equal(row);
	});
	cy.getFrameWindow().then(function(win) { return helper.processToIdle(win); });
}

describe(['tagdesktop'], 'Calc navigation onto a cell wider than the free pane', function() {

	beforeEach(function() {
		// Column B of this document is 100cm wide, so it cannot fit in the
		// pane to the right of a freeze at any viewport this suite uses.
		helper.setupAndLoadDocument('calc/wide-cell.fods');
		cy.getFrameWindow().then(function(win) {
			win.app.map.sendUnoCommand('.uno:FreezePanesColumn'); // Freeze first column.
			return helper.processToIdle(win);
		});
		cy.getFrameWindow().should(function(win) {
			const state = cursorAndFreePane(win);
			expect(win.app.calc.splitCoordinate.x, 'column A frozen').to.be.greaterThan(0);
			expect(state.pane.width, 'free pane narrower than column B').to.be.lessThan(56692);
		});
	});

	it('Case 1: stepping onto the wide cell from the frozen column', function() {
		let scrollBefore;
		cy.getFrameWindow().then(function(win) {
			scrollBefore = cursorAndFreePane(win).scrollX;
		});

		// A1 -> B1. B1 starts right at the freeze, so its start is already on
		// screen and the view has no reason to move. Scrolling to park the far
		// end of B1 against the right edge instead would push the beginning of
		// the cell the user just selected off screen.
		helper.typeIntoDocument('{rightarrow}');
		afterCursorReaches(1, 0);

		cy.getFrameWindow().should(function(win) {
			const state = cursorAndFreePane(win);
			expect(state.scrollX, 'view did not move').to.equal(scrollBefore);
			expect(state.cursor.x1, 'start of B1 on screen')
				.to.be.within(state.pane.x1, state.pane.x2);
		});
	});

	it('Case 2: coming back to the wide cell from the right', function() {
		// A1 -> B1 -> C1 scrolls the view right, past the whole of column B.
		helper.typeIntoDocument('{rightarrow}{rightarrow}');
		afterCursorReaches(2, 0);
		cy.getFrameWindow().should(function(win) {
			expect(cursorAndFreePane(win).scrollX, 'reaching C1 scrolled right')
				.to.be.greaterThan(0);
		});

		// Back onto B1. Its far end is what is on screen now, so the view has
		// to come back to the start of the cell.
		helper.typeIntoDocument('{leftarrow}');
		afterCursorReaches(1, 0);

		cy.getFrameWindow().should(function(win) {
			const state = cursorAndFreePane(win);
			expect(state.cursor.x1, 'start of B1 on screen')
				.to.be.within(state.pane.x1, state.pane.x2);
		});
	});
});

describe(['tagdesktop'], 'Calc navigation onto a cell taller than the free pane', function() {

	beforeEach(function() {
		// Row 3 of this document is 100cm tall, so it cannot fit in the pane
		// below a freeze at any viewport this suite uses.
		helper.setupAndLoadDocument('calc/tall-cell.fods');
		cy.getFrameWindow().then(function(win) {
			win.app.map.sendUnoCommand('.uno:FreezePanesRow'); // Freeze first row.
			return helper.processToIdle(win);
		});
		cy.getFrameWindow().should(function(win) {
			const state = cursorAndFreePane(win);
			expect(win.app.calc.splitCoordinate.y, 'row 1 frozen').to.be.greaterThan(0);
			expect(state.pane.height, 'free pane shorter than row 3').to.be.lessThan(56692);
		});
	});

	it('Case 3: stepping onto the tall cell from the frozen row', function() {
		let scrollBefore;
		cy.getFrameWindow().then(function(win) {
			scrollBefore = cursorAndFreePane(win).scrollY;
		});

		// A1 -> A2 -> A3. The top of row 3 is already on screen below row 2,
		// so the view stays where it is rather than scrolling down to bring
		// the bottom of the cell into view and slicing off its top.
		helper.typeIntoDocument('{downarrow}{downarrow}');
		afterCursorReaches(0, 2);

		cy.getFrameWindow().should(function(win) {
			const state = cursorAndFreePane(win);
			expect(state.scrollY, 'view did not move').to.equal(scrollBefore);
			expect(state.cursor.y1, 'top of A3 on screen')
				.to.be.within(state.pane.y1, state.pane.y2);
		});
	});

	it('Case 4: coming back to the tall cell from below', function() {
		// Down to A4, which is past the bottom of the tall row and so scrolls
		// the view down.
		helper.typeIntoDocument('{downarrow}{downarrow}{downarrow}');
		afterCursorReaches(0, 3);
		cy.getFrameWindow().should(function(win) {
			expect(cursorAndFreePane(win).scrollY, 'reaching A4 scrolled down')
				.to.be.greaterThan(0);
		});

		// Back up onto A3. Only its bottom is on screen, so the view has to
		// come back up to the top of the cell.
		helper.typeIntoDocument('{uparrow}');
		afterCursorReaches(0, 2);

		cy.getFrameWindow().should(function(win) {
			const state = cursorAndFreePane(win);
			expect(state.cursor.y1, 'top of A3 on screen')
				.to.be.within(state.pane.y1, state.pane.y2);
		});
	});

	it('Case 5: moving to the next column in the same tall row', function() {
		helper.typeIntoDocument('{downarrow}{downarrow}');
		afterCursorReaches(0, 2);

		// Scroll into the middle of row 3, where neither its top nor its
		// bottom edge is on screen. This is the reading position the user
		// picked, and stepping sideways within the same row must not throw it
		// away: B3 begins and ends at exactly the same height as A3, so there
		// is nothing new for a scroll to reveal.
		cy.getFrameWindow().then(function(win) {
			win.app.activeDocument.activeLayout.scroll(0, 1000);
			return helper.processToIdle(win);
		});

		let scrolledY;
		cy.getFrameWindow().should(function(win) {
			const state = cursorAndFreePane(win);
			expect(state.cursor.y1, 'top of row 3 off screen above')
				.to.be.lessThan(state.pane.y1);
			expect(state.cursor.y2, 'bottom of row 3 off screen below')
				.to.be.greaterThan(state.pane.y2);
			scrolledY = state.scrollY;
		});

		helper.typeIntoDocument('{rightarrow}');
		afterCursorReaches(1, 2);

		cy.getFrameWindow().should(function(win) {
			expect(cursorAndFreePane(win).scrollY, 'view did not move')
				.to.equal(scrolledY);
		});
	});
});
