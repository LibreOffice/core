/* global describe it cy beforeEach require expect Cypress */

var helper = require('../../common/helper');
var calcHelper = require('../../common/calc_helper');
var desktopHelper = require('../../common/desktop_helper');

function selectColumnB(win) {
	// Each click arms a click timer. A second click while it is pending counts as a double
	// click, and mouse-control drops moves while it is pending.
	calcHelper.clickOnFirstCell();
	helper.waitForTimers(win, 'clicktimer');
	calcHelper.clickOnACell(1, 1, 2, 1);
	calcHelper.assertAddressAfterIdle(win, 'B1');
	cy.realPress(['Control', 'Space']);
	calcHelper.assertAddressInput('B1:B{lastRow}');

	helper.waitForTimers(win, 'clicktimer');
	helper.processToIdle(win);
}

// Drag from the first row of column B to the first row of the column with the 0-based
// index targetColumn, in a few steps. pressOptions are the modifier keys held at the press,
// dragOptions the ones held on every move and at the release.
function dragColumnB(win, targetColumn, pressOptions, dragOptions) {
	const SOURCE_COLUMN = 1;
	const STEPS = 5;
	cy.then(function() {
		const anchor = win.app.sectionContainer.getDocumentAnchor();
		const dpiScale = win.app.dpiScale;
		const containerBounds = win.document.getElementById('canvas-container').getBoundingClientRect();
		const geometry = win.app.map._docLayer.sheetGeometry;
		const cellCenter = function(column) {
			const rectangle = geometry.getCellRect(column, 0);
			return {
				x: containerBounds.left + (anchor[0] + (rectangle.min.x + rectangle.max.x) / 2) / dpiScale,
				y: containerBounds.top + (anchor[1] + (rectangle.min.y + rectangle.max.y) / 2) / dpiScale,
			};
		};
		const start = cellCenter(SOURCE_COLUMN);
		const end = cellCenter(targetColumn);

		cy.cGet('body').realMouseDown(Object.assign({ x: start.x, y: start.y }, pressOptions));
		for (let step = 1; step <= STEPS; ++step) {
			const x = start.x + (end.x - start.x) * step / STEPS;
			cy.cGet('body').realMouseMove(x, start.y, dragOptions);
		}
		cy.cGet('body').realMouseUp(Object.assign({ x: end.x, y: end.y }, dragOptions));
	});
	helper.processToIdle(win);
}

describe(['tagdesktop'], 'Calc drag and drop of a selected column.', function() {

	beforeEach(function() {
		// Row 1 holds A to F, one letter per column, the letter naming its column.
		helper.setupAndLoadDocument('calc/cell_drag_drop.fods');
		cy.getFrameWindow().then((win) => {
			this.win = win;
		});
	});

	function assertFirstRow(range, expectedData) {
		helper.setDummyClipboardForCopy();
		calcHelper.selectCellsInRange(range);
		helper.copy();
		calcHelper.assertDataClipboardTable(expectedData);
	}

	it('Alt+drag of a selected column moves it in front of the drop column', function() {
		selectColumnB(this.win);
		// Three columns in five steps, so the first step already leaves column B.
		dragColumnB(this.win, 4, { altKey: true }, { altKey: true });

		assertFirstRow('A1:F1', ['A', 'C', 'D', 'B', 'E', 'F']);
	});

	it('Ctrl pressed right after the button copies the dragged column', function() {
		selectColumnB(this.win);
		// A Ctrl press on a selected cell deselects it, so Ctrl goes down only once the button
		// is held.
		dragColumnB(this.win, 7, {}, { ctrlKey: true });

		assertFirstRow('A1:H1', ['A', 'B', 'C', 'D', 'E', 'F', '', 'B']);
	});
});

describe(['tagdesktop'], 'Calc Alt-drag and the accelerator info boxes.', function() {

	beforeEach(function() {
		helper.setupAndLoadDocument('calc/cell_drag_drop.fods');
		desktopHelper.switchUIToNotebookbar();
		cy.getFrameWindow().then((win) => {
			this.win = win;
		});
		// NotebookbarAccessibility initializes on a timer after the notebookbar is shown, and
		// ignores Alt until then.
		cy.getFrameWindow().should(function(win) {
			expect(win.app.UI.notebookbarAccessibility.initialized).to.be.true;
		});
	});

	// A real key event, so the page sees Alt go down and up the way it does when the user holds
	// the key.
	function dispatchAltKey(type) {
		return Cypress.automation('remote:debugger:protocol', {
			command: 'Input.dispatchKeyEvent',
			params: {
				type: type,
				key: 'Alt',
				code: 'AltLeft',
				windowsVirtualKeyCode: 18,
				modifiers: type === 'keyUp' ? 0 : 1,
			},
		});
	}

	it('Releasing Alt after an Alt-drag does not show the accelerator info boxes', function() {
		selectColumnB(this.win);

		cy.then(function() {
			return dispatchAltKey('rawKeyDown');
		});
		cy.getFrameWindow().should(function(win) {
			expect(win.app.UI.notebookbarAccessibility.mayShowAcceleratorInfoBoxes).to.be.true;
		});

		dragColumnB(this.win, 4, { altKey: true }, { altKey: true });
		cy.then(function() {
			return dispatchAltKey('keyUp');
		});

		cy.cGet('body').should('not.have.class', 'activate-info-boxes');
		cy.getFrameWindow().should(function(win) {
			expect(win.document.activeElement.id).to.not.equal('accessibilityInputElement');
		});
	});
});
