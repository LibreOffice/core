/* global cy expect Cypress require expect */

var helper = require('./helper');

// Click on the formula bar.
// mouseover is triggered to avoid leaving the mouse on the Formula-Bar,
// which shows the tooltip and messes up tests.
function clickFormulaBar() {
	cy.log('>> clickFormulaBar - start');

	cy.cGet('#sc_input_window.formulabar').focus();
	cy.cGet('body').trigger('mouseover');

	cy.log('<< clickFormulaBar - end');
}

// Click on the document at the specified offset from the origin of cell A1.
function clickAtOffset(offsetX, offsetY, right=false) {
	cy.log('>> clickAtOffset - start');
	cy.log('Param - offsetX: ' + offsetX);
	cy.log('Param - offsetY: ' + offsetY);

	cy.cGet('#map').should('exist');
	cy.cGet('#map').should('be.visible');
	cy.cGet('#map')
		.then(function(items) {
			expect(items).to.have.lengthOf(1);
			const XPos = items[0].getBoundingClientRect().left + 2 + offsetX;
			const YPos = items[0].getBoundingClientRect().top + 2 + offsetY;
			if (right) {
				cy.cGet('body').rightclick(XPos, YPos);
			} else {
				cy.cGet('body').click(XPos, YPos);
			}
		});

	cy.log('<< clickAtOffset - end');
}

// Click on the first cell of the sheet (A1), we use the document
// top left corner to achieve that, so it works if the view is at the
// start of the sheet.
// Parameters:
// firstClick - this is the first click on the cell. It matters on mobile only,
//              because on mobile, the first click/tap selects the cell, the second
//              one makes the document to step in cell editing.
// dblClick - to do a double click or not. The result of double click is that the cell
//            editing is triggered both on desktop and mobile.
// expectedCell - the expected cell address after clicking (default 'A1').
function clickOnFirstCell(firstClick = true, dblClick = false, expectedCell = 'A1') {
	cy.log('>> clickOnFirstCell - start');
	cy.log('Param - firstClick: ' + firstClick);
	cy.log('Param - dblClick: ' + dblClick);
	cy.log('Param - expectedCell: ' + expectedCell);

	// Flush any pending layouting tasks before clicking
	cy.getFrameWindow().then(function(win) {
		helper.processToIdle(win);
	});

	// Compute the screen position of the centre of the first cell
	cy.getFrameWindow().then(function(win) {
		var anchor = win.app.sectionContainer.getDocumentAnchor();
		var dpiScale = win.app.dpiScale;
		var cellRect = win.app.map._docLayer.sheetGeometry.getCellRect(0, 0);
		var cellWidth = cellRect.max.x - cellRect.min.x;
		var cellHeight = cellRect.max.y - cellRect.min.y;
		var container = win.document.getElementById('canvas-container');
		var bcr = container.getBoundingClientRect();
		var XPos = bcr.left + (anchor[0] + cellWidth / 2) / dpiScale;
		var YPos = bcr.top + (anchor[1] + cellHeight / 2) / dpiScale;
		if (dblClick)
			cy.cGet('body').dblclick(XPos, YPos);
		else
			cy.cGet('body').click(XPos, YPos);
	});

	if (firstClick && !dblClick) {
		cy.cGet('#test-div-OwnCellCursor').should('exist');
	} else {
		cy.cGet('.cursor-overlay .blinking-cursor').should('be.visible');
	}

	cy.getFrameWindow().then(function(win) {
		assertAddressAfterIdle(win, expectedCell);
	});

	cy.log('<< clickOnFirstCell - end');
}

/*
	This function assumes:
	* The cell to be clicked is visible.
	* Row and column width / height values are the same.
	* Indexes are 1-based.
*/
function clickOnACell(currentColumnIndex, currentRowIndex, clickColumnIndex, clickRowIndex) {
	cy.log('>> clickOnACell - start');
	cy.log('Param - clickColumnIndex: ' + clickColumnIndex);
	cy.log('Param - clickRowIndex: ' + clickRowIndex);

	// Use the tile's edge to find the first cell's position
	cy.cGet('#test-div-OwnCellCursor').should('exist');
	cy.cGet('#test-div-OwnCellCursor')
		.then(function(items) {
			expect(items).to.have.lengthOf(1);
			const clientRect = items[0].getBoundingClientRect();
			const currentX = clientRect.left + parseInt(clientRect.width * 0.5);
			const currentY = clientRect.top + parseInt(clientRect.height * 0.5);
			const clickX = currentX + clientRect.width * (clickColumnIndex - currentColumnIndex);
			const clickY = currentY + clientRect.height * (clickRowIndex - currentRowIndex);
			cy.cGet('body').click(clickX, clickY);
		});

	cy.log('>> clickOnACell - end');
}

// Double click on the A1 cell.
function dblClickOnFirstCell() {
	cy.log('>> dblClickOnFirstCell - start');

	helper.typeIntoInputField(helper.addressInputSelector, 'A1');
	clickOnFirstCell(false, true);

	cy.log('<< dblClickOnFirstCell - end');
}

