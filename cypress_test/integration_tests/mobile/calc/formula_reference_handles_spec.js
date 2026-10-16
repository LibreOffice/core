/* global describe it cy beforeEach require expect */

var helper = require('../../common/helper');
var calcHelper = require('../../common/calc_helper');
var mobileHelper = require('../../common/mobile_helper');

describe(['tagmobile', 'tagnextcloud'], 'Cell handles on a formula reference.', function() {
	var formulaText = '#sc_input_window.formulabar .ui-custom-textarea-text-layer';

	beforeEach(function() {
		helper.setupAndLoadDocument('calc/formulabar.ods');
		mobileHelper.enableEditingMobile();
		cy.getFrameWindow().then((win) => {
			this.win = win;
		});
	});

	function handleCenter($handle) {
		var rectangle = $handle[0].getBoundingClientRect();
		return [rectangle.left + rectangle.width / 2, rectangle.top + rectangle.height / 2];
	}

	function cellClientRectangle(win, column, row) {
		var anchor = win.app.sectionContainer.getDocumentAnchor();
		var view = win.app.activeDocument.activeLayout.viewedRectangle;
		var cell = win.app.map._docLayer.sheetGeometry.getCellSimpleRectangle(column, row);
		var canvas = win.document.getElementById('canvas-container').getBoundingClientRect();
		var scale = win.app.dpiScale;
		return {
			left: canvas.left + (anchor[0] + cell.pX1 - view.pX1) / scale,
			top: canvas.top + (anchor[1] + cell.pY1 - view.pY1) / scale,
			right: canvas.left + (anchor[0] + cell.pX2 - view.pX1) / scale,
			bottom: canvas.top + (anchor[1] + cell.pY2 - view.pY1) / scale,
		};
	}

	// Leaves "=SUM()" in A2 with the caret between the parentheses.
	function startSumInA2() {
		calcHelper.clickOnFirstCell();
		helper.typeIntoInputField(helper.addressInputSelector, 'A2');
		calcHelper.typeIntoFormulabar('=su');

		cy.cGet('#mobile-wizard').should('be.visible');
		// A synthetic click blurs the window, which accepts the "=su" typed so far, so tap.
		cy.cGet('body').contains('.func-name', /^SUM$/).then(function($name) {
			var center = handleCenter($name);
			cy.cGet('body').realClick({ x: center[0], y: center[1] });
		});
		cy.cGet('#mobile-wizard').should('not.be.visible');
		cy.cGet(formulaText).should('have.text', '=SUM()');
	}

	// A synthetic click blurs the formula bar, which accepts the formula, so use a real one.
	function tapOnCell(win, column, row) {
		var cell = cellClientRectangle(win, column, row);
		cy.cGet('body').realClick({
			x: (cell.left + cell.right) / 2,
			y: (cell.top + cell.bottom) / 2,
		});
	}

	function expectHandleOnCorner(win, column, row) {
		cy.cGet('#test-div-cell_selection_handle_end').should(function($handle) {
			// Read inside the retried check, so a scroll in between is seen.
			var cell = cellClientRectangle(win, column, row);
			var center = handleCenter($handle);
			expect(center[0]).to.be.closeTo(cell.right, 3);
			expect(center[1]).to.be.closeTo(cell.bottom, 3);
		});
	}

	it('Dragging the end handle widens the reference', function() {
		var win = this.win;

		startSumInA2();
		tapOnCell(win, 1, 1);
		cy.cGet(formulaText).should('have.text', '=SUM(B2)');

		expectHandleOnCorner(win, 1, 1);

		cy.cGet('#test-div-selection_start_handle').should('not.exist');
		cy.cGet('#test-div-selection_end_handle').should('not.exist');

		// Moves are dropped while the click timer of the tap is pending.
		helper.waitForTimers(win, 'clicktimer');
		helper.processToIdle(win);

		var closings;
		cy.then(function() {
			closings = mobileHelper.watchKeyboardClosing(win);
		});

		// The handle sits on the far corner of B2, so this lands inside B4.
		cy.cGet('#test-div-cell_selection_handle_end').then(function($handle) {
			var start = handleCenter($handle);
			var sheetGeometry = win.app.map._docLayer.sheetGeometry;
			var rows = sheetGeometry.getCellSimpleRectangle(1, 2).pHeight +
				sheetGeometry.getCellSimpleRectangle(1, 3).pHeight;
			var endX = start[0] - 4;
			var endY = start[1] + Math.round(rows / win.app.dpiScale) - 4;

			cy.cGet('body').realMouseDown({ x: start[0], y: start[1] });
			cy.cGet('body').realMouseMove(endX, endY);
			cy.cGet('body').realMouseUp({ x: endX, y: endY });
		});

		cy.cGet(formulaText).should('have.text', '=SUM(B2:B4)');

		expectHandleOnCorner(win, 1, 3);

		cy.then(function() {
			expect(closings.count).to.equal(0);
			expect(win.app.map.canAcceptKeyboardInput()).to.be.true;
		});

		helper.processToIdle(win);

		// Accepting moves the cell cursor down a row.
		cy.cGet(helper.addressInputSelector).invoke('val').then(function(address) {
			cy.cGet('#toolbar-up #acceptformula').click();
			helper.typeIntoInputField(helper.addressInputSelector, address);
			cy.cGet(formulaText).should('have.text', '=SUM(B2:B4)');

			expectHandleOnCorner(win, address.charCodeAt(0) - 'A'.charCodeAt(0),
				parseInt(address.substring(1)) - 1);
		});
	});

	it('The handles return to the edited cell when typing continues', function() {
		var win = this.win;

		calcHelper.clickOnFirstCell();
		helper.typeIntoInputField(helper.addressInputSelector, 'A2');
		helper.typeIntoDocument('=2*');
		cy.cGet(formulaText).should('have.text', '=2*');

		tapOnCell(win, 1, 1);
		cy.cGet(formulaText).should('have.text', '=2*B2');
		expectHandleOnCorner(win, 1, 1);

		helper.typeIntoDocument('+1');
		cy.cGet(formulaText).should('have.text', '=2*B2+1');
		expectHandleOnCorner(win, 0, 1);
	});

	// Scrolls A2, the edited cell, out of the view and taps a cell in view into the formula.
	// Neither the tap nor the keyboard relayouts around it may scroll the view back to A2.
	function pickReferenceInScrolledView(win, textBefore, textAfter) {
		var row;
		var viewTop;

		function expectViewKept(getText) {
			cy.then(function() {
				helper.processToIdle(win);
			});
			cy.then(function() {
				expect(win.app.activeDocument.activeLayout.viewedRectangle.pY1).to.equal(viewTop);
				cy.cGet(formulaText).should('have.text', getText());
			});
		}

		function textBeforeTap() {
			return textBefore + textAfter;
		}

		function textAfterTap() {
			return textBefore + 'B' + (row + 1) + textAfter;
		}

		cy.then(function() {
			win.app.sectionContainer.getSectionWithName(win.app.CSections.Scroll.name)
				.scrollVerticalWithOffset(400 * win.app.dpiScale);
			helper.processToIdle(win);
		});
		cy.then(function() {
			var geometry = win.app.map._docLayer.sheetGeometry;
			viewTop = win.app.activeDocument.activeLayout.viewedRectangle.pY1;
			expect(viewTop).to.be.greaterThan(geometry.getCellSimpleRectangle(0, 1).pY2);
		});
		mobileHelper.relayoutForKeyboard(win, function() {
			expectViewKept(textBeforeTap);
		});

		cy.then(function() {
			var geometry = win.app.map._docLayer.sheetGeometry;

			row = 2;
			while (geometry.getCellSimpleRectangle(1, row).pY1 < viewTop + 40 * win.app.dpiScale)
				row++;
			tapOnCell(win, 1, row);
		});
		cy.then(function() {
			expectHandleOnCorner(win, 1, row);
		});
		expectViewKept(textAfterTap);
		mobileHelper.relayoutForKeyboard(win, function() {
			expectViewKept(textAfterTap);
		});
	}

	it('Keep the scrolled view while a reference is picked', function() {
		startSumInA2();
		pickReferenceInScrolledView(this.win, '=SUM(', ')');
	});

	// "=2*" opens no function list, so the edit stays in the cell.
	it('Keep the scrolled view while a reference is picked in the cell', function() {
		calcHelper.clickOnFirstCell();
		helper.typeIntoInputField(helper.addressInputSelector, 'A2');
		helper.typeIntoDocument('=2*');
		cy.cGet(formulaText).should('have.text', '=2*');
		cy.cGet('#mobile-wizard').should('not.be.visible');

		pickReferenceInScrolledView(this.win, '=2*', '');
	});
});
