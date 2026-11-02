/* global describe expect it cy before require */

const helper = require('../../common/helper');
const a11yHelper = require('../../common/a11y_helper');

describe(['tagdesktop'], 'Writer table given to the reader', { testIsolation: false }, function () {
	let win;

	const ROWS = [
		['Name', 'Colour', 'Price'],
		['Apples', 'Red', '1.20'],
		['Green', '1.10'],
		['Lemons', 'Yellow'],
		['Pears', 'Small Large 0.80 1.40', '0.90'],
	];
	const SIZES = [['Small', 'Large'], ['0.80', '1.40']];

	function move(keys) {
		helper.typeIntoDocument(keys);
		cy.then(function () {
			return a11yHelper.processToIdleWithA11yContext(win);
		});
	}

	function editing(text) {
		cy.cGet('#readable-content').should('contain.text', text);
	}

	// queryAXTree is in document order: a cell follows the row it belongs to.
	function tableRows() {
		return a11yHelper.getAXNodesWithin('#a11y-context-table').then(function (nodes) {
			const tables = [];
			nodes.forEach(function (node) {
				if (node.ignored)
					return;
				if (node.role === 'table')
					tables.push([]);
				else if (node.role === 'row' && tables.length)
					tables[tables.length - 1].push([]);
				else if (node.role === 'cell' && tables.length) {
					const rows = tables[tables.length - 1];
					if (rows.length)
						rows[rows.length - 1].push(node.name.replace(/\s+/g, ' ').trim());
				}
			});
			return tables;
		});
	}

	function regionText(id) {
		return cy.cGet('#' + id).invoke('text');
	}

	function cellsOf(rows) {
		return [].concat.apply([], rows);
	}

	before(function () {
		helper.setupAndLoadDocument('writer/table_context.fodt');

		cy.getFrameWindow().then(function (frameWindow) {
			win = frameWindow;
		});

		cy.then(function () {
			win.app.map.setAccessibilityState(true);
			return a11yHelper.processToIdleWithA11yContext(win);
		});

		move('{ctrl}{home}');
		editing('Paragraph above.');
	});

	it('outside a table the reader is given none', function () {
		cy.cGet('#a11y-context-table').should('be.empty');
	});

	it('the table being edited reaches the reader', function () {
		move('{downarrow}');
		editing('Name');
		cy.cGet('#a11y-context-table').should('contain.text', '0.90');

		cy.then(function () {
			return tableRows().then(function (tables) {
				expect(tables, 'one table').to.have.length(1);
				expect(tables[0], 'its rows, each with the cells starting in it').to.deep.equal(ROWS);
			});
		});
	});

	it('a merged cell spans its rows or columns', function () {
		cy.cGet('#a11y-context-table td').contains('Apples').should('have.prop', 'rowSpan', 2);
		cy.cGet('#a11y-context-table td').contains('Yellow').should('have.prop', 'colSpan', 2);
	});

	it('the cells are given once, in the table and not around it', function () {
		move('{downarrow}');
		editing('Apples');

		regionText('a11y-context-before').then(function (text) {
			expect(text, 'the paragraphs before').to.contain('Paragraph above.');
			cellsOf(ROWS).forEach(function (cell) {
				expect(text, 'the paragraphs before').to.not.contain(cell.split(' ')[0]);
			});
		});
		regionText('a11y-context-after').then(function (text) {
			expect(text, 'the paragraphs after').to.contain('Paragraph below.');
			cellsOf(ROWS).forEach(function (cell) {
				expect(text, 'the paragraphs after').to.not.contain(cell.split(' ')[0]);
			});
		});

		// a cell covered by a merged one has an empty paragraph of its own
		cy.then(function () {
			const paragraphs = function (id) {
				return Array.from(win.document.querySelectorAll('#' + id + ' > span')).map(function (span) {
					return span.textContent;
				});
			};
			expect(paragraphs('a11y-context-before'), 'the paragraphs before').to.deep.equal(['Paragraph above.']);
			expect(paragraphs('a11y-context-after'), 'the paragraphs after').to.deep.equal(['Paragraph below.']);
		});

		cy.then(function () {
			const text = win.app.map._textInput.getPlainTextContent();
			expect(text, 'the editable holds the cell being edited').to.contain('Apples');
			expect(text, 'the editable is free of the other cells').to.not.contain('Red');
		});
	});

	function toFirstCell() {
		move('{ctrl}{home}{ctrl}{home}{downarrow}');
		editing(ROWS[0][0]);
	}

	function onWindows(windows) {
		cy.then(function () {
			win.L.Browser.win = windows;
		});
		toFirstCell();
	}

	function cell(text) {
		return Array.from(win.document.querySelectorAll('#a11y-context-table td')).find(function (td) {
			return td.textContent === text;
		});
	}

	function backInTheEditable(text) {
		cy.then(function () {
			expect(win.app.map._textInput.getPlainTextContent(), 'the editable holds the cell').to.contain(text);
			return a11yHelper.getFocusedAXNode().then(function (node) {
				expect(node, 'something holds the focus').to.not.equal(null);
				expect(node.properties.editable, 'the focus is back in the editable').to.not.equal(undefined);
			});
		});
	}

	// Readers on Windows focus the object under their cursor on leaving browse mode.
	it('a cell can take the focus only on Windows', function () {
		let platform;
		cy.then(function () {
			platform = win.L.Browser.win;
		});

		[false, true].forEach(function (windows) {
			onWindows(windows);
			cy.then(function () {
				return a11yHelper.getAXNodesWithin('#a11y-context-table').then(function (nodes) {
					const cells = nodes.filter(function (node) {
						return !node.ignored && node.role === 'cell';
					});
					expect(cells, 'the cells').to.have.length(cellsOf(ROWS).length);
					cells.forEach(function (node, index) {
						expect(node.properties.focusable === true, 'cell ' + index + ' focusable, on Windows: ' + windows)
							.to.equal(windows);
					});
				});
			});
		});

		cy.then(function () {
			win.L.Browser.win = platform;
		});
	});

	it('focusing a cell puts the caret on it', function () {
		let platform;
		cy.then(function () {
			platform = win.L.Browser.win;
		});
		onWindows(true);

		cy.then(function () {
			cell(ROWS[3][1]).focus();
		});
		cy.then(function () {
			return a11yHelper.processToIdleWithA11yContext(win);
		});
		editing(ROWS[3][1]);
		backInTheEditable(ROWS[3][1]);

		cy.then(function () {
			win.L.Browser.win = platform;
		});
	});

	// Orca enters focus mode with the focus on the page and the selection where it was reading.
	function keyFromThePage(from, key, keyCode) {
		cy.then(function () {
			const element = cell(from);
			win.document.activeElement.blur();
			win.getSelection().collapse(element.firstChild, 1);
			expect(win.document.activeElement, 'the focus is on the page').to.equal(win.document.body);
			win.document.body.dispatchEvent(new win.KeyboardEvent('keydown',
				{ key: key, code: key, keyCode: keyCode, bubbles: true, cancelable: true }));
		});
		cy.then(function () {
			return a11yHelper.processToIdleWithA11yContext(win);
		});
	}

	it('a key reaching the page from a cell moves the caret to it', function () {
		toFirstCell();
		keyFromThePage(ROWS[2][1], 'ArrowRight', 39);
		editing(ROWS[2][1]);
		backInTheEditable(ROWS[2][1]);
	});

	[
		{ from: ROWS[1][1], key: 'ArrowDown', keyCode: 40, to: ROWS[2][0] },
		{ from: ROWS[2][0], key: 'ArrowUp', keyCode: 38, to: ROWS[1][1] },
		{ from: ROWS[1][0], key: 'ArrowDown', keyCode: 40, to: ROWS[3][0] },
		{ from: ROWS[3][0], key: 'ArrowUp', keyCode: 38, to: ROWS[1][0] },
		{ from: ROWS[0][0], key: 'ArrowUp', keyCode: 38, to: 'Paragraph above.' },
		{ from: ROWS[4][2], key: 'ArrowDown', keyCode: 40, to: 'Paragraph below.' },
	].forEach(function (step) {
		it('a key reaching the page from ' + step.from + ' moves the caret to ' + step.to, function () {
			toFirstCell();
			keyFromThePage(step.from, step.key, step.keyCode);
			editing(step.to);
			backInTheEditable(step.to);
		});
	});

	it('Down from the paragraph above the table moves the caret to its first cell', function () {
		toFirstCell();
		move('{downarrow}');
		editing(ROWS[1][0]);
		cy.then(function () {
			const above = Array.from(win.document.querySelectorAll('#a11y-context-before > span')).pop();
			win.document.activeElement.blur();
			win.getSelection().collapse(above.firstChild, 1);
			win.document.body.dispatchEvent(new win.KeyboardEvent('keydown',
				{ key: 'ArrowDown', code: 'ArrowDown', keyCode: 40, bubbles: true, cancelable: true }));
		});
		cy.then(function () {
			return a11yHelper.processToIdleWithA11yContext(win);
		});
		editing(ROWS[0][0]);
		backInTheEditable(ROWS[0][0]);
	});

	it('inside a nested table the reader is given that table', function () {
		toFirstCell();
		move('{downarrow}');
		editing(ROWS[1][0]);
		move('{downarrow}{downarrow}');
		editing('Pears');
		move('{end}{rightarrow}');
		editing('Small');
		cy.cGet('#a11y-context-table').should('not.contain.text', 'Apples');

		cy.then(function () {
			return tableRows().then(function (tables) {
				expect(tables, 'one table').to.have.length(1);
				expect(tables[0], 'the rows of the nested table').to.deep.equal(SIZES);
			});
		});
	});

	function rowsWithFirstCell(text) {
		const rows = ROWS.map(function (row) {
			return row.slice();
		});
		rows[0][0] = text;
		return rows;
	}

	function expectRows(rows) {
		cy.cGet('#a11y-context-table').should('contain.text', rows[0][0].split(' ')[0]);
		cy.then(function () {
			return tableRows().then(function (tables) {
				expect(tables, 'one table').to.have.length(1);
				expect(tables[0], 'its rows').to.deep.equal(rows);
			});
		});
	}

	it('typing in a cell changes that cell, and only it, for the reader', function () {
		toFirstCell();
		move('X');
		editing('X' + ROWS[0][0]);
		expectRows(rowsWithFirstCell('X' + ROWS[0][0]));

		move('{enter}');
		editing(ROWS[0][0]);
		expectRows(rowsWithFirstCell('X ' + ROWS[0][0]));

		move('{backspace}{backspace}');
		editing(ROWS[0][0]);
		expectRows(ROWS);
	});

	it('leaving the table takes it away from the reader', function () {
		move('{ctrl}{end}{ctrl}{end}{ctrl}{end}');
		editing('Paragraph below.');
		cy.cGet('#a11y-context-table').should('be.empty');
		cy.cGet('#a11y-context-before').should('contain.text', 'Pears');
	});
});