// Type some text into the formula bar.
// Parameters:
// text - the text the method type into the formula bar's input field.
function typeIntoFormulabar(text) {
	cy.log('>> typeIntoFormulabar - start');

	cy.cGet('#sc_input_window.formulabar')
		.then(function(cursor) {
			if (!Cypress.dom.isVisible(cursor)) {
				clickFormulaBar();
			}
		});

	cy.cGet('#sc_input_window.formulabar').click(); // This probably shouldn't be here, but acceptformula doesn't get visible without a click.
	cy.cGet('#sc_input_window.formulabar').should('have.class', 'focused');

	helper.doIfOnMobile(function() {
		cy.cGet('#toolbar-up #acceptformula').should('be.visible');
		cy.cGet('#toolbar-up #cancelformula').should('be.visible');
	});
	helper.doIfOnDesktop(function() {
		cy.cGet('#acceptformula').should('be.visible');
		cy.cGet('#cancelformula').should('be.visible');
	});

	cy.cGet('body').type(text);

	cy.log('<< typeIntoFormulabar - end');
}

// Remove exisiting text selection by clicking on
// row headers at the center position, until a
// a row is selected (and text seletion is removed).
function removeTextSelection() {
	cy.log('>> removeTextSelection - start');

	cy.cGet('[id="test-div-row header"]')
		.then(function(header) {
			expect(header).to.have.lengthOf(1);
			var rect = header[0].getBoundingClientRect();
			var posX = (rect.right + rect.left) / 2.0;
			var posY = (rect.top + rect.bottom) / 2.0;

			var moveY = 0.0;
			cy.waitUntil(function() {
				cy.cGet('body').click(posX, posY + moveY);

				moveY += 1.0;
				var regex = /A([0-9]+):(AMJ|XFD)\1$/;
				return cy.cGet(helper.addressInputSelector)
					.should('have.prop', 'value')
					.then(function(value) {
						return regex.test(value);
					});
			});
		});

	cy.log('<< removeTextSelection - end');
}

// Click on rows header and select Hide rows from a context menu
function hideSelectedRows() {
	cy.log('>> hideSelectedRows - start');

	cy.cGet('[id="test-div-row header"]')
		.then(function(header) {
			expect(header).to.have.lengthOf(1);
			var rect = header[0].getBoundingClientRect();
			var posX = (rect.right + rect.left) / 2.0;
			var posY = (rect.top + rect.bottom) / 2.0;
			cy.cGet('body').rightclick(posX, posY);
			cy.cGet('body').contains('.context-menu-item', 'Hide Rows').click();
			cy.cGet('.context-menu-list').should('not.be.visible');
		});

	cy.log('<< hideSelectedRows - end');
}

// Select the entire sheet using the "Select All" button
// at the corner of the row and column headers.
// Additionally, this method removes any preexisting text selection.
// Without this step, if there is an active text selection,
// the "Select All" command would only select the content
// of the currently edited cell instead of the entire table.
function selectEntireSheet() {
	cy.log('>> selectEntireSheet - start');

	removeTextSelection();

	cy.cGet('[id="test-div-corner header"]')
		.then(function(items) {
			expect(items).to.have.lengthOf(1);
			var corner = items[0];
			var XPos = (corner.getBoundingClientRect().right + items[0].getBoundingClientRect().left) / 2;
			var YPos = items[0].getBoundingClientRect().bottom - 10;
			cy.cGet('body').click(XPos, YPos);
		});

	helper.doIfOnMobile(function() {
		cy.cGet('#test-div-cell_selection_handle_start').should('exist');
	});

	var regex = /^A1:(AMJ|XFD)1048576$/;
	cy.cGet(helper.addressInputSelector)
		.should('have.prop', 'value')
		.then(function(value) {
			return regex.test(value);
		});

	cy.log('<< selectEntireSheet - end');
}

// Select first column of a calc document.
// We try to achive this by clicking on the left end
// of the column headers. Of course if the first column
// has a very small width, then this might fail.
function selectFirstColumn() {
	cy.log('>> selectFirstColumn - start');

	cy.cGet('[id="test-div-column header"]')
		.then(function(items) {
			expect(items).to.have.lengthOf(1);

			var bounds = items[0].getBoundingClientRect();
			var XPos = bounds.left + 10;
			var YPos = (bounds.top + bounds.bottom) / 2;
			cy.cGet('body').click(XPos, YPos);
		});

		cy.cGet(helper.addressInputSelector).should('have.prop', 'value', 'A1:A1048576');

	cy.log('<< selectFirstColumn - end');
}

function ensureViewContainsCellCursor() {
	cy.log('>> ensureViewContainsCellCursor - start');

	var sheetViewBounds = new helper.Bounds();
	var sheetCursorBounds = new helper.Bounds();

	helper.getOverlayItemBounds('#test-div-OwnCellCursor', sheetCursorBounds);
	helper.getItemBounds('#test-div-tiles', sheetViewBounds);

	cy.wrap(true).then(function () {
		cy.log('ensureViewContainsCellCursor: cursor-area is ' + sheetCursorBounds.toString() + ' view-area is ' + sheetViewBounds.toString());
		expect(sheetViewBounds.contains(sheetCursorBounds)).to.equal(true, 'view-area must contain cursor-area');
	});

	cy.log('<< ensureViewContainsCellCursor - end');
}

