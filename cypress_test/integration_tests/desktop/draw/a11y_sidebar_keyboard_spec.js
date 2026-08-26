/* global describe before beforeEach it cy require */

var helper = require('../../common/helper');
var desktopHelper = require('../../common/desktop_helper');
var a11yHelper = require('../../common/a11y_helper');

// The sidebar is one stop of the F6 ring, and once the focus is inside it the
// Tab key has to walk the widgets of the open deck. The checks are shared with
// the other modules and read every expectation from the DOM, so each spec
// follows whatever core describes for its own property deck.
describe(['tagdesktop'], 'Draw sidebar keyboard navigation', { testIsolation: false }, function () {
	let win;

	function getWin() {
		return win;
	}

	before(function () {
		cy.viewport(1920, 1080);
		helper.setupAndLoadDocument('draw/to_curve.fodg');

		cy.getFrameWindow().then(function (frameWindow) {
			win = frameWindow;
		});

		desktopHelper.switchUIToNotebookbar();
		cy.cGet('.notebookbar-tabs-container').should('be.visible');

		a11yHelper.openSidebarPropertyDeck(getWin);
	});

	beforeEach(function () {
		helper.typeIntoDocument('{esc}');
		helper.assertFocus('className', 'clipboard');
	});

	it('no widget of the deck forces its own tab position', function () {
		a11yHelper.sidebarKeyboard.assertNoForcedTabPosition(getWin);
	});

	it('the focusable helper skips widgets hidden with visibility', function () {
		a11yHelper.sidebarKeyboard.assertHelperSkipsInvisible(getWin);
	});

	it('the focusable helper reports the tab order of the deck', function () {
		a11yHelper.sidebarKeyboard.assertHelperReportsTabOrder(getWin);
	});

	it('the focusable predicate agrees with the helper', function () {
		a11yHelper.sidebarKeyboard.assertPredicateAgreesWithHelper(getWin);
	});

	it('F6 enters the deck on its first widget', function () {
		a11yHelper.sidebarKeyboard.assertRingEntersOnFirstWidget(getWin);
	});

	it('Tab walks the deck in order and Shift+Tab comes back', function () {
		a11yHelper.sidebarKeyboard.assertTabWalksTheDeck(getWin);
	});

	it('F6 leaves the deck from a widget inside it', function () {
		a11yHelper.sidebarKeyboard.assertRingLeavesTheDeck(getWin);
	});
});
