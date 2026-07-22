/* global describe it cy beforeEach require */

var helper = require('../../common/helper');
var calcHelper = require('../../common/calc_helper');

describe(['tagdesktop'], 'Calc autofill marker drag.', function() {

	beforeEach(function() {
		helper.setupAndLoadDocument('calc/autofill.fods');
		cy.getFrameWindow().then((win) => {
			this.win = win;
		});
	});

	// Drag the autofill marker two cells down with a real mouse drag, the same
	// gesture a user makes on the fill handle. The canvas container drives the
	// drag, so the whole fill path runs end to end. modifierOptions carries the
	// keyboard modifier (for example ctrlKey) that is held during the drag.
	function dragMarkerTwoCellsDown(win, modifierOptions) {
		calcHelper.clickOnFirstCell();
		cy.cGet('[id="test-div-auto fill marker"]').should('exist');

		// The click that selected the cell arms a 250ms click timer, and
		// mouse-control drops moves while it is pending. Let it finish so the
		// drag below is not swallowed.
		helper.waitForTimers(win, 'clicktimer');
		helper.processToIdle(win);

		cy.cGet('[id="test-div-auto fill marker"]').then(function($handle) {
			var rectangle = $handle[0].getBoundingClientRect();
			var startX = rectangle.left + rectangle.width / 2;
			var startY = rectangle.top + rectangle.height / 2;
			var cellHeight = win.app.calc.cellCursorRectangle.pHeight / win.app.dpiScale;
			var endY = startY + Math.round(cellHeight * 2);

			cy.cGet('body').realMouseDown(Object.assign({ x: startX, y: startY }, modifierOptions));
			cy.cGet('body').realMouseMove(startX, endY, modifierOptions);
			cy.cGet('body').realMouseUp(Object.assign({ x: startX, y: endY }, modifierOptions));
		});

		helper.processToIdle(win);

		// After the drag core opens the autofill options dropdown
		// (Copy Cells / Fill Series), dismiss it.
		cy.cGet('#jsd-context-menu-dropdown-overlay').click();
		cy.cGet('#jsd-context-menu-dropdown-overlay').should('not.exist');
	}

	// Assert the document contents via clipboard copy rather than via
	// the formulabar: in the copy case A2/A3 hold the same text as A1,
	// so core never re-sends the formulabar setText and the assertion
	// would depend on the load-time message that can be lost in a slow
	// session (i.e. in CI). Select the range via the address input
	// instead of selectEntireSheet, whose row header clicks are not
	// reliable right after the autofill drag.
	function assertCellContents(expectedData) {
		helper.setDummyClipboardForCopy();
		calcHelper.selectCellsInRange('A1:A3');
		helper.copy();
		calcHelper.assertDataClipboardTable(expectedData);
	}

	it('Drag autofill marker fills cells with incremented numbers', function() {
		dragMarkerTwoCellsDown(this.win, {});

		assertCellContents(['1', '2', '3']);
	});

	it('Ctrl+drag autofill marker copies the source cell', function() {
		dragMarkerTwoCellsDown(this.win, { ctrlKey: true });

		assertCellContents(['1', '1', '1']);
	});
});
