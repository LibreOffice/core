/* global describe it cy beforeEach require */

var helper = require('../../common/helper');
var calcHelper = require('../../common/calc_helper');

describe(['tagdesktop'], 'Calc styled table row delete.', function() {

	beforeEach(function() {
		helper.setupAndLoadDocument('calc/table_resize_handles.xlsx');
		cy.getFrameWindow().then((win) => {
			this.win = win;
		});
	});

	// Table2 covers A2:C6: header row 2, data rows 3-5 and a Total Row in row 6.
	// Taking the header row while leaving the rest of the table behind would
	// promote the row below to header, so the command is refused instead.
	it('refuses a row delete that would take the table header row', function() {
		helper.processToIdle(this.win);
		calcHelper.selectCellsInRange('A2:C4');

		helper.waitForMapState('.uno:DeleteRows', 'disabled');
		helper.waitForMapState('.uno:DeleteCell', 'disabled');
	});

	it('allows deleting only the data rows', function() {
		helper.processToIdle(this.win);
		calcHelper.selectCellsInRange('A3:C5');

		helper.waitForMapState('.uno:DeleteRows', 'enabled');
		helper.waitForMapState('.uno:DeleteCell', 'enabled');
	});

	// Taking the whole table is allowed, that just removes the table.
	it('allows deleting the whole table', function() {
		helper.processToIdle(this.win);
		calcHelper.selectCellsInRange('A2:C6');

		helper.waitForMapState('.uno:DeleteRows', 'enabled');
		helper.waitForMapState('.uno:DeleteCell', 'enabled');
	});
});
