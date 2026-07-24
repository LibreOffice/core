/* global describe it cy beforeEach require */

var helper = require('../../common/helper');
var calcHelper = require('../../common/calc_helper');
var desktopHelper = require('../../common/desktop_helper');

var dimDialogSelector = '#modal-dialog-inactive_user_message-overlay';

function checkIfIsInteractiveAgain(selectViaNameBox) {
	cy.getFrameWindow()
		.then(function(win) {
			var ih = win.app.idleHandler, um = win.app.map.uiManager;
			// Keep the document from going idle again during the check:
			win.idleTimeoutSecs = 100000;
			ih.notifyActive();
			um.closeModal(um.generateModalId(ih.dimId));
			// Remove any popup left on screen from before the document dimmed:
			var jsdialog = win.app.map.jsdialog;
			Object.keys(jsdialog.dialogs).forEach(function(id) {
				var dialog = jsdialog.dialogs[id];
				if (dialog.isPopup && dialog.overlay
					&& dialog.overlay.classList.contains('cancellable'))
					jsdialog.clearDialog(id);
			});
		});
	cy.cGet(dimDialogSelector).should('not.exist');

	calcHelper.dblClickOnFirstCell();

	const content = 'New content';
	helper.typeIntoDocument(content + '{enter}');

	const expected = ['Cypress ' + content + 'Test', 'Status', 'Test 1', 'Pass', 'Test 2', 'Fail', 'Test 3', 'Pass', 'Test 4', '', 'Test 5', 'Fail'];

	if (selectViaNameBox) {
		cy.getFrameWindow().then(function(win) {
			const range = 'A1:' + calcHelper.columnNumberToLabel(win.app.calc.maxColumnCount) + win.app.calc.maxRowCount;
			calcHelper.selectCellsInRange(range);
		});
		helper.copy();
		calcHelper.assertDataClipboardTable(expected);
	} else {
		calcHelper.assertSheetContents(expected, true);
	}
}

describe(['tagdesktop'], 'Idle', function() {
	beforeEach(function() {
		helper.setupAndLoadDocument('calc/idle.ods');
	});

	it('Check idle out of focus', function() {
		helper.setDummyClipboardForCopy();
		cy.getFrameWindow()
			.its('L')
			.then(function(L) {
				L.Map.THIS._onLostFocus();
			});

		cy.cGet(dimDialogSelector, { timeout: 1000 }).should('not.exist');
		cy.wait(1100); // out of focus timeout is 1s
		cy.cGet(dimDialogSelector, { timeout: 1000 }).should('exist');

		checkIfIsInteractiveAgain();
	});

	it('Check idle after inactivity', function() {
		helper.setDummyClipboardForCopy();
		cy.cGet(dimDialogSelector).should('not.exist');
		cy.wait(7100); // inactivity timeout is 7s
		cy.cGet(dimDialogSelector).should('exist');

		checkIfIsInteractiveAgain();
	});

	it('Check interactivity of document after dialog close', function() {
		helper.setDummyClipboardForCopy();
		// Check if sidebar-dock-wrapper is visible
		cy.cGet('#sidebar-dock-wrapper').should('be.visible').then(($sidebar) => {
			// If it's not visible, click on SidebarDeck.PropertyDeck to make it visible
			if (!$sidebar.is(':visible')) {
				desktopHelper.sidebarToggle();
			}
		});
		cy.cGet('#sidebar-dock-wrapper .unoUnderline .arrowbackground').click();
		cy.cGet('.jsdialog-window.modalpopup').should('exist');
		cy.cGet(dimDialogSelector).should('not.exist');
		cy.wait(7100); // inactivity timeout is 7s
		cy.cGet(dimDialogSelector).should('exist');

		// check if cell is editable or not after document again become active
		checkIfIsInteractiveAgain(true);

		// Make sure the sidebar dropdown is closed after document again become interactive
		cy.cGet('.jsdialog-window.modalpopup').should('not.exist');
	});
});

describe(['tagdesktop'], 'Idle recover with comment', function() {
	beforeEach(function() {
		helper.setupAndLoadDocument('calc/far_comment.ods');
	});

	it('Check if we jump to comment on activation', function() {
		desktopHelper.assertScrollbarPosition('vertical', 10, 30);

		helper.setDummyClipboardForCopy();
		cy.getFrameWindow()
			.its('L')
			.then(function(L) {
				L.Map.THIS._onLostFocus();
			});

		cy.cGet(dimDialogSelector, { timeout: 1000 }).should('not.exist');
		cy.wait(1100); // out of focus timeout is 1s
		cy.cGet(dimDialogSelector, { timeout: 1000 }).should('exist');

		cy.getFrameWindow()
			.its('app')
			.then(function(app) {
				app.idleHandler._activate();

				cy.wait(500);

				desktopHelper.assertScrollbarPosition('vertical', 10, 30);
			});
	});
});
