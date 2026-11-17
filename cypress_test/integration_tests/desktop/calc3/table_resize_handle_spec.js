/* global describe it cy beforeEach require expect */

var helper = require('../../common/helper');
var calcHelper = require('../../common/calc_helper');

describe(['tagdesktop'], 'Calc styled table resize handle.', function() {

	beforeEach(function() {
		helper.setupAndLoadDocument('calc/table_resize_handles.xlsx');
		cy.getFrameWindow().then((win) => {
			this.win = win;
		});
	});

	// The corner resize handle is shown for every visible styled table, not only
	// the one under the cursor. With the cursor at A1 (in neither table) both
	// tables must still show a handle.
	it('shows the resize handle for every visible table, not just the one under the cursor', function() {
		calcHelper.clickOnFirstCell();

		// Two visible tables -> exactly two handle sections, indexes 0 and 1
		// (one per table), and no third.
		cy.cGet('[id="test-div-TableRange handle 0"]').should('exist');
		cy.cGet('[id="test-div-TableRange handle 1"]').should('exist');
		cy.cGet('[id="test-div-TableRange handle 2"]').should('not.exist');
	});

	// Both tables have empty rows below, so the grow can't be refused. Drag with a
	// real mouse drag like the autofill spec, so the container drives the drag.
	it('resizing a table by dragging its handle down grows the table', function() {
		var win = this.win;

		calcHelper.clickOnFirstCell();
		cy.cGet('[id="test-div-TableRange handle 0"]').should('exist');

		// Mouse-control drops moves while the click timer is pending.
		helper.waitForTimers(win, 'clicktimer');
		helper.processToIdle(win);

		var startY;
		cy.cGet('[id="test-div-TableRange handle 0"]').then(function($handle) {
			var handle = win.app.sectionContainer.getSectionWithName('TableRange handle 0');
			expect(handle, 'table resize handle section').to.exist;
			startY = handle.position[1];

			var rectangle = $handle[0].getBoundingClientRect();
			var dragX = rectangle.left + rectangle.width / 2;
			var dragStartY = rectangle.top + rectangle.height / 2;
			var cellHeight = win.app.calc.cellCursorRectangle.pHeight / win.app.dpiScale;
			var dragEndY = dragStartY + Math.round(cellHeight * 3);

			cy.cGet('body').realMouseDown({ x: dragX, y: dragStartY });
			cy.cGet('body').realMouseMove(dragX, dragEndY);
			cy.cGet('body').realMouseUp({ x: dragX, y: dragEndY });
		});

		helper.processToIdle(win);

		cy.getFrameWindow().then(function(win) {
			var handle = win.app.sectionContainer.getSectionWithName('TableRange handle 0');
			expect(handle, 'table resize handle section').to.exist;
			expect(handle.position[1], 'handle moved down after grow').to.be.greaterThan(startY);
		});
	});

	// The client draws the tables from the sheet cell ranges it fetches when the engine says the
	// sheet changed, and ignores the handles the engine sends on a cursor move. So a handle for a
	// new table is the invalidation and the fetch at work.
	it('shows a handle for a newly inserted table', function() {
		calcHelper.clickOnFirstCell();
		cy.cGet('[id="test-div-TableRange handle 1"]').should('exist');
		cy.cGet('[id="test-div-TableRange handle 2"]').should('not.exist');

		calcHelper.selectCellsInRange('I2:K4');
		cy.getFrameWindow().then(function(win) {
			win.app.socket.sendMessage('uno .uno:InsertCalcTable');
		});
		cy.cGet('#tablerangedialog').should('be.visible');
		cy.cGet('#tablerangedialog #ok').click();
		cy.cGet('#tablerangedialog').should('not.exist');

		cy.cGet('[id="test-div-TableRange handle 2"]').should('exist');
	});
});

describe(['tagdesktop'], 'Calc sparkline group marker.', function() {

	beforeEach(function() {
		helper.setupAndLoadDocument('calc/sparkline_group.fods');
	});

	// One sparkline group in E1:E2. It is filled only while the cell cursor is on one of its
	// cells, which the section works out from the sheet cell ranges and the cursor on every draw.
	it('marks the sparkline group under the cell cursor only', function() {
		function getMarkerSection(win) {
			return win.app.sectionContainer.getSectionWithName(win.app.CSections.CellRangeMarker.name);
		}

		cy.getFrameWindow().should(function(win) {
			var groups = getMarkerSection(win).sectionProperties.sheetCellRanges.get(0).sparklineGroups;
			expect(groups, 'sparkline groups').to.have.length(1);
			expect(groups[0][0].min.x, 'start column').to.equal(4);
			expect(groups[0][0].max.y, 'end row').to.equal(1);
		});

		calcHelper.selectCellsInRange('E2');
		cy.getFrameWindow().should(function(win) {
			expect(getMarkerSection(win).getSparklineGroupAtCursor(0), 'group at E2').to.exist;
		});

		calcHelper.selectCellsInRange('A4');
		cy.getFrameWindow().should(function(win) {
			expect(getMarkerSection(win).getSparklineGroupAtCursor(0), 'group at A4').to.not.exist;
		});
	});
});

describe(['tagdesktop'], 'Calc styled table resize handle below the fold.', function() {

	beforeEach(function() {
		helper.setupAndLoadDocument('calc/table_resize_handles_below_fold.xlsx');
	});

	// 12 styled tables down to row 55, most of them outside the initial viewport. Nothing is
	// clicked here: a handle that only arrives with the cursor change is the bug this guards.
	// An off screen section has no test div, so ask the section container rather than the DOM.
	it('shows a handle for every table of the sheet, not only the ones on screen', function() {
		cy.getFrameWindow().should(function(win) {
			expect(win.app.sectionContainer.getSectionWithName('TableRange handle 11'),
				'handle of the 12th table').to.exist;
		});

		cy.getFrameWindow().then(function(win) {
			expect(win.app.sectionContainer.getSectionWithName('TableRange handle 12'),
				'no handle past the 12 tables').to.not.exist;
		});
	});
});
