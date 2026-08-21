/* global describe it cy beforeEach require */

var helper = require('../../common/helper');
var calcHelper = require('../../common/calc_helper');

describe(['tagdesktop', 'tagnextcloud', 'tagproxy'], 'Middle click moves the cell cursor.', function() {

	beforeEach(function() {
		helper.setupAndLoadDocument('calc/empty-selections.ods');
		cy.getFrameWindow().then((win) => {
			this.win = win;
		});
	});

	it('Middle click away from the cell cursor moves it there first.', function() {
		// cool#2055: the browser pastes the primary selection as the middle
		// button's own default action, but until now nothing moved the cell
		// cursor there first, so the paste landed on whichever cell was
		// already selected instead of the one under the pointer.
		calcHelper.clickOnFirstCell(true, false, 'A1');

		cy.cGet('#test-div-OwnCellCursor').then((items) => {
			const clientRect = items[0].getBoundingClientRect();
			const currentX = clientRect.left + clientRect.width * 0.5;
			const currentY = clientRect.top + clientRect.height * 0.5;
			// Three columns and three rows away from A1, i.e. D4.
			const clickX = currentX + clientRect.width * 3;
			const clickY = currentY + clientRect.height * 3;
			cy.cGet('body').realClick({ x: clickX, y: clickY, button: 'middle' });
		});

		calcHelper.assertAddressAfterIdle(this.win, 'D4');
	});
});
