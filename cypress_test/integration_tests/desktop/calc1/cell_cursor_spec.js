/* global describe it cy beforeEach expect require */

var helper = require('../../common/helper');
var calcHelper = require('../../common/calc_helper');
var desktopHelper = require('../../common/desktop_helper');

// That properties popup doesn't go by itself.
// So I close it here in order to prevent this test from failure when we fix that popup closing issue.
function closeNotebookbarPopup() {
	cy.cGet('body').type('{esc}');
	cy.cGet('#document-canvas').realClick();
	cy.cGet('.jsdialog-overlay').should('not.exist');
}

describe(['tagdesktop', 'tagnextcloud', 'tagproxy'], 'Test jumping on large cell selection', function() {

	beforeEach(function() {
		helper.setupAndLoadDocument('calc/cell_cursor.ods');
		cy.getFrameWindow().then((win) => {
			this.win = win;
		});
	});

	it('No jump on long merged cell', function() {
		desktopHelper.assertScrollbarPosition('horizontal', 205, 330);
		calcHelper.clickOnFirstCell(true, false, 'A1:Z1');
		desktopHelper.assertScrollbarPosition('horizontal', 205, 330);
	});

	it('Jump on address with not visible cursor', function() {
		desktopHelper.assertScrollbarPosition('vertical', 0, 30);
		cy.cGet(helper.addressInputSelector).should('have.value', 'Z11');

		helper.typeIntoInputField(helper.addressInputSelector, 'A110');
		desktopHelper.assertScrollbarPosition('vertical', 205, 330);
	});

	it('Jump on search with not visible cursor', function() {
		desktopHelper.assertScrollbarPosition('vertical', 0, 30);
		cy.cGet(helper.addressInputSelector).should('have.value', 'Z11');

		desktopHelper.assertScrollbarPosition('horizontal', 205, 330);
		helper.typeIntoDocument('{ctrl}f');
		cy.cGet('input#searchterm-input-dialog').type('{selectAll}FIRST');
		cy.cGet('#search').find('button').click();

		cy.cGet(helper.addressInputSelector).should('have.value', 'A10');
		desktopHelper.assertScrollbarPosition('horizontal', 40, 60);
	});

	it('Show cursor on sheet insertion', function() {
		// scroll down
		helper.typeIntoInputField(helper.addressInputSelector, 'A110');
		desktopHelper.assertScrollbarPosition('vertical', 205, 330);

		// insert sheet before
		calcHelper.selectOptionFromContextMenu('Insert sheet before this');

		// we should see the top left corner of the sheet
		calcHelper.assertAddressAfterIdle(this.win, 'A1');
		desktopHelper.assertScrollbarPosition('vertical', 0, 30);
	});

	it('Scroll and check drawing on frozen part of the view', function() {
		// We will add a new sheet. Go to a cell other than A1. We will check if the new sheet is added by checking the current cell.
		calcHelper.enterCellAddressAndConfirm(this.win, 'B2');

		// Add a new sheet.
		cy.cGet('#insertsheet-button').click();
		// Cell cursor will go to A1 by default. So we understand that the new sheet is added.
		calcHelper.assertAddressAfterIdle(this.win, 'A1');

		// Go to a cell that we know is visible.
		calcHelper.enterCellAddressAndConfirm(this.win, 'D7');

		// Find freeze panes button and click.
		cy.cGet('#View-tab-label').click();
		desktopHelper.getNbIconArrow('FreezePanes').click();
		// There are two FreezePanes buttons, the first in the main
		// toolbar we clicked to create the dropdown in which the
		// second appears. We want to wait until that second one is
		// available and click that one, not reclick the first.
		desktopHelper.getNbIcon('FreezePanes').should('have.length', 2).last().click();
		closeNotebookbarPopup();

		// Wait for freeze panes statechanged message to arrive from core.
		helper.waitForMapState('.uno:FreezePanes', 'true');

		// Scroll down.
		calcHelper.enterCellAddressAndConfirm(this.win, 'Z110');

		// Now click on A1. Use click for this, not the input field. We also need to test the core coordinates.
		calcHelper.clickOnFirstCell();

		// Before the fix for mouse coordinate calculation, this would not go to A1, but somewhere else.
		// Core side coordinates were not calculated properly.
		// Fix is here: https://github.com/CollaboraOnline/online/pull/13631
		cy.cGet(helper.addressInputSelector).should('have.value', 'A1');
	});

	it('Check selected text visual.', function() {
		cy.cGet('#insertsheet-button').click();

		helper.processToIdle(this.win);

		// Ensure starting point.
		helper.typeIntoInputField(helper.addressInputSelector, 'A1');

		// Put cell cursor somewhere else.
		helper.typeIntoInputField(helper.addressInputSelector, 'B10');

		desktopHelper.getNbIconArrow('AlignTop').click();
		desktopHelper.getNbIcon('WrapText').click();

		closeNotebookbarPopup();
		helper.typeIntoInputField(helper.addressInputSelector, 'B10');

		helper.typeIntoDocument('Lorem ipsum dolor sit amet. Lorem ipsum dolor sit amet. Lorem ipsum dolor sit amet. Lorem ipsum dolor sit amet.');
		helper.typeIntoDocument('{ctrl}a');

		helper.processToIdle(this.win);

		cy.cGet('#document-container').compareSnapshot('text-selection', 0.02);
	});

	it('Check right click shows correct context menu.', function() {
		cy.cGet('#document-container').then(function(items) {
			const rect = items[0].getBoundingClientRect();
			const centerX = rect.left + rect.width / 2;
			const centerY = rect.top + rect.height / 2;
			const topY = rect.top + 2;

			// Show column context menu first.
			// Real mouse move to trigger the issue that previous commit fixes.
			cy.cGet('body').realMouseMove(centerX, topY);
			cy.cGet('body').rightclick(centerX, topY);

			cy.cGet('body').contains('.ui-combobox-entry.jsdialog.ui-grid-cell', 'Insert Columns Before')
				.should('exist')
				.should('be.visible');

			cy.cGet('body').realMouseMove(centerX, centerY);
			// click in the document to close the header context menu.
			cy.cGet('body').realClick();

			cy.cGet('body').contains('.ui-combobox-entry.jsdialog.ui-grid-cell', 'Insert Columns Before').should('not.exist');

			// Now show document context menu.
			cy.cGet('body').rightclick(centerX, centerY);

			// Note: The context menu of the document area uses jsdialog dropdown.
			const pasteEntry = helper.getContextMenuItem('Paste');
			pasteEntry.should('exist');
			pasteEntry.should('be.visible');
		});
	});
});

