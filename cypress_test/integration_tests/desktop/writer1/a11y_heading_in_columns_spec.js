/* global describe expect it cy before require */

const helper = require('../../common/helper');
const a11yHelper = require('../../common/a11y_helper');

describe(['tagdesktop'], 'Writer headings in a page with columns', { testIsolation: false }, function () {
	let win;

	before(function () {
		helper.setupAndLoadDocument('writer/heading_in_columns.fodt', false, false, undefined, 'vector=false');

		cy.getFrameWindow().then(function (frameWindow) {
			win = frameWindow;
		});

		cy.then(function () {
			win.app.map.setAccessibilityState(true);
			return a11yHelper.processToIdleWithA11yContext(win);
		});
	});

	it('a heading at the top of the next column is read after the caret at the bottom of the first', function () {
		cy.then(function () {
			win.app.map.sendUnoCommand('.uno:JumpToMark?Bookmark:string=caret');
			return a11yHelper.processToIdleWithA11yContext(win);
		});
		cy.cGet('#readable-content').should('have.text', 'Last line of the left column.');

		cy.then(function () {
			const right = win.app.map._textInput._headings.find(function (heading) {
				return heading.text === 'Right heading';
			});
			const rect = right.rect.split(',').map(Number);
			expect(rect[1], 'the top of the next column, above the caret on the page')
				.to.be.lessThan(win.app.file.textCursor.rectangle.y1);

			const links = Array.from(win.document.querySelectorAll('#a11y-headings-above a, #a11y-headings-below a'))
				.map(function (link) { return link.textContent; });
			expect(links, 'the headings given as links, outside the paragraphs around the caret')
				.to.include('Right heading');

			const order = [];
			['#a11y-headings-above a', '#a11y-context-before > span', 'caret', '#a11y-context-after > span',
				'#a11y-headings-below a'].forEach(function (selector) {
				if (selector === 'caret') {
					order.push('(caret)');
					return;
				}
				win.document.querySelectorAll(selector).forEach(function (item) {
					if (item.textContent === 'Left heading' || item.textContent === 'Right heading')
						order.push(item.textContent);
				});
			});
			expect(order, 'the headings a reader moves through, in document order')
				.to.deep.equal(['Left heading', '(caret)', 'Right heading']);
		});
	});
});
