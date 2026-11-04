/* -*- js-indent-level: 8 -*- */
/* global describe it cy beforeEach require expect */

var helper = require('../../common/helper');
var calcHelper = require('../../common/calc_helper');
var desktopHelper = require('../../common/desktop_helper');

// A spreadsheet drawn from vector primitives: the engine serves the cells as primitives in
// blocks, the browser places and draws them, and no tile is asked for or kept.
describe(['tagdesktop'], 'Spreadsheet drawn from vector primitives', function() {
	var win;
	// The text messages sent to the server since the last reset, oldest first.
	var sent;
	var formulaText = '#sc_input_window .ui-custom-textarea-text-layer';

	beforeEach(function() {
		var path = helper.setupDocument('calc/cell_edit.fods');
		helper.loadDocument(path, true, undefined, undefined, 'vector=true');
		// The commands below are queued once the window is known, as a helper given the
		// window from the hook body would take it before it is fetched, or take the window
		// of an earlier attempt.
		cy.getFrameWindow().then(function(w) {
			win = w;
			// The stock load checks wait for tiles that a vector view lacks: the view is
			// loaded once it holds the blocks of the sheet.
			cy.wrap(null).should(function() {
				var manager = w.app.calc.vectorManager;
				expect(manager, 'the vector manager').to.exist;
				var sheet = manager.getSheet(0);
				expect(sheet, 'the sheet held').to.exist;
				expect(sheet.blocks.size).to.be.greaterThan(0);
			});
			helper.processToIdle(w);
			cy.then(function() {
				sent = [];
				var socket = w.app.socket;
				var doSend = socket._doSend;
				socket._doSend = function(message) {
					if (typeof message === 'string')
						sent.push(message);
					return doSend.apply(this, arguments);
				};
			});
			// The fixture opens at its stored cursor, far from the start of the sheet.
			calcHelper.selectCellsInRange('A1');
			calcHelper.assertAddressAfterIdle(w, 'A1');
		});
	});

	function tileRequests() {
		return sent.filter(function(message) {
			return message.indexOf('tilecombine') === 0;
		});
	}

	function manager() {
		return win.app.calc.vectorManager;
	}

	function sheet() {
		return manager().getSheet(0);
	}

	// The tiles the browser holds. A vector view drops them all.
	function tilesHeld() {
		return win.app.tile.count();
	}

	it('asks for no tiles and holds none', function() {
		cy.then(function() { sent.length = 0; });
		helper.typeIntoDocument('{pagedown}');
		helper.processToIdle(win);
		helper.typeIntoDocument('{pageup}');
		helper.processToIdle(win);
		cy.wrap(null).should(function() {
			expect(tileRequests()).to.deep.equal([]);
			expect(tilesHeld()).to.equal(0);
			expect(sheet().blocks.size).to.be.greaterThan(0);
		});
	});

	it('completes a typed text from its column and keeps it after the second character', function() {
		// Every keystroke shows at once in the browser and is answered by the engine. A text
		// completed from the column keeps the same text after the second character, which the
		// engine must still answer, or the browser keeps showing only what it typed.
		calcHelper.selectCellsInRange('E1');
		calcHelper.assertAddressAfterIdle(win, 'E1');
		helper.typeIntoDocument('Teszt asztal v{enter}');
		calcHelper.assertAddressAfterIdle(win, 'E2');
		helper.typeIntoDocument('{downArrow}');
		calcHelper.assertAddressAfterIdle(win, 'E3');

		helper.typeIntoDocument('T');
		helper.processToIdle(win);
		cy.cGet(formulaText).should('have.text', 'Teszt asztal v');

		helper.typeIntoDocument('e');
		helper.processToIdle(win);
		cy.cGet(formulaText).should('have.text', 'Teszt asztal v');
		cy.wrap(null).should(function() {
			// The engine answered the keystrokes, so nothing typed waits to be shown.
			expect(manager()._typing.isTyping()).to.be.false;
		});
		helper.typeIntoDocument('{esc}');
		helper.processToIdle(win);
	});

	it('shows a column at its new width from the pushed content', function() {
		cy.then(function() {
			sent.length = 0;
			win.app.map.sendUnoCommand('.uno:ColumnWidth', {
				ColumnWidth: { type: 'unsigned short', value: 3000 },
				Column: { type: 'long', value: 1 },
			});
		});
		helper.processToIdle(win);
		cy.wrap(null).should(function() {
			// The command takes hundredths of a millimetre: 30 mm is 1701 twips.
			var columns = win.app.map._docLayer.sheetGeometry.getColumnsGeometry();
			expect(columns.getElementDataAny(0, 'printtwips').size).to.equal(1701);
			// The blocks the view shows came built in the push; none waits to be asked for.
			expect(sheet().staleBlocks.size).to.equal(0);
			expect(tileRequests()).to.deep.equal([]);
		});
	});

	it('shows inserted rows from the pushed content', function() {
		calcHelper.selectCellsInRange('E1');
		calcHelper.assertAddressAfterIdle(win, 'E1');
		helper.typeIntoDocument('Moved{enter}');
		calcHelper.assertAddressAfterIdle(win, 'E2');
		calcHelper.selectCellsInRange('A1');
		calcHelper.assertAddressAfterIdle(win, 'A1');
		cy.then(function() {
			sent.length = 0;
			win.app.map.sendUnoCommand('.uno:InsertRowsBefore');
		});
		helper.processToIdle(win);
		cy.wrap(null).should(function() {
			// The shown blocks came rebuilt, so no shift of an older block is kept.
			expect(sheet().shifts).to.deep.equal([]);
			expect(sheet().staleBlocks.size).to.equal(0);
			expect(tileRequests()).to.deep.equal([]);
		});
		calcHelper.selectCellsInRange('E2');
		calcHelper.assertAddressAfterIdle(win, 'E2');
		cy.cGet(formulaText).should('have.text', 'Moved');
	});

	it('zooms from the content it holds', function() {
		cy.then(function() { sent.length = 0; });
		desktopHelper.zoomIn();
		helper.processToIdle(win);
		helper.waitForCanvasAnimation(win);
		cy.wrap(null).should(function() {
			expect(manager()).to.exist;
			expect(sheet().blocks.size).to.be.greaterThan(0);
			expect(tileRequests()).to.deep.equal([]);
			expect(tilesHeld()).to.equal(0);
		});
	});
});
