/* global describe it cy beforeEach require expect */

var helper = require('../../common/helper');
var calcHelper = require('../../common/calc_helper');
var desktopHelper = require('../../common/desktop_helper');

describe(['tagmultiuser'], 'Check following the other views', function() {

	beforeEach(function() {
		helper.setupAndLoadDocument('calc/following.ods',true);
		desktopHelper.switchUIToNotebookbar();
	});

	it('Stop following on click', function() {
		// second view follow the first one
		cy.cSetActiveFrame('#iframe2');
		cy.cGet('#userListHeader').click();
		cy.cGet('.user-list-item').eq(1).click();
		cy.cGet('.jsdialog-overlay').should('not.exist');

		// second view clicks on the cell
		cy.cGet('#followingChip').should('be.visible');
		calcHelper.clickOnFirstCell();

		// following is off
		cy.cGet('#followingChip').should('not.be.visible');
	});

	it('Stop following on formulabar', function() {
		// second view follow the first one
		cy.cSetActiveFrame('#iframe2');
		cy.cGet('#userListHeader').click();
		cy.cGet('.user-list-item').eq(1).click();
		cy.cGet('.jsdialog-overlay').should('not.exist');

		// second view activates the formulabar
		cy.cGet('#followingChip').should('be.visible');
		cy.cGet('#sc_input_window').click();

		// following is off
		cy.cGet('#followingChip').should('not.be.visible');
	});

	// Write into one view's cell, leaving the text committed and the core idle.
	function writeIntoCell(frame, address, text) {
		const formulaBar = '#sc_input_window .ui-custom-textarea-text-layer';

		cy.cSetActiveFrame(frame);
		cy.getFrameWindow().then(function(win) {
			calcHelper.enterCellAddressAndConfirm(win, address);
		});
		cy.cGet(formulaBar).click();
		cy.cGet(formulaBar).type(text + '{enter}');
		cy.getFrameWindow().then(function(win) {
			helper.processToIdle(win);
		});
	}

	it('Follow a view that wrote far down the sheet', function() {
		const FOLLOWED_ROW_INDEX = 289919;

		writeIntoCell('#iframe1', 'C289920', 'far');
		writeIntoCell('#iframe2', 'C7', 'near');

		// The second view starts following the first one.
		cy.cGet('#userListHeader').click();
		cy.cGet('.user-list-item').eq(1).click();
		cy.cGet('.jsdialog-overlay').should('not.exist');

		// The second view is looking at the row the first view is on.
		cy.getFrameWindow().then(function(win2) {
			cy.wrap(win2).should(function(win) {
				const rows = win.app.map._docLayer.sheetGeometry.getRowsGeometry();
				const viewed = win.app.activeDocument.activeLayout.viewedRectangle;
				const firstRow = rows.getIndexFromPos(viewed.y1, 'tiletwips');
				const lastRow = rows.getIndexFromPos(viewed.y2, 'tiletwips');
				expect(firstRow, 'first visible row').to.be.at.most(FOLLOWED_ROW_INDEX);
				expect(lastRow, 'last visible row').to.be.at.least(FOLLOWED_ROW_INDEX);
			});
		});
	});
});

describe(['tagmultiuser'], 'A user with two connections', function() {

	beforeEach(function() {
		// One user opens the document in iframe2 and in iframe3, so that user has
		// two connections. iframe1 is somebody else and does the following.
		// The document has content down to row 588, so a view can scroll to the
		// cells the test works in.
		helper.setupAndLoadDocument('calc/cell_cursor_jump.ods', true, false, undefined,
			'userid1=test&userid2=test2&userid3=test2');

		cy.cSetActiveFrame('#iframe3');
		helper.documentChecks(true);

		cy.cSetActiveFrame('#iframe1');
		desktopHelper.switchUIToNotebookbar();
	});

	// Moves the cell cursor of the active frame and gives back where that cell is
	// in cell.y, for the assertion that follows.
	function moveCellCursor(address, cell) {
		cy.cGet(helper.addressInputSelector).type('{selectAll}' + address + '{enter}');
		cy.getFrameWindow().then(function(win) {
			calcHelper.assertAddressAfterIdle(win, address);
		});
		cy.getFrameWindow().then(function(win) {
			cell.y = win.app.calc.cellCursor.rectangle.y1;
		});
	}

	// The follower ends up looking at the cell the other user works in.
	function assertFollowerShows(cell) {
		cy.cSetActiveFrame('#iframe1');
		cy.getFrameWindow().should(function(win) {
			const viewed = win.app.activeDocument.activeLayout.viewedRectangle;
			expect(viewed.y1, 'top of the followed area').to.be.at.most(cell.y);
			expect(viewed.y2, 'bottom of the followed area').to.be.at.least(cell.y);
		});
	}

	it('Has one entry in the list and is followed in whichever connection it works', function() {
		// Three views are open, and the two that belong to one user share an
		// entry, so the list and the avatars in the header show two users.
		cy.getFrameWindow().should(function(win) {
			expect(Object.keys(win.app.map._viewInfo)).to.have.length(3);
		});

		cy.cGet('#userListHeader').click();
		cy.cGet('.user-list-item').should('have.length', 2);
		cy.cGet('#userListSummaryButton img').should('have.length', 2);

		// Follow the user with the two connections.
		cy.cGet('.user-list-item').eq(1).click();
		cy.cGet('#followingChip').should('be.visible');

		// The followed user works in the first of their connections.
		const firstCell = {};
		cy.cSetActiveFrame('#iframe2');
		moveCellCursor('A200', firstCell);
		assertFollowerShows(firstCell);

		// The same user carries on in their other connection.
		const otherCell = {};
		cy.cSetActiveFrame('#iframe3');
		moveCellCursor('A400', otherCell);
		assertFollowerShows(otherCell);
	});
});
