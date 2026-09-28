/* global describe expect it cy before require */

const helper = require('../../common/helper');
const a11yHelper = require('../../common/a11y_helper');

// What a screen reader reads inside the canvas does not come from the DOM of
// the document: core sends a11yfocuschanged and the client puts the paragraph
// holding the caret into #readable-content.
describe(['tagdesktop'], 'Writer readable content', { testIsolation: false }, function () {
	let win;

	const FIRST = 'First paragraph for the reader';
	const SECOND = 'Second paragraph for the reader';

	// What the reader is handed, not what the DOM holds: the nodes of the
	// tree that carry the text and are not ignored.
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

		// The user preference starts off and persists between runs, so turn it
		// on rather than depend on whatever the last run left behind. Core is
		// only told to send the a11y events once it is on.
		cy.then(function () {
			win.app.map.setAccessibilityState(true);
			return helper.processToIdle(win);
		});

		cy.then(function () {
			expect(win.prefs.getBoolean('accessibilityState'),
				'accessibility is on').to.equal(true);
			expect(typeof win.app.map._textInput.onAccessibilityFocusChanged,
				'the a11y text input is in use').to.equal('function');
		});
	});

	it('the readable content follows the caret between paragraphs', function () {
		helper.typeIntoDocument('{ctrl}{home}');
		helper.typeIntoDocument(FIRST + '{enter}' + SECOND);
		cy.then(function () {
			return helper.processToIdle(win);
		});

		// the caret sits in the second paragraph
		cy.then(function () {
			return announced(SECOND).then(function (nodes) {
				expect(nodes, 'the paragraph being edited').to.not.be.empty;
			});
		});

		// walking up moves it to the first
		helper.typeIntoDocument('{uparrow}');
		cy.then(function () {
			return helper.processToIdle(win);
		});

		cy.then(function () {
			return announced(FIRST).then(function (nodes) {
				expect(nodes, 'the paragraph above').to.not.be.empty;
			});
		});
		// The paragraphs around the view reach the tree as context, so what says
		// the readable content moved is that it no longer carries this one.
		cy.cGet('#readable-content').should(function ($span) {
			expect($span.text(), 'the readable content moved off it').to.not.contain(SECOND);
		});

		// and back down
		helper.typeIntoDocument('{downarrow}');
		cy.then(function () {
			return helper.processToIdle(win);
		});

		cy.then(function () {
			return announced(SECOND).then(function (nodes) {
				expect(nodes, 'the paragraph below').to.not.be.empty;
			});
		});
	});
});
