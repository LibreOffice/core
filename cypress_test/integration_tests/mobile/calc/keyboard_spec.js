/* global describe it cy beforeEach require */

var helper = require('../../common/helper');
var calcHelper = require('../../common/calc_helper');
var mobileHelper = require('../../common/mobile_helper');

describe(['tagmobile', 'tagproxy'], 'On-screen keyboard only on request.', function() {

	beforeEach(function() {
		helper.setupAndLoadDocument('calc/bottom_toolbar.ods');
		mobileHelper.enableEditingMobile();
		calcHelper.dblClickOnFirstCell();
	});

	it('Double tap edits the cell with the keyboard.', function() {
		helper.assertHaveKeyboardInput();
	});

	it('Zoom while editing a cell does not raise the keyboard.', function() {
		cy.getFrameWindow().then(function(win) {
			win.app.map.setAcceptInput(false);
			win.app.map.fire('zoomstart');
			win.app.map.fire('zoomend');
			helper.processToIdle(win);
		});

		helper.assertNoKeyboardInput();
		helper.assertFocus('className', 'clipboard');
	});
});
