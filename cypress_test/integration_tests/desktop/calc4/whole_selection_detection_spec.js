/* global describe it cy beforeEach require expect */

var helper = require('../../common/helper');
var calcHelper = require('../../common/calc_helper');

function assertWholeSelectionFlags(win, columnSelected, rowSelected) {
	helper.processToIdle(win);
	cy.wrap(win).its('app.map.wholeColumnSelected').should('eq', columnSelected);
	cy.wrap(win).its('app.map.wholeRowSelected').should('eq', rowSelected);
}

// Selects an exact range through the Name Box (data independent).
function selectViaNameBox(win, range) {
	cy.cGet(helper.addressInputSelector).type('{selectAll}' + range + '{enter}');
	cy.cGet(helper.addressInputSelector).should('have.prop', 'value', range);
	helper.processToIdle(win);
}

function sendUno(win, command) {
	cy.then(function() {
		win.app.map.sendUnoCommand(command);
	});
	helper.processToIdle(win);
}

// Covers the whole-column / whole-row detection in CalcTileLayer._onRowColSelCount
// and CanvasTileLayer._isWholeColumnSelected / _isWholeRowSelected, which now
// compare the reported selection against app.calc.maxRowCount / maxColumnCount
// instead of the previous hardcoded 1048576 / 'XFD' literals.
describe(['tagdesktop', 'tagnextcloud', 'tagproxy'], 'Whole column/row selection detection', function() {
	beforeEach(function() {
		helper.setupAndLoadDocument('calc/top_toolbar.ods');
		cy.viewport(1920, 1080);
		cy.getFrameWindow().then((win) => {
			this.win = win;
		});
	});

	it('Selecting a whole column sets only wholeColumnSelected', function() {
		// Whole column: rowCount === app.calc.maxRowCount (address A1:A<maxRowCount>).
		calcHelper.selectFirstColumn();
		assertWholeSelectionFlags(this.win, true, false);
	});

	it('Selecting a whole row sets only wholeRowSelected', function() {
		// Whole row: columnCount === app.calc.maxColumnCount (address A1:<lastColumn>1).
		calcHelper.selectFirstRow();
		assertWholeSelectionFlags(this.win, false, true);
	});

	it('A partial selection clears both flags', function() {
		// First take a whole column so both flags are defined and one is true.
		calcHelper.selectFirstColumn();
		assertWholeSelectionFlags(this.win, true, false);

		// A single-cell selection is neither a whole column nor a whole row.
		helper.typeIntoDocument('{ctrl}{home}');
		cy.cGet(helper.addressInputSelector).should('have.prop', 'value', 'A1');
		assertWholeSelectionFlags(this.win, false, false);
	});
});

// The whole-row / whole-column detection compares against the sheet's absolute
// limits (maxColumnCount / maxRowCount), NOT the visible extent. These tests
// hide the trailing cells so maxVisible{Column,Row}Index falls below the sheet
// limit, then verify the detection still fires - a whole row / column selection
// spans every cell, including the hidden ones.
describe(['tagdesktop'], 'Whole selection detection when maxVisible index is below the sheet limit', function() {
	beforeEach(function() {
		helper.setupAndLoadDocument('calc/empty-selections.ods');
		cy.getFrameWindow().then((win) => {
			this.win = win;
		});
	});

	it('detects a whole row even when trailing columns are hidden (maxVisibleColumnIndex < maxColumnCount)', function() {
		var win = this.win;
		var sheetGeometry = win.app.map._docLayer.sheetGeometry;
		// Hide columns D..last, leaving only A-C visible.
		selectViaNameBox(win, 'D1:' + calcHelper.columnNumberToLabel(sheetGeometry.maxVisibleColumnIndex + 1) + '1');
		sendUno(win, '.uno:HideColumn');
		// The geometry update from HideColumn arrives after processToIdle
		// returns, so retry the read against the live object until it lands.
		cy.wrap(sheetGeometry).should(function(sg) {
			expect(sg.maxVisibleColumnIndex, 'trailing columns hidden')
				.to.be.lessThan(win.app.calc.maxColumnCount - 1);
		});
		// A whole-row selection still spans every column (including the hidden ones).
		calcHelper.selectFirstRow();
		assertWholeSelectionFlags(win, false, true);
	});

	it('detects a whole column even when trailing rows are hidden (maxVisibleRowIndex < maxRowCount)', function() {
		var win = this.win;
		var sheetGeometry = win.app.map._docLayer.sheetGeometry;
		// Hide rows 4..last, leaving only rows 1-3 visible.
		selectViaNameBox(win, 'A4:A' + (sheetGeometry.maxVisibleRowIndex + 1));
		sendUno(win, '.uno:HideRow');
		// The geometry update from HideRow arrives after processToIdle
		// returns, so retry the read against the live object until it lands.
		cy.wrap(sheetGeometry).should(function(sg) {
			expect(sg.maxVisibleRowIndex, 'trailing rows hidden')
				.to.be.lessThan(win.app.calc.maxRowCount - 1);
		});
		// A whole-column selection still spans every row (including the hidden ones).
		calcHelper.selectFirstColumn();
		assertWholeSelectionFlags(win, true, false);
	});
});
