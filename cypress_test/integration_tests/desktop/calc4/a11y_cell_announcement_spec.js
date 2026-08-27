/* global describe expect it cy before require */

const helper = require('../../common/helper');
const calcHelper = require('../../common/calc_helper');
const a11yHelper = require('../../common/a11y_helper');

// Moving between cells is announced through the aria-description of the
// clipboard area, which core drives with a11yselectionchanged. The value is
// cleared by a timeout, so every value it takes is recorded as it arrives.
describe(['tagdesktop'], 'Calc cell announcement', { testIsolation: false }, function () {
	let win;
	let announced = [];

	const CONTENT = 'announced cell';

	before(function () {
		helper.setupAndLoadDocument('calc/top_toolbar.ods');

		cy.getFrameWindow().then(function (frameWindow) {
			win = frameWindow;
		});

		cy.then(function () {
			win.app.map.setAccessibilityState(true);
			return helper.processToIdle(win);
		});

		cy.then(function () {
			expect(win.prefs.getBoolean('accessibilityState'),
				'accessibility is on').to.equal(true);

			const area = win.document.getElementById('clipboard-area');
			expect(area, 'the clipboard area').to.not.equal(null);
			new win.MutationObserver(function (records) {
				records.forEach(function (record) {
					if (record.attributeName !== 'aria-description') return;
					const text = record.target.getAttribute('aria-description');
					if (text) announced.push(text);
				});
			}).observe(area, { attributes: true, attributeFilter: ['aria-description'] });
		});
	});

	it('a cell is announced with its address and its content', function () {
		calcHelper.clickOnFirstCell();
		cy.then(function () {
			return helper.processToIdle(win);
		});

		cy.then(function () {
			announced = [];
		});

		helper.typeIntoDocument(CONTENT + '{enter}');
		cy.then(function () {
			return helper.processToIdle(win);
		});

		// coming back to it announces the address and what it holds
		helper.typeIntoDocument('{uparrow}');
		cy.then(function () {
			return helper.processToIdle(win);
		});

		cy.then(function () {
			const forCell = announced.filter(function (text) {
				return text.indexOf('A1') !== -1;
			});
			expect(forCell, 'something was announced for A1').to.not.be.empty;
			expect(forCell.join(' | '), 'the announcement carries the content')
				.to.contain(CONTENT);
		});

		// The attribute is what the announcement is written into; the tree is
		// what the reader is handed, and a hidden area carries neither.
		cy.then(function () {
			return a11yHelper.getAXNodes().then(function (nodes) {
				const said = nodes.filter(function (node) {
					return !node.ignored && node.description &&
						node.description.indexOf(CONTENT) !== -1;
				});
				expect(said, 'the announcement reaches the reader').to.not.be.empty;
			});
		});
	});
});
