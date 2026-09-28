/* global describe expect it cy before require */

const helper = require('../../common/helper');
const a11yHelper = require('../../common/a11y_helper');

describe(['tagdesktop'], 'Writer paragraph split across pages', { testIsolation: false }, function () {
	let win;
	let whole;

	const ABOVE = 'Above the split.';
	const BELOW = 'Below the split.';

	function move(keys) {
		helper.typeIntoDocument(keys);
		cy.then(function () {
			return helper.processToIdle(win);
		});
	}

	function editing(text) {
		cy.cGet('#readable-content').should('have.text', text);
	}

	function given(id) {
		return a11yHelper.getAXNodesWithin('#' + id).then(function (nodes) {
			return nodes.filter(function (node) {
				return !node.ignored && node.role === 'StaticText';
			}).map(function (node) { return node.name; });
		});
	}

	before(function () {
		helper.setupAndLoadDocument('writer/split_paragraph_context.fodt');

		cy.getFrameWindow().then(function (frameWindow) {
			win = frameWindow;
		});

		cy.then(function () {
			win.app.map.setAccessibilityState(true);
			return helper.processToIdle(win);
		});

		move('{ctrl}{home}');
		editing(ABOVE);
	});

	it('a paragraph split across pages is given whole after the caret', function () {
		cy.then(function () {
			expect(win.app.map._docLayer._pages, 'the paragraph spreads over pages')
				.to.be.at.least(3);
			return given('a11y-context-after').then(function (after) {
				expect(after, 'the paragraphs after the caret').to.have.length(2);
				expect(after[1], 'the last paragraph').to.equal(BELOW);
				whole = after[0];
			});
		});
	});

	it('and before it', function () {
		move('{ctrl}{end}');
		editing(BELOW);

		cy.then(function () {
			return given('a11y-context-before').then(function (before) {
				expect(before[before.length - 1], 'the paragraph before the caret').to.equal(whole);
			});
		});
	});

	it('the rest of the paragraph the caret is in is given as one', function () {
		move('{ctrl}{home}{downarrow}');

		cy.then(function () {
			const piece = win.app.map._textInput.getPlainTextContent();
			expect(whole.indexOf(piece), 'the editable holds the start of the paragraph').to.equal(0);
			return given('a11y-context-after').then(function (after) {
				expect(after, 'the paragraphs after the caret')
					.to.deep.equal([whole.substring(piece.length), BELOW]);
			});
		});
	});
});
