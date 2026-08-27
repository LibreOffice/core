/* global describe expect it cy before require */

const helper = require('../../common/helper');
const a11yHelper = require('../../common/a11y_helper');

// a11ycaretchanged and a11ytextselectionchanged put the caret and the
// selection of the document into the readable content, which is where a screen
// reader reads them from.
describe(['tagdesktop'], 'Writer caret and selection', { testIsolation: false }, function () {
	let win;

	const TYPED = 'abcdefghij';
	const STEPS = 3;

	function readable() {
		return win.document.getElementById('readable-content');
	}

	function selection() {
		return win.getSelection();
	}

	// The selection lives in the readable content whether or not the reader is
	// handed it, so assert the paragraph reaches the tree as well.
	function announced(text) {
		return a11yHelper.getAXNodes().then(function (nodes) {
			return nodes.filter(function (node) {
				return !node.ignored && node.name &&
					node.name.indexOf(text) !== -1;
			});
		});
	}

	before(function () {
		helper.setupAndLoadDocument('writer/copy_paste.odt');

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
		});

		helper.typeIntoDocument('{ctrl}{home}');
		helper.typeIntoDocument(TYPED);
		cy.then(function () {
			return helper.processToIdle(win);
		});
	});

	it('the caret position follows the document', function () {
		helper.typeIntoDocument('{home}');
		cy.then(function () {
			return helper.processToIdle(win);
		});

		cy.then(function () {
			return announced(TYPED).then(function (nodes) {
				expect(nodes, 'the paragraph reaches the reader').to.not.be.empty;
			});
		});

		cy.cGet('#readable-content').should(function () {
			const sel = selection();
			expect(readable().contains(sel.anchorNode),
				'the caret sits in the readable content').to.equal(true);
			expect(sel.isCollapsed, 'nothing is selected').to.equal(true);
			expect(sel.anchorOffset, 'the caret is at the start').to.equal(0);
		});

		helper.typeIntoDocument('{rightarrow}'.repeat(STEPS));
		cy.then(function () {
			return helper.processToIdle(win);
		});

		cy.cGet('#readable-content').should(function () {
			expect(selection().anchorOffset, 'the caret moved by that many')
				.to.equal(STEPS);
		});
	});

	it('a selection reaches the readable content', function () {
		helper.typeIntoDocument('{home}');
		helper.typeIntoDocument('{rightarrow}'.repeat(STEPS));
		cy.then(function () {
			return helper.processToIdle(win);
		});

		helper.typeIntoDocument('{shift}{end}');
		cy.then(function () {
			return helper.processToIdle(win);
		});

		cy.then(function () {
			return announced(TYPED).then(function (nodes) {
				expect(nodes, 'the paragraph reaches the reader').to.not.be.empty;
			});
		});

		cy.cGet('#readable-content').should(function () {
			const sel = selection();
			const paragraph = readable().textContent;

			expect(sel.isCollapsed, 'there is a selection').to.equal(false);
			expect(sel.anchorOffset, 'it starts where the caret was')
				.to.equal(STEPS);
			expect(sel.focusOffset, 'it reaches the end of the paragraph')
				.to.equal(paragraph.length);
			expect(sel.toString(), 'it holds the rest of the paragraph')
				.to.equal(paragraph.slice(STEPS));
		});
	});
});
