/* global describe expect it cy before require */

const helper = require('../../common/helper');
const a11yHelper = require('../../common/a11y_helper');

describe(['tagdesktop'], 'Writer spelling errors reach the reader', { testIsolation: false }, function () {
	let win;

	before(function () {
		helper.setupAndLoadDocument('writer/spelling_context.fodt');

		cy.getFrameWindow().then(function (frameWindow) {
			win = frameWindow;
		});

		cy.then(function () {
			win.app.map.setAccessibilityState(true);
			return helper.processToIdle(win);
		});

		helper.typeIntoDocument('{ctrl}{home}');
		cy.then(function () {
			return a11yHelper.processToIdleWithA11yContext(win);
		});
	});

	// Chrome keeps aria-invalid on a span out of the CDP tree; IA2 and AT-SPI get it as a text attribute
	function errorRanges(selector) {
		return a11yHelper.getAXNodesWithin(selector).then(function (nodes) {
			const texts = nodes.filter(function (node) { return node.role === 'StaticText'; })
				.map(function (node) { return node.name; });
			return Array.from(win.document.querySelectorAll(selector + ' [aria-invalid]'))
				.filter(function (span) { return texts.includes(span.textContent); })
				.map(function (span) { return span.getAttribute('aria-invalid') + ': ' + span.textContent; });
		});
	}

	function waitForErrorRanges(selector, check) {
		let last = [];
		return cy.waitUntil(function () {
			return errorRanges(selector).then(function (ranges) {
				last = ranges;
				return check(ranges);
			});
		}, { timeout: 10000, interval: 250, errorMsg: function () {
			return 'error ranges in ' + selector + ': [' + last.join(', ') + ']';
		} });
	}

	it('a misspelled word at the caret is a spelling error', function () {
		waitForErrorRanges('#clipboard-area', function (ranges) {
			return ranges.includes('spelling: Wrongg');
		}).then(function () {
			return errorRanges('#clipboard-area');
		}).then(function (ranges) {
			expect(ranges, 'only the misspelled word').to.deep.equal(['spelling: Wrongg']);
		});
	});

	it('a misspelled word in the paragraph after the caret is a spelling error', function () {
		waitForErrorRanges('#a11y-context-after', function (ranges) {
			return ranges.includes('spelling: secnd');
		});
	});

	it('a misspelled word in another item of the list is a spelling error', function () {
		helper.typeIntoDocument('{ctrl}{home}{downarrow}{downarrow}');
		cy.then(function () {
			return a11yHelper.processToIdleWithA11yContext(win);
		});

		cy.cGet('#readable-content').should('contain.text', 'First item');
		waitForErrorRanges('#a11y-context-list', function (ranges) {
			return ranges.includes('spelling: Secondd');
		});
	});

	it('a misspelled word in another cell of the table is a spelling error', function () {
		helper.typeIntoDocument('{ctrl}{home}{downarrow}{downarrow}{downarrow}{downarrow}');
		cy.then(function () {
			return a11yHelper.processToIdleWithA11yContext(win);
		});

		cy.cGet('#readable-content').should('contain.text', 'First cell');
		waitForErrorRanges('#a11y-context-table', function (ranges) {
			return ranges.includes('spelling: Secondd');
		});
	});

	it('a corrected word is no longer an error', function () {
		helper.typeIntoDocument('{ctrl}{home}');
		helper.typeIntoDocument('{rightarrow}{rightarrow}{rightarrow}{rightarrow}{rightarrow}{del}');
		cy.then(function () {
			return a11yHelper.processToIdleWithA11yContext(win);
		});

		cy.cGet('#readable-content').should('contain.text', 'Wrong is the first word here.');
		waitForErrorRanges('#clipboard-area', function (ranges) {
			return ranges.length === 0;
		});
	});
});
