/* global describe expect it cy before require */

const helper = require('../../common/helper');
const a11yHelper = require('../../common/a11y_helper');

describe(['tagdesktop'], 'Writer comments in the list and table being edited', { testIsolation: false }, function () {
	let win;

	function detailsText(mark) {
		const ids = (mark.getAttribute('aria-details') || '').split(' ').filter(Boolean);
		expect(ids, 'the targets').to.have.length(1);
		const target = win.document.getElementById(ids[0]);
		expect(target, 'the target').to.not.equal(null);
		expect(target.getAttribute('role'), 'the role of the target').to.equal('comment');
		return target.innerText.replace(/\s+/g, ' ').trim();
	}

	function expectComment(selector, details) {
		cy.cGet(selector + ' [role="mark"]').should(function ($marks) {
			expect($marks, 'the marks').to.have.length(1);
			expect($marks.attr('aria-roledescription'), 'the role read').to.equal('Comment');
			expect(detailsText($marks[0]), 'the details').to.equal(details);
		});
		cy.waitUntil(function () {
			return a11yHelper.getAXNodesWithin(selector).then(function (nodes) {
				return nodes.filter(function (node) {
					return !node.ignored && node.role === 'mark';
				}).length === 1;
			});
		});
	}

	function moveTo(keys, text) {
		helper.typeIntoDocument('{ctrl}{home}' + keys);
		cy.then(function () {
			return a11yHelper.processToIdleWithA11yContext(win);
		});
		cy.cGet('#readable-content').should('contain.text', text);
	}

	before(function () {
		helper.setupAndLoadDocument('writer/comment_context.fodt');

		cy.getFrameWindow().then(function (frameWindow) {
			win = frameWindow;
		});

		cy.then(function () {
			win.app.map.setAccessibilityState(true);
			return helper.processToIdle(win);
		});
	});

	it('a comment in another item of the list is marked with its details', function () {
		moveTo('{downarrow}', 'First item');
		expectComment('#a11y-context-list', 'Alice About the second item.');
	});

	it('a comment in another cell of the table is marked with its details', function () {
		moveTo('{downarrow}{downarrow}{downarrow}', 'First cell');
		expectComment('#a11y-context-table', 'Bob About the second cell.');
	});
});
