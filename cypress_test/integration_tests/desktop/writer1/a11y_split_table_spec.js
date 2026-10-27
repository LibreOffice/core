/* global describe expect it cy before require */

const helper = require('../../common/helper');
const a11yHelper = require('../../common/a11y_helper');

describe(['tagdesktop'], 'Writer table split across pages', { testIsolation: false }, function () {
	let win;

	// split_table_context.fodt: a repeated heading row, then R1|V1 to R70|V70
	const ROWS = 70;
	const WINDOW = 25;

	function move(keys) {
		helper.typeIntoDocument(keys);
		cy.then(function () {
			return helper.processToIdle(win);
		});
	}

	function editing(text) {
		cy.cGet('#readable-content').should('have.text', text);
	}

	// Crossing the page break does not take one arrow per row, so read where the caret is.
	function toRow(row) {
		// From inside a cell Writer takes Ctrl+Home to the cell's start, then the table's, then the document's.
		move('{ctrl}{home}{home}{home}');
		for (let left = row + 1; left > 0; left -= 10)
			move('{downarrow}'.repeat(Math.min(10, left)));
		cy.cGet('#readable-content').invoke('text').then(function (text) {
			const off = parseInt(text.replace('R', ''), 10) - row;
			if (off > 0)
				move('{uparrow}'.repeat(off));
			else if (off < 0)
				move('{downarrow}'.repeat(-off));
		});
		editing('R' + row);
	}

	function cellNames() {
		return a11yHelper.getAXNodesWithin('#a11y-context-table').then(function (nodes) {
			return nodes.filter(function (node) {
				return !node.ignored && node.role === 'cell';
			}).map(function (node) { return node.name.trim(); });
		});
	}

	function rowsAround(row) {
		const cells = [];
		for (let r = Math.max(0, row - WINDOW); r <= Math.min(ROWS, row + WINDOW); r++)
			cells.push.apply(cells, r === 0 ? ['Name', 'Value'] : ['R' + r, 'V' + r]);
		return cells;
	}

	function givenAround(row) {
		cy.then(function () {
			return cellNames().then(function (names) {
				expect(names.join(' '), 'the rows around R' + row + ', each once, in order')
					.to.equal(rowsAround(row).join(' '));
			});
		});
	}

	// The heading is row 1 of the table, so Rk is row k + 1.
	function toldRowOnMove(keys, row) {
		let told;
		cy.then(function () {
			told = cy.spy(win.app.map._textInput, '_setDescription');
		});
		move(keys);
		editing('R' + row);
		cy.then(function () {
			const said = told.getCalls().map(function (call) { return call.args[0]; }).join(' | ');
			told.restore();
			expect(said, 'what the reader is told on reaching R' + row).to.contain('Row ' + (row + 1) + '.');
			expect(said, 'the same table, not another one').to.not.contain('Table with');
			expect(said, 'still inside the table').to.not.contain('Out of table');
		});
	}

	function keyFromThePage(from, key, keyCode) {
		cy.then(function () {
			const cell = Array.from(win.document.querySelectorAll('#a11y-context-table td')).find(function (td) {
				return td.textContent === from;
			});
			expect(cell, 'the cell ' + from + ' is given').to.not.equal(undefined);
			win.document.activeElement.blur();
			win.getSelection().collapse(cell.firstChild, 1);
			win.document.body.dispatchEvent(new win.KeyboardEvent('keydown',
				{ key: key, code: key, keyCode: keyCode, bubbles: true, cancelable: true }));
		});
		cy.then(function () {
			return a11yHelper.processToIdleWithA11yContext(win);
		});
	}

	before(function () {
		helper.setupAndLoadDocument('writer/split_table_context.fodt');

		cy.getFrameWindow().then(function (frameWindow) {
			win = frameWindow;
		});

		cy.then(function () {
			win.app.map.setAccessibilityState(true);
			expect(win.app.map._docLayer._pages, 'the table spreads over pages').to.be.at.least(2);
			return helper.processToIdle(win);
		});
	});

	it('the rows either side of a page break are given as one table', function () {
		toRow(40);
		givenAround(40);
	});

	it('on a later page the rows are the table\'s, without its repeated heading', function () {
		toRow(60);
		givenAround(60);
	});

	it('Down from the last row given moves the caret to the row after it', function () {
		toRow(10);
		keyFromThePage('V' + (10 + WINDOW), 'ArrowDown', 40);
		editing('V' + (10 + WINDOW + 1));
	});

	it('Up from the first row given moves the caret to the row before it', function () {
		toRow(40);
		keyFromThePage('R' + (40 - WINDOW), 'ArrowUp', 38);
		editing('R' + (40 - WINDOW - 1));
	});

	it('crossing the page break tells the row of the same table', function () {
		toRow(44);
		toldRowOnMove('{downarrow}', 45);
		toldRowOnMove('{downarrow}', 46);
	});
});
