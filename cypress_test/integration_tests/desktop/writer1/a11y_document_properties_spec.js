/* global describe it cy before after expect require */

const helper = require('../../common/helper');
const desktopHelper = require('../../common/desktop_helper');
const a11yHelper = require('../../common/a11y_helper');

describe(['tagdesktop'], 'Accessibility of the document properties', { testIsolation: false }, function () {
	let win;
	let rows;

	before(function () {
		helper.setupAndLoadDocument('writer/file_properties.odt');
		desktopHelper.switchUIToNotebookbar();

		cy.getFrameWindow().then(function (frameWindow) {
			win = frameWindow;
		});
		cy.cGet('.notebookbar-tabs-container').should('be.visible');

		cy.then(function () {
			win.app.map.sendUnoCommand('.uno:SetDocumentProperties');
			return helper.processToIdle(win);
		});

		cy.cGet('#DocumentInfoPage #showtype').should('be.visible');

		a11yHelper.getActiveDialog(1).then(function ($dialog) {
			const page = $dialog[0].querySelector('#DocumentInfoPage');
			expect(page, 'the General page').to.not.be.null;

			const cells = Array.from(page.querySelectorAll('.ui-grid-cell')).filter(
				function (cell) { return win.getComputedStyle(cell).visibility !== 'hidden'; }
			);

			rows = [];
			cells.forEach(function (cell, at) {
				const text = cell.textContent.trim();
				const next = cells[at + 1];
				if (!next || !/:$/.test(text)) return;
				if (/:$/.test(next.textContent.trim())) return;
				rows.push({ label: text, valueId: next.id, value: shownValue(next) });
			});

			expect(rows.filter(isBlank), 'properties left blank').to.not.be.empty;
			expect(rows.filter(function (row) { return !isBlank(row); }),
				'properties carrying a value').to.not.be.empty;
		});
	});

	after(function () {
		a11yHelper.closeActiveDialog(1);
	});

	function shownValue(cell) {
		const input = cell.querySelector('input');
		return (input ? input.value : cell.textContent).trim();
	}

	function isBlank(row) {
		return row.value === '';
	}

	function namesWithin(id) {
		return a11yHelper.getAXNodesWithin('#' + id).then(function (nodes) {
			return nodes
				.filter(function (node) { return !node.ignored; })
				.map(function (node) { return node.name.trim(); })
				.filter(function (name) { return name !== ''; });
		});
	}

	it('A property left blank is not announced with its label', function () {
		if (!a11yHelper.axTreeAvailable()) {
			this._runnable.title += ' (skipped: needs a chromium browser)';
			this.skip();
		}

		cy.then(function () {
			rows.filter(isBlank).forEach(function (row) {
				namesWithin(row.valueId).then(function (names) {
					expect(names, '#' + row.valueId + ' holds no value, so it must not repeat ' + row.label)
						.to.not.include(row.label);
				});
			});
		});
	});

	it('The button that opens a dropdown says which list it opens', function () {
		if (!a11yHelper.axTreeAvailable()) {
			this._runnable.title += ' (skipped: needs a chromium browser)';
			this.skip();
		}

		let comboboxes;

		a11yHelper.getActiveDialog(1).then(function ($dialog) {
			const page = $dialog[0].querySelector('#DocumentInfoPage');
			comboboxes = Array.from(page.querySelectorAll('.ui-combobox[id]'))
				.filter(function (box) {
					return win.getComputedStyle(box).visibility !== 'hidden';
				})
				.map(function (box) { return box.id; });

			expect(comboboxes, 'comboboxes on the General page').to.not.be.empty;
		});

		cy.then(function () {
			comboboxes.forEach(function (id) {
				a11yHelper.assertDropdownButtonNamesItsList(id);
			});
		});
	});

	it('A property carrying a value announces the value it shows', function () {
		if (!a11yHelper.axTreeAvailable()) {
			this._runnable.title += ' (skipped: needs a chromium browser)';
			this.skip();
		}

		cy.then(function () {
			rows.filter(function (row) { return !isBlank(row); }).forEach(function (row) {
				namesWithin(row.valueId).then(function (names) {
					expect(names, '#' + row.valueId + ' announces what it shows')
						.to.include(row.value);
				});
			});
		});
	});
});