function assertSheetContents(expectedData, copy) {
	cy.log('>> assertSheetContents - start');

	selectEntireSheet();
	if (copy === true) {
		helper.copy();
	}
	assertDataClipboardTable(expectedData);

	cy.log('<< assertSheetContents - end');
}

function assertDataClipboardTable(expectedData) {
	cy.log('>> assertDataClipboardTable - start');

	cy.cGet('#copy-paste-container table td')
		.should('have.length', expectedData.length)
		.should(function($td) {
		var actualData = $td.map(function(i,el) {
			return Cypress.$(el).text();
		}).get();
		expect(actualData).to.deep.eq(expectedData);
	});

	cy.log('<< assertDataClipboardTable - end');
}

function selectCellsInRange(range) {
	cy.log('>> selectCellsInRange - start');

	cy.cGet(helper.addressInputSelector)
		.type('{selectall}{backspace}' + range + '{enter}');

	cy.log('<< selectCellsInRange - end');
}

function openAutoFilterMenu(secondColumn) {
	cy.log('>> openAutoFilterMenu - start');

	// Get canvas container first.
	// Then get its coordinates relative to window.
	// Then calculate the position of the autofilter easier.
	cy.cGet('#canvas-container').then(function(items) {
		const clientRect = items[0].getBoundingClientRect();
		let XPos = clientRect.left;
		let YPos = clientRect.top;

		cy.cGet('body').click(XPos + 147 + (secondColumn ? 103 : 0), YPos + 25);
	});

	cy.log('<< openAutoFilterMenu - end');
}

function assertNumberofSheets(n) {
	cy.log('>> assertNumberofSheets - start');

	cy.cGet('button.spreadsheet-tab').should('have.length', n);

	cy.log('>> assertNumberofSheets - end');
}

function selectOptionFromContextMenu(contextMenu) {
	cy.log('>> selectOptionFromContextMenu - start');

	cy.wait(1000);
	cy.cGet('.spreadsheet-tab.spreadsheet-tab-selected').rightclick();
	cy.cGet('body').contains('.ui-combobox-entry.jsdialog.ui-grid-cell', contextMenu).click();

	cy.log('>> selectOptionFromContextMenu - end');
}

function selectOptionMobileWizard(menu) {
	var eventOptions = {
		force: true,
		button: 0,
		pointerType: 'mouse'
	};

	cy.cGet('.spreadsheet-tab.spreadsheet-tab-selected')
		.trigger('pointerdown', eventOptions)
		.wait(1000);

	cy.cGet('body').contains('.ui-header.level-0.mobile-wizard.ui-widget', menu)
		.click();
}

// Wait for the core to be idle and then assert the address input has the expected value.
// This is useful after operations that navigate to a cell (e.g., find, goto)
// where there is a round-trip from browser to core before the address updates.
// Parameters:
// win - the frame window object (from cy.getFrameWindow())
// expectedAddress - the expected cell address (e.g., 'A1', 'C300')
function assertAddressAfterIdle(win, expectedAddress) {
	cy.log('>> assertAddressAfterIdle - start');
	cy.log('Param - expectedAddress: ' + expectedAddress);

	helper.processToIdle(win);
	// Use a longer timeout to account for slow operations like search wrap-around
	cy.cGet(helper.addressInputSelector, {timeout: 15000}).should('have.value', expectedAddress);

	cy.log('<< assertAddressAfterIdle - end');
}

module.exports.clickAtOffset = clickAtOffset;
module.exports.clickOnFirstCell = clickOnFirstCell;
module.exports.clickOnACell = clickOnACell;
module.exports.dblClickOnFirstCell = dblClickOnFirstCell;
module.exports.clickFormulaBar = clickFormulaBar;
module.exports.typeIntoFormulabar = typeIntoFormulabar;
module.exports.removeTextSelection = removeTextSelection;
module.exports.selectEntireSheet = selectEntireSheet;
module.exports.selectFirstColumn = selectFirstColumn;
module.exports.ensureViewContainsCellCursor = ensureViewContainsCellCursor;
module.exports.assertSheetContents = assertSheetContents;
module.exports.selectCellsInRange = selectCellsInRange;
module.exports.openAutoFilterMenu = openAutoFilterMenu;
module.exports.assertNumberofSheets = assertNumberofSheets;
module.exports.selectOptionFromContextMenu = selectOptionFromContextMenu;
module.exports.selectOptionMobileWizard = selectOptionMobileWizard;
module.exports.hideSelectedRows = hideSelectedRows;
module.exports.assertAddressAfterIdle = assertAddressAfterIdle;

// Navigate to a cell address by typing into the address input field,
// then wait for the core to be idle and assert the address is correct.
function enterCellAddressAndConfirm(win, address) {
	helper.typeIntoInputField(helper.addressInputSelector, address);
	assertAddressAfterIdle(win, address);
}
module.exports.enterCellAddressAndConfirm = enterCellAddressAndConfirm;
