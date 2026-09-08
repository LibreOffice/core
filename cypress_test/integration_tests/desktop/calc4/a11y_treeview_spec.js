/* global describe expect it cy before require */

const helper = require('../../common/helper');
const a11yHelper = require('../../common/a11y_helper');

// The Navigator is a landmark of the page and a treeview inside it: a named
// tree of rows that carry a level, a selection and, for some, a disclosure.
describe(['tagdesktop'], 'Navigator accessibility', { testIsolation: false }, function () {
	let win;

	before(function () {
		cy.viewport(1920, 1080);
		helper.setupAndLoadDocument('calc/top_toolbar.ods');

		cy.getFrameWindow().then(function (frameWindow) {
			win = frameWindow;
		});

		cy.then(function () {
			win.app.map.sendUnoCommand('.uno:Navigator');
			return helper.processToIdle(win);
		});
		cy.cGet('#contentbox .ui-treeview-entry').should('exist');
	});

	function needsAXTree(test) {
		if (a11yHelper.axTreeAvailable()) return false;

		test._runnable.title += ' (skipped: needs a chromium browser)';
		test.skip();
		return true;
	}

	/// The rows a screen reader can reach: the collapsed ones keep their
	/// children in the DOM, and those are not on screen.
	function shownRows() {
		return Array.from(win.document.querySelectorAll('#contentbox .ui-treeview-entry'))
			.filter(function (row) { return row.offsetParent !== null; });
	}

	function treeNodes() {
		return a11yHelper.getAXNodesWithin('#contentbox').then(function (nodes) {
			return nodes.filter(function (node) { return !node.ignored; });
		});
	}

	it('every navigation landmark carries a name of its own', function () {
		if (needsAXTree(this)) return;

		let expected;

		cy.then(function () {
			expected = Array.from(win.document.querySelectorAll('nav'))
				.filter(function (nav) { return nav.offsetParent !== null; })
				.map(function (nav) { return nav.className || nav.id; });

			expect(expected, 'the page shows more than one nav').to.have.length.above(1);
		});

		cy.then(function () {
			return a11yHelper.getAXNodes().then(function (nodes) {
				const landmarks = nodes.filter(function (node) {
					return node.role === 'navigation' && !node.ignored;
				});

				expect(landmarks.map(function (node) { return node.name; }),
					'a landmark per nav the page shows')
					.to.have.lengthOf(expected.length);

				const names = [];

				landmarks.forEach(function (node) {
					const name = node.name.trim();

					expect(name, 'name of a navigation landmark').to.not.be.empty;
					// The role is announced already, so a name that repeats it
					// makes the screen reader say "navigation" twice.
					expect(name.toLowerCase(), 'name of the ' + name + ' landmark')
						.to.not.contain('navigation');
					expect(names, name + ' is not the name of another landmark')
						.to.not.include(name);

					names.push(name);
				});
			});
		});
	});

	it('the tree names itself', function () {
		if (needsAXTree(this)) return;

		treeNodes().then(function (nodes) {
			const tree = nodes.filter(function (node) { return node.role === 'tree'; })[0];

			expect(tree, 'the container is a tree in the tree').to.not.be.undefined;
			expect(tree.name.trim(), 'accessible name of the tree').to.not.be.empty;
		});
	});

	it('every row is announced by its own text', function () {
		if (needsAXTree(this)) return;

		let expected;

		cy.then(function () {
			expected = shownRows().map(function (row) { return row.textContent.trim(); });
			expect(expected, 'rows on screen').to.not.be.empty;
		});

		cy.then(function () {
			return treeNodes().then(function (nodes) {
				const names = nodes
					.filter(function (node) { return node.role === 'treeitem'; })
					.map(function (node) { return node.name.trim(); });

				expect(names, 'a treeitem per row on screen').to.have.members(expected);
			});
		});
	});

	it('every row announces the level it sits at', function () {
		if (needsAXTree(this)) return;

		let levels;

		cy.then(function () {
			levels = {};
			shownRows().forEach(function (row) {
				levels[row.textContent.trim()] = parseInt(row.getAttribute('aria-level'), 10);
			});
		});

		cy.then(function () {
			return treeNodes().then(function (nodes) {
				nodes.filter(function (node) { return node.role === 'treeitem'; })
					.forEach(function (node) {
						const name = node.name.trim();

						expect(node.properties.level, 'level announced for ' + name)
							.to.equal(levels[name]);
					});
			});
		});
	});

	it('one row is selected, and it is the one the focus starts on', function () {
		if (needsAXTree(this)) return;

		let selected;

		cy.then(function () {
			const marked = shownRows().filter(function (row) {
				return row.getAttribute('aria-selected') === 'true';
			});

			expect(marked, 'rows marked as selected').to.have.lengthOf(1);
			selected = marked[0].textContent.trim();

			expect(marked[0].getAttribute('tabindex'),
				'the selected row is where Tab lands').to.equal('0');
		});

		cy.then(function () {
			return treeNodes().then(function (nodes) {
				const announced = nodes
					.filter(function (node) {
						return node.role === 'treeitem' && node.properties.selected === true;
					})
					.map(function (node) { return node.name.trim(); });

				expect(announced, 'rows announced as selected').to.deep.equal([selected]);
			});
		});
	});

	it('an expandable row announces whether it is expanded, and the arrow that opens it', function () {
		if (needsAXTree(this)) return;

		let name, level;

		cy.then(function () {
			const row = shownRows().filter(function (candidate) {
				return candidate.classList.contains('ui-treeview-expandable') &&
					candidate.getAttribute('aria-expanded') === 'false';
			})[0];

			expect(row, 'a collapsed expandable row').to.not.be.undefined;
			name = row.textContent.trim();
			level = parseInt(row.getAttribute('aria-level'), 10);
			row.focus();
		});

		cy.then(function () {
			return treeNodes().then(function (nodes) {
				const node = nodes.filter(function (candidate) {
					return candidate.role === 'treeitem' && candidate.name.trim() === name;
				})[0];

				expect(node.properties.expanded, name + ' announces itself collapsed')
					.to.equal(false);
			});
		});

		cy.realPress('ArrowRight');
		cy.then(function () { return helper.processToIdle(win); });

		cy.then(function () {
			return treeNodes().then(function (nodes) {
				const items = nodes.filter(function (node) { return node.role === 'treeitem'; });
				const node = items.filter(function (candidate) {
					return candidate.name.trim() === name;
				})[0];

				expect(node.properties.expanded, name + ' announces itself expanded')
					.to.equal(true);

				const children = items.filter(function (candidate) {
					return candidate.properties.level === level + 1;
				});

				expect(children, 'the rows ' + name + ' opened').to.not.be.empty;
			});
		});
	});

	it('the arrows move the focus the tree reports', function () {
		if (needsAXTree(this)) return;

		let next;

		cy.then(function () {
			const rows = shownRows();
			const at = rows.findIndex(function (row) {
				return row.getAttribute('tabindex') === '0';
			});

			expect(at, 'a row carries the focus').to.be.greaterThan(-1);
			expect(rows[at + 1], 'a row below it').to.not.be.undefined;
			next = rows[at + 1].textContent.trim();
			rows[at].focus();
		});

		cy.realPress('ArrowDown');
		cy.then(function () { return helper.processToIdle(win); });

		cy.then(function () {
			return a11yHelper.getFocusedAXNode().then(function (node) {
				expect(node, 'a focused node').to.not.equal(null);
				expect(node.role, 'role of the focused node').to.equal('treeitem');
				expect(node.name.trim(), 'the row the arrow reached').to.equal(next);
			});
		});
	});
});
