/* global describe it cy require beforeEach expect */

const helper = require('../../common/helper');
const desktopHelper = require('../../common/desktop_helper');

describe(['tagdesktop'], 'Iconview keyboard navigation', { testIsolation: false }, function () {
	let win;

	desktopHelper.shareDocumentAcrossTests('writer/navigator.odt');

	beforeEach(function () {
		cy.getFrameWindow().then(function (w) {
			win = w;
		});
		openSymbolDialog();
	});

	function openSymbolDialog() {
		cy.cGet('body').then(function ($body) {
			if ($body.find('#SpecialCharactersDialog').length > 0)
				return;

			cy.then(function () {
				win.app.map.sendUnoCommand('.uno:InsertSymbol');
			});
		});

		cy.cGet('#SpecialCharactersDialog #showcharset .ui-iconview-entry').should('exist');
		cy.then(function () {
			return helper.processToIdle(win);
		});
	}

	function columnCount() {
		return cy.cGet('#SpecialCharactersDialog #showcharset').then(function ($grid) {
			const tracks = win.getComputedStyle($grid[0]).gridTemplateColumns;
			return tracks.split(' ').filter(function (track) {
				return track.length > 0;
			}).length;
		});
	}

	function focusedRow() {
		return cy.cGet('#SpecialCharactersDialog #showcharset').then(function () {
			const active = win.document.activeElement;
			expect(active.id, 'a charset entry has focus').to.match(/^showcharset_\d+$/);
			return Number(active.id.replace('showcharset_', ''));
		});
	}

	function focusRow(row) {
		cy.cGet('#SpecialCharactersDialog #showcharset_' + row).click();
		cy.then(function () {
			return helper.processToIdle(win);
		});
	}

	it('Down moves one row of the charset, not one symbol', function () {
		let columns;

		columnCount().then(function (count) {
			columns = count;
			expect(columns, 'the charset is laid out in a grid').to.be.greaterThan(1);
		});

		cy.then(function () {
			focusRow(columns);
		});

		cy.then(function () {
			cy.realPress('ArrowDown');
		});

		focusedRow().then(function (row) {
			expect(row, 'Down landed one row lower').to.equal(columns * 2);
		});
	});

	it('Up moves one row of the charset, not one symbol', function () {
		let columns;

		columnCount().then(function (count) {
			columns = count;
		});

		cy.then(function () {
			focusRow(columns * 2);
		});

		cy.then(function () {
			cy.realPress('ArrowUp');
		});

		focusedRow().then(function (row) {
			expect(row, 'Up landed one row higher').to.equal(columns);
		});
	});

	it('Right and Left still move one symbol', function () {
		let columns;

		columnCount().then(function (count) {
			columns = count;
		});

		cy.then(function () {
			focusRow(columns);
		});

		cy.then(function () {
			cy.realPress('ArrowRight');
		});

		focusedRow().then(function (row) {
			expect(row, 'Right landed on the next symbol').to.equal(columns + 1);
		});

		cy.then(function () {
			cy.realPress('ArrowLeft');
		});

		focusedRow().then(function (row) {
			expect(row, 'Left landed back on the previous symbol').to.equal(columns);
		});
	});
});