describe(['tagdesktop', 'tagnextcloud', 'tagproxy'], 'Test Cell Selections', function() {
	beforeEach(function() {
		helper.setupAndLoadDocument('calc/empty-selections.ods');
		desktopHelper.sidebarToggle();
		cy.cGet('#sidebar-dock-wrapper').should('not.be.visible');
		cy.viewport(1000, helper.maxScreenshotableViewportHeight);
		cy.getFrameWindow().then((win) => {
			this.win = win;
			helper.processToIdle(win);
		});
	});

	it('Check non-range cell selection with CTRL', function() {
		calcHelper.clickOnACell(1, 1, 2, 3);

		// Press CTRL and hold.
		cy.cGet('div.clipboard').type('{ctrl}', { force: true, release: false });

		cy.wait(500);
		calcHelper.clickOnACell(2, 3, 4, 3);

		cy.wait(500);
		calcHelper.clickOnACell(4, 3, 2, 6);

		// Press SHIFT and hold.
		cy.cGet('div.clipboard').type('{shift}', { force: true, release: false });

		cy.wait(500);
		calcHelper.clickOnACell(2, 6, 2, 10);

		helper.processToIdle(this.win);

		cy.cGet('#document-container').compareSnapshot('selections', 0.02);
	});

	it('Should not scroll after a right click', function() {
		helper.typeIntoInputField(helper.addressInputSelector, 'Z1000');

		cy.cGet('#document-container').rightclick();
		const pasteEntry = helper.getContextMenuItem('Paste');
		pasteEntry.should('exist');
		pasteEntry.should('be.visible');

		cy.cGet('#document-container').then(function(items) {
			const rect = items[0].getBoundingClientRect();
			const left = rect.left + 20;
			const topY = rect.top + 20;

			cy.cGet('body').click(left, topY);


			// We clicked on right button, then left button. Then we will move the mouse outside of the view.
			// It shouldn't scroll when the mouse is outside.
			cy.cGet('#document-container').realMouseMove(left + 50, topY + 50);
			cy.cGet('#document-container').realMouseMove(left + 75, topY + 75);
			cy.cGet('#document-container').realMouseMove(left + 100, topY + 100);
			cy.cGet('#document-container').realMouseMove(left + 125, topY + 125);
			cy.cGet('#document-container').realMouseMove(left + 150, topY + 150);
		});

		cy.wait(1000);

		// This doesn't pass without the fix in this commit.
		cy.cGet('#document-container').compareSnapshot('scroll-check', 0.02);
	});
});

