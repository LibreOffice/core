/* global describe it cy beforeEach require */

var helper = require('../../common/helper');
var calcHelper = require('../../common/calc_helper');
var mobileHelper = require('../../common/mobile_helper');

describe(['tagmobile', 'tagnextcloud'], 'Calc autofill marker on mobile.', function() {

	beforeEach(function() {
		helper.setupAndLoadDocument('calc/formulabar.ods');
		mobileHelper.enableEditingMobile();
		cy.getFrameWindow().then((win) => {
			this.win = win;
		});
	});

	// On mobile the autofill marker is drawn at the bottom-centre of the
	// selected cell, half a cell to the left of the corner where the engine's
	// autofill hit-test lives. Dragging the handle must still fill the cells
	// below, so the shift has to be undone before the drag reaches the engine.
	it('Drag autofill marker fills cells below the source', function() {
		var win = this.win;
		var expectedText = 'long line long line long line';

		calcHelper.clickOnFirstCell();
		cy.cGet('[id="test-div-auto fill marker"]').should('exist');

		// The tap that selected the cell arms a 250ms click timer, and
		// mouse-control drops moves while it is pending. Let it finish so the
		// drag below is not swallowed.
		helper.waitForTimers(win, 'clicktimer');
		helper.processToIdle(win);

		// Press the fill handle, drag two rows down, then release, the same
		// steps a user makes. Pressing first lets the container register the
		// handle as the drag target before the move arrives, so the whole fill
		// path runs. Target body so the coordinates are read as absolute page
		// positions; passing them relative to the canvas would add the canvas
		// offset a second time and land the press below the handle.
		cy.cGet('[id="test-div-auto fill marker"]').then(function($handle) {
			var rectangle = $handle[0].getBoundingClientRect();
			var startX = rectangle.left + rectangle.width / 2;
			var startY = rectangle.top + rectangle.height / 2;
			var cellHeight = win.app.calc.cellCursorRectangle.pHeight / win.app.dpiScale;
			var endY = startY + Math.round(cellHeight * 2);

			cy.cGet('body').realMouseDown({ x: startX, y: startY });
			cy.cGet('body').realMouseMove(startX, endY);
			cy.cGet('body').realMouseUp({ x: startX, y: endY });
		});

		helper.processToIdle(win);

		helper.typeIntoInputField(helper.addressInputSelector, 'A2');
		cy.cGet('#sc_input_window.formulabar .ui-custom-textarea-text-layer')
			.should('have.text', expectedText);

		helper.typeIntoInputField(helper.addressInputSelector, 'A3');
		cy.cGet('#sc_input_window.formulabar .ui-custom-textarea-text-layer')
			.should('have.text', expectedText);
	});
});
