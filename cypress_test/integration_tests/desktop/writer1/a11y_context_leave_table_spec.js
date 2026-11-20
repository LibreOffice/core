/* global describe expect it cy before require */

const helper = require('../../common/helper');

describe(['tagdesktop'], 'Writer context around a table', { testIsolation: false }, function () {
	let win;

	function move(keys) {
		helper.typeIntoDocument(keys);
		cy.then(function () {
			return helper.processToIdle(win);
		});
	}

	function region(id) {
		return Array.from(win.document.querySelectorAll('#' + id + ' > span')).map(function (span) {
			return span.textContent;
		});
	}

	before(function () {
		helper.setupAndLoadDocument('writer/context_leave_table.fodt', false, false, undefined, 'vector=false');

		cy.getFrameWindow().then(function (frameWindow) {
			win = frameWindow;
		});

		cy.then(function () {
			win.app.map.setAccessibilityState(true);
			return helper.processToIdle(win);
		});
	});

	it('the paragraphs around the caret follow it out of a table', function () {
		move('{ctrl}{home}{downarrow}{downarrow}');
		cy.cGet('#readable-content').should('have.text', 'First cell');

		// Ctrl+Home inside a table stops at the table's start first.
		move('{ctrl}{home}{ctrl}{home}');
		cy.cGet('#readable-content').should('have.text', 'Top of the document.');
		cy.then(function () {
			expect(region('a11y-context-before'), 'before the first paragraph').to.deep.equal([]);
			expect(region('a11y-context-after'), 'after the first paragraph').to.include('Before the table.');
		});
	});
});
