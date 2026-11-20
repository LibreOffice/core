/* global describe expect it cy before require Cypress */

const helper = require('../../common/helper');
const a11yHelper = require('../../common/a11y_helper');

describe(['tagdesktop'], 'Writer heading in a table cell', { testIsolation: false }, function () {
	let win;

	function move(keys) {
		helper.typeIntoDocument(keys);
		cy.then(function () {
			return a11yHelper.processToIdleWithA11yContext(win);
		});
	}

	function described() {
		return a11yHelper.getAXNodes().then(function (nodes) {
			return nodes.filter(function (node) {
				return !node.ignored && node.description;
			}).map(function (node) { return node.description; }).join(' | ');
		});
	}

	const REGIONS = ['#a11y-headings-above a', '#a11y-context-before > span', '#a11y-context-after > span',
		'#a11y-headings-below a'];

	function headingsInReadingOrder() {
		cy.wrap(null).should(function () {
			const headings = win.app.map._textInput._headings || [];
			const names = headings.map(function (heading) { return heading.text; });
			const expected = headings.filter(function (heading) { return heading.side !== 'caret'; })
				.map(function (heading) { return heading.text; });
			const order = [];
			REGIONS.forEach(function (selector) {
				win.document.querySelectorAll(selector).forEach(function (item) {
					if (names.indexOf(item.textContent) !== -1)
						order.push(item.textContent);
				});
			});
			expect(order.join(' | '), 'the headings a reader moves through, in document order')
				.to.equal(expected.join(' | '));
		});
	}

	before(function () {
		helper.setupAndLoadDocument('writer/heading_in_cell.fodt', false, false, undefined, 'vector=false');

		cy.getFrameWindow().then(function (frameWindow) {
			win = frameWindow;
		});

		cy.then(function () {
			win.app.map.setAccessibilityState(true);
			return a11yHelper.processToIdleWithA11yContext(win);
		});
	});

	it('the level of a heading in a cell is told with the cell and after it', function () {
		// The cell announcement lasts a second, which a slow run spends before it can read the tree.
		const toldOnEntering = [];
		let observer;
		cy.then(function () {
			const editable = win.document.getElementById('clipboard-area');
			observer = new win.MutationObserver(function () {
				toldOnEntering.push(editable.getAttribute('aria-description') || '');
			});
			observer.observe(editable, { attributes: true, attributeFilter: ['aria-description'] });
		});
		move('{ctrl}{home}{downarrow}');
		cy.cGet('#readable-content').should('have.text', 'Heading in a cell');

		cy.then(function () {
			observer.disconnect();
			const told = 'Heading level 2';
			const withCell = toldOnEntering.some(function (text) {
				return text.indexOf('Row 1') !== -1 && text.indexOf(told) !== -1;
			});
			expect(withCell, 'what the reader is told on entering the cell: ' + toldOnEntering.join(' / ')).to.equal(true);
			// Queued after the table announcement's own one second timer, so it runs after it.
			return new Cypress.Promise(function (resolve) { win.setTimeout(resolve, 1000); });
		});
		cy.then(described).then(function (text) {
			expect(text, 'what the reader is told once the cell is announced').to.contain('Heading level 2')
				.and.not.contain('Row 1');
		});
	});

	it('the heading being edited in a cell is read once, in document order', function () {
		headingsInReadingOrder();
	});

	it('a plain cell next to it is not told a level', function () {
		move('{end}{rightarrow}');
		cy.cGet('#readable-content').should('have.text', 'Plain cell');

		cy.then(function () {
			return described().then(function (text) {
				expect(text, 'what the reader is told in the plain cell').to.not.contain('Heading level');
			});
		});
	});

	it('from the next cell the headings are read once, in document order', function () {
		headingsInReadingOrder();
	});

	it('from above the table the heading in a cell is read before the one after it', function () {
		move('{ctrl}{home}{ctrl}{home}{ctrl}{home}');
		cy.cGet('#readable-content').should('have.text', 'Top of the document.');
		headingsInReadingOrder();
	});
});
