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

	it('inside a nested table the reader is given that table', function () {
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

	it('leaving the table takes it away from the reader', function () {
		move('{ctrl}{end}{ctrl}{end}{ctrl}{end}');
		editing('Paragraph below.');
		cy.cGet('#a11y-context-table').should('be.empty');
		cy.cGet('#a11y-context-before').should('contain.text', 'Pears');
	});
});
