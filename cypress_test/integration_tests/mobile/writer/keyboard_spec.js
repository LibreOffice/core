/* global describe it cy beforeEach require */

var helper = require('../../common/helper');
var mobileHelper = require('../../common/mobile_helper');

describe(['tagmobile', 'tagproxy'], 'On-screen keyboard only on request.', function() {

	beforeEach(function() {
		helper.setupAndLoadDocument('writer/focus.odt');
		mobileHelper.enableEditingMobile();
		helper.clickIntoDocument();
	});

	function putKeyboardAway(win) {
		win.app.map.setAcceptInput(false);
		helper.processToIdle(win);
		helper.assertNoKeyboardInput();
	}

	function assertKeyboardStaysAway(win) {
		helper.processToIdle(win);
		helper.assertNoKeyboardInput();
		helper.assertFocus('className', 'clipboard');
		cy.cGet('div.clipboard').should('have.attr', 'inputmode', 'none');
	}

	it('Zoom does not raise the keyboard.', function() {
		cy.getFrameWindow().then(function(win) {
			putKeyboardAway(win);

			win.app.map.fire('zoomstart');
			win.app.map.fire('zoomend');

			assertKeyboardStaysAway(win);
		});

		// A hardware keyboard still types into the document.
		helper.setDummyClipboardForCopy();
		helper.typeIntoDocument('y');
		helper.selectAllText();
		helper.copy();
		helper.expectTextForClipboard('y');
	});

	it('Toolbar button with the keyboard down.', function() {
		helper.typeIntoDocument('x');
		helper.selectAllText();

		cy.getFrameWindow().then(function(win) {
			putKeyboardAway(win);
		});

		cy.cGet('#toolbar-down .unoBold').click();
		cy.cGet('#toolbar-down .unoBold').should('have.class', 'selected');

		// Typing makes the cursor move, which used to raise the keyboard.
		helper.typeIntoDocument('y');

		cy.getFrameWindow().then(assertKeyboardStaysAway);
	});
});
