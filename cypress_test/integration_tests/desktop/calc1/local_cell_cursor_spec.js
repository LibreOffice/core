/* global describe it cy beforeEach expect require Cypress */

var helper = require('../../common/helper');
var calcHelper = require('../../common/calc_helper');

// Get the last setcellcursor message the client sent, or null.
function lastSetCellCursor(win) {
	var last = null;
	win.app.socket.sendMessage.getCalls().forEach(function(call) {
		var message = call.args[0];
		if (typeof message === 'string' && message.startsWith('setcellcursor '))
			last = message;
	});
	return last;
}

// Get the messages of a type that the client received, such as
// 'cellcursorstate:'.
function receivedMessages(win, type) {
	var messages = [];
	win.app.socket._onMessage.getCalls().forEach(function(call) {
		var textMsg = call.args[0] && call.args[0].textMsg;
		if (typeof textMsg === 'string' && textMsg.startsWith(type))
			messages.push(textMsg);
	});
	return messages;
}

// Get the last cell cursor state the client received, or null.
function lastCellCursorState(win) {
	var messages = receivedMessages(win, 'cellcursorstate:');
	if (!messages.length)
		return null;
	var last = messages[messages.length - 1];
	return JSON.parse(last.substring('cellcursorstate:'.length));
}

// Get the page position of the center of a cell, from the cell cursor on A1.
// Columns and rows are 0-based, and every cell of the sheet has the same size.
function cellCenter(cursorRect, col, row) {
	return {
		x: Math.floor(cursorRect.left + cursorRect.width * (col + 0.5)),
		y: Math.floor(cursorRect.top + cursorRect.height * (row + 0.5)),
	};
}

// Move the cell cursor to A1, and pass the screen position and size of A1
// to the callback.
function withCursorOnA1(win, callback) {
	calcHelper.enterCellAddressAndConfirm(win, 'A1');
	cy.cGet('#test-div-OwnCellCursor').then(function(items) {
		expect(items).to.have.lengthOf(1);
		callback(items[0].getBoundingClientRect());
	});
}

// Select the range between two cells with a mouse drag.
function selectRangeWithMouse(cursorRect, from, to) {
	var start = cellCenter(cursorRect, from[0], from[1]);
	var end = cellCenter(cursorRect, to[0], to[1]);
	cy.cGet('body').realMouseDown({ x: start.x, y: start.y });
	cy.cGet('body').realMouseMove(end.x, end.y);
	cy.cGet('body').realMouseUp({ x: end.x, y: end.y });
}

// Wait until the client reported its last move and the engine answered it.
function waitForReport(win) {
	helper.waitForTimers(win, 'localcellcursorrest');
	helper.processToIdle(win);
}