describe(['tagdesktop', 'tagnextcloud', 'tagproxy'], 'Test jumping on large cell selection with split panes', function() {

	beforeEach(function() {
		helper.setupAndLoadDocument('calc/cell_cursor_split.ods');
	});

	it('No jump on long merged cell with split panes', function() {
		desktopHelper.assertScrollbarPosition('horizontal', 270, 390);

		// Click on second cell in second row
		cy.cGet('#map')
		.then(function(items) {
			expect(items).to.have.lengthOf(1);
			var XPos = items[0].getBoundingClientRect().left + 140;
			var YPos = items[0].getBoundingClientRect().top + 30;
			cy.cGet('body').click(XPos, YPos);
		});

		cy.cGet(helper.addressInputSelector).should('have.value', 'B2:AA2');
		desktopHelper.assertScrollbarPosition('horizontal', 270, 390);
	});
});

describe(['tagdesktop', 'tagnextcloud', 'tagproxy'], 'Test triple click content selection.', function() {

	beforeEach(function() {
		helper.setupAndLoadDocument('calc/cell-content-selection.ods');
		cy.getFrameWindow().then((win) => {
			this.win = win;
		});
	});

	it('Triple click should select the cell content.', function() {
		calcHelper.enterCellAddressAndConfirm(this.win, 'A1');

		// Triple click on second first in second row
		cy.cGet('#document-container')
		.then(function(items) {
			expect(items).to.have.lengthOf(1);
			var XPos = items[0].getBoundingClientRect().left + 60;
			var YPos = items[0].getBoundingClientRect().top + 30;
			cy.cGet('body').realClick({position: {x: XPos, y: YPos}, clickCount: 3}) // Triple click.
		});

		helper.waitForTimers(this.win, 'clicktimer');
		helper.processToIdle(this.win);

		cy.cGet('#document-container').compareSnapshot('triple-click', 0.02);

	});
});

describe(['tagdesktop', 'tagnextcloud', 'tagproxy'], 'Test decimal separator of cells with different languages.', function() {
	beforeEach(function() {
		helper.setupAndLoadDocument('calc/decimal_separator.ods');
		cy.getFrameWindow().then((win) => {
			this.win = win;
		});
	});

	it('Check different decimal separators', function() {
		calcHelper.enterCellAddressAndConfirm(this.win, 'A1');

		cy.wrap(this.win).then(win => {
			cy.expect(win.app.calc.decimalSeparator).to.be.equal('.');
		});

		calcHelper.enterCellAddressAndConfirm(this.win, 'B1');

		cy.wrap(this.win).then(win => {
			cy.expect(win.app.calc.decimalSeparator).to.be.equal(',');
		});
	});
});