describe(['tagdesktop'], 'Local cell cursor and selection', function() {

	beforeEach(function() {
		// Turn on the local cell cursor, which is off by default.
		helper.setupAndLoadDocument('calc/empty-selections.ods', false, false, undefined, 'localcellcursor=true');
		cy.getFrameWindow().then(function(win) {
			this.win = win;
			Cypress.sinon.spy(win.app.socket, 'sendMessage');
			Cypress.sinon.spy(win.app.socket, '_onMessage');
			helper.processToIdle(win);
		}.bind(this));
	});

	it('moves the cell cursor and extends the selection with the keys', function() {
		calcHelper.enterCellAddressAndConfirm(this.win, 'A1');

		// Move to B2, then select B2:D3 from there.
		helper.typeIntoDocument('{rightArrow}{downArrow}');
		waitForReport(this.win);
		calcHelper.assertAddressAfterIdle(this.win, 'B2');
		cy.wrap(null).should(function() {
			// A move with nothing selected carries no anchor.
			expect(lastSetCellCursor(this.win)).to.match(/^setcellcursor part=0 col=1 row=1 sequence=\d+$/);
		}.bind(this));

		helper.typeIntoDocument('{shift}{rightArrow}{rightArrow}{downArrow}');
		waitForReport(this.win);
		calcHelper.assertAddressAfterIdle(this.win, 'B2:D3');
		cy.wrap(null).should(function() {
			expect(lastSetCellCursor(this.win)).to.contain('col=3 row=2 anchorcol=1 anchorrow=1');
		}.bind(this));

		// Shrink the selection back to B2:C3.
		helper.typeIntoDocument('{shift}{leftArrow}');
		waitForReport(this.win);
		calcHelper.assertAddressAfterIdle(this.win, 'B2:C3');

		// Select C4:E6.  The engine should report the cursor and the
		// selection in one cell cursor state, and send no cellselection.
		calcHelper.enterCellAddressAndConfirm(this.win, 'C4:E6');
		cy.wrap(null).should(function() {
			var state = lastCellCursorState(this.win);
			expect(state.cursor).to.deep.equal({ cell: [2, 3], visible: true });
			expect(state.selection).to.deep.equal([[2, 3, 4, 5]]);
			expect(receivedMessages(this.win, 'cellselection:')).to.have.lengthOf(0);
		}.bind(this));

		// Extend the engine's selection to C4:E7 on the client.
		helper.typeIntoDocument('{shift}{downArrow}');
		waitForReport(this.win);
		calcHelper.assertAddressAfterIdle(this.win, 'C4:E7');
		cy.wrap(null).should(function() {
			expect(lastSetCellCursor(this.win)).to.contain('col=4 row=6 anchorcol=2 anchorrow=3');
		}.bind(this));

		// Move to B9 with an arrow key, then to D9 with Tab.  The engine
		// should take the Tab presses and start their run from B9.
		calcHelper.enterCellAddressAndConfirm(this.win, 'A9');
		helper.typeIntoDocument('{rightArrow}');
		cy.realPress('Tab');
		cy.realPress('Tab');
		calcHelper.assertAddressAfterIdle(this.win, 'D9');

		// Press Enter; the cursor should go below the cell that the Tab run
		// started from.
		helper.typeIntoDocument('{enter}');
		calcHelper.assertAddressAfterIdle(this.win, 'B10');
	});

	it('moves and selects over a merged cell', function() {
		// Merge B2:C3.
		calcHelper.enterCellAddressAndConfirm(this.win, 'B2:C3');
		cy.then(function() {
			this.win.app.map.sendUnoCommand('.uno:ToggleMergeCells');
		}.bind(this));
		helper.processToIdle(this.win);

		// Move right from A3 into the merged cell, which puts the cursor on
		// its first cell, B2.  The Name Box shows a merged cell as a range.
		calcHelper.enterCellAddressAndConfirm(this.win, 'A3');
		helper.typeIntoDocument('{rightArrow}');
		waitForReport(this.win);
		calcHelper.assertAddressAfterIdle(this.win, 'B2:C3');
		cy.wrap(null).should(function() {
			expect(lastSetCellCursor(this.win)).to.match(/^setcellcursor part=0 col=1 row=1 sequence=\d+$/);
		}.bind(this));

		// Move right again to leave the merged cell for D2.
		helper.typeIntoDocument('{rightArrow}');
		waitForReport(this.win);
		calcHelper.assertAddressAfterIdle(this.win, 'D2');

		// Move back left into the merged cell, then down to leave it for B4.
		helper.typeIntoDocument('{leftArrow}');
		waitForReport(this.win);
		calcHelper.assertAddressAfterIdle(this.win, 'B2:C3');
		helper.typeIntoDocument('{downArrow}');
		waitForReport(this.win);
		calcHelper.assertAddressAfterIdle(this.win, 'B4');

		// Extend the selection from A1 to B1, then down to B2, which the
		// merged cell covers, so the selection grows to A1:C3.  But keep
		// the logical range to be A1:B2.
		calcHelper.enterCellAddressAndConfirm(this.win, 'A1');
		helper.typeIntoDocument('{shift}{rightArrow}{downArrow}');
		waitForReport(this.win);
		calcHelper.assertAddressAfterIdle(this.win, 'A1:C3');
		cy.wrap(null).should(function() {
			// the client should send the logical range to engine
			expect(lastSetCellCursor(this.win)).to.contain('col=1 row=1 anchorcol=0 anchorrow=0');
			// the engine should send back to the client the grown range
			expect(lastCellCursorState(this.win).selection).to.deep.equal([[0, 0, 2, 2]]);
		}.bind(this));

		// Shrink it back up, out of the merged cell, to A1:B1.  We kept the
		// previous logical range to be A1:B2 because of this behavior.
		helper.typeIntoDocument('{shift}{upArrow}');
		waitForReport(this.win);
		calcHelper.assertAddressAfterIdle(this.win, 'A1:B1');
	});

	it('selects cells with Shift+click and a mouse drag', function() {
		withCursorOnA1(this.win, function(cursorRect) {
			// Click on cell C3 while Shift is pressed
			var c3 = cellCenter(cursorRect, 2, 2); // center of cell C3
			cy.cGet('body').realClick({ x: c3.x, y: c3.y, shiftKey: true });
			helper.waitForTimers(this.win, 'clicktimer');
			waitForReport(this.win);

			// Make sure A1:C3 is selected with A1 being the anchor.
			calcHelper.assertAddressAfterIdle(this.win, 'A1:C3');
			cy.wrap(null).should(function() {
				expect(lastSetCellCursor(this.win)).to.contain('col=2 row=2 anchorcol=0 anchorrow=0');
			}.bind(this));

			// Select D2:F4 with the mouse cursor from D2 to F4.
			selectRangeWithMouse(cursorRect, [3, 1], [5, 3]);
			waitForReport(this.win);
			calcHelper.assertAddressAfterIdle(this.win, 'D2:F4');
			cy.wrap(null).should(function() {
				expect(lastSetCellCursor(this.win)).to.contain('col=5 row=3 anchorcol=3 anchorrow=1');
			}.bind(this));
		}.bind(this));
	});

	it('deletes the cells selected by a drag', function() {
		// type something in A1, A2 and A3.
		calcHelper.enterCellAddressAndConfirm(this.win, 'A1');
		helper.typeIntoDocument('a{enter}b{enter}c{enter}');
		helper.processToIdle(this.win);

        // Select A1:A2 with mouse and hit DEL.
		withCursorOnA1(this.win, function(cursorRect) {
			selectRangeWithMouse(cursorRect, [0, 0], [0, 1]);
			helper.typeIntoDocument('{del}');
		});
		waitForReport(this.win);

		calcHelper.enterCellAddressAndConfirm(this.win, 'A1');
		cy.cGet('#sc_input_window.formulabar .ui-custom-textarea-text-layer').should('have.text', '');
		calcHelper.enterCellAddressAndConfirm(this.win, 'A2');
		cy.cGet('#sc_input_window.formulabar .ui-custom-textarea-text-layer').should('have.text', '');
		calcHelper.enterCellAddressAndConfirm(this.win, 'A3');
		cy.cGet('#sc_input_window.formulabar .ui-custom-textarea-text-layer').should('have.text', 'c');
	});

	it('adds a reference to a formula with a click on a cell', function() {
		withCursorOnA1(this.win, function(cursorRect) {
			var c3 = cellCenter(cursorRect, 2, 2); // position of cell C3

			// Start a formula in A1 by typing '=", and click on C3 to
			// reference C3.
			helper.typeIntoDocument('=');
			helper.processToIdle(this.win);
			cy.cGet('body').realClick({ x: c3.x, y: c3.y });
			helper.waitForTimers(this.win, 'clicktimer');
			helper.processToIdle(this.win);
			helper.typeIntoDocument('{enter}');
			helper.processToIdle(this.win);
		}.bind(this));

		// A1 should now contain '=C3'
		calcHelper.enterCellAddressAndConfirm(this.win, 'A1');
		cy.cGet('#sc_input_window.formulabar .ui-custom-textarea-text-layer').should('have.text', '=C3');
	});

	it('fills cells with a drag of the fill handle', function() {
		// Type 1 in A1, and move the cursor back to A1 on the client.
		calcHelper.enterCellAddressAndConfirm(this.win, 'A1');
		helper.typeIntoDocument('1{enter}');
		helper.processToIdle(this.win);
		helper.typeIntoDocument('{upArrow}');
		waitForReport(this.win);
		calcHelper.assertAddressAfterIdle(this.win, 'A1');

		// A pending click timer would drop the mouse moves of the drag.
		helper.waitForTimers(this.win, 'clicktimer');

		// Drag the fill handle from A1 two cells down.
		cy.cGet('[id="test-div-auto fill marker"]').then(function(handles) {
			var handleRect = handles[0].getBoundingClientRect();
			var x = handleRect.left + handleRect.width / 2;
			var startY = handleRect.top + handleRect.height / 2;
			var cellHeight = this.win.app.calc.cellCursorRectangle.pHeight / this.win.app.dpiScale;
			var endY = startY + Math.round(cellHeight * 2);

			cy.cGet('body').realMouseDown({ x: x, y: startY });
			cy.cGet('body').realMouseMove(x, endY);
			cy.cGet('body').realMouseUp({ x: x, y: endY });
		}.bind(this));
		helper.processToIdle(this.win);

		// Close the drop-down of fill options that the engine opens after
		// the drag.
		cy.cGet('#jsd-context-menu-dropdown-overlay').click();
		cy.cGet('#jsd-context-menu-dropdown-overlay').should('not.exist');

		// A2 and A3 should continue the series.
		calcHelper.enterCellAddressAndConfirm(this.win, 'A2');
		cy.cGet('#sc_input_window.formulabar .ui-custom-textarea-text-layer').should('have.text', '2');
		calcHelper.enterCellAddressAndConfirm(this.win, 'A3');
		cy.cGet('#sc_input_window.formulabar .ui-custom-textarea-text-layer').should('have.text', '3');

		// The engine should not have sent the area of the fill handle.
		cy.wrap(null).should(function() {
			expect(receivedMessages(this.win, 'cellautofillarea:')).to.have.lengthOf(0);
		}.bind(this));
	});

	it('keeps the cell cursor sliding when its cell is reported again', function() {
		calcHelper.enterCellAddressAndConfirm(this.win, 'A1');

		cy.then(function() {
			var docLayer = this.win.app.map._docLayer;
			var section = docLayer._cellCursorSection;
			var sectionClass = section.constructor;
			var slideTime = sectionClass.slideTime;

			// Make the slide long enough to still run at the checks below.
			sectionClass.slideTime = 60000; // 60 seconds

			// Move to C3; the cursor should be drawn on its way there.
			docLayer.setCellCursor({ col: 2, row: 2 });
			var target = this.win.app.calc.cellCursorRectangle;
			expect(section.getDrawnRectangle().x1).to.be.lessThan(target.x1);

			// Set the cursor on C3 again, as a second report of the same
			// move from the engine does; the slide should continue.
			docLayer.setCellCursor({ col: 2, row: 2 });
			expect(section.getDrawnRectangle().x1).to.be.lessThan(target.x1);

			// End the slide; the cursor should be drawn on C3.  Hiding the
			// cursor first prevents the cursor from sliding.
			sectionClass.slideTime = slideTime;
			docLayer.setCellCursor(null); // hide the cursor
			docLayer.setCellCursor({ col: 2, row: 2 });
			expect(section.getDrawnRectangle().x1).to.equal(target.x1);
		}.bind(this));
	});
});
