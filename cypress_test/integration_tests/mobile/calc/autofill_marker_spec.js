/* global describe it cy beforeEach require expect */

var helper = require('../../common/helper');
var calcHelper = require('../../common/calc_helper');
var mobileHelper = require('../../common/mobile_helper');

describe(['tagmobile', 'tagnextcloud'], 'Calc autofill marker on mobile.', function() {

	beforeEach(function() {
		helper.setupAndLoadDocument('calc/formulabar.ods');
		mobileHelper.enableEditingMobile();
	});

	// On mobile the autofill marker is rendered at the bottom-centre of the
	// selected cell instead of the bottom-right corner like on desktop. The
	// mouse events posted to core must still be at the bottom-right corner
	// where core's autofill hit-test lives, otherwise the drag has no effect
	// and no cells get filled.
	it('Drag autofill marker fills cells below the source', function() {
		var expectedText = 'long line long line long line';

		calcHelper.clickOnFirstCell();
		cy.cGet('[id="test-div-auto fill marker"]').should('exist');

		// Cypress synthetic DOM events are guarded against by the canvas
		// section container (mouseIsInside, button-state checks). Invoke
		// the marker's handlers directly so the coordinate computation is
		// exercised against the live core kit.
		cy.getFrameWindow().then(function(win) {
			var marker = win.app.sectionContainer.getSectionWithName('auto fill marker');
			expect(marker, 'autofill marker section').to.exist;

			var halfWidth = Math.floor(marker.size[0] / 2);
			var halfHeight = Math.floor(marker.size[1] / 2);
			var cellHeight = win.app.calc.cellCursorRectangle.pHeight;

			var localStart = win.cool.SimplePoint.fromCorePixels([halfWidth, halfHeight]);
			marker.onMouseDown(localStart, new win.MouseEvent('mousedown', { button: 0 }));

			var localEnd = win.cool.SimplePoint.fromCorePixels([halfWidth, halfHeight + cellHeight * 2]);
			marker.onMouseMove(localEnd, [0, cellHeight * 2], new win.MouseEvent('mousemove'));
			marker.onMouseUp(localEnd, new win.MouseEvent('mouseup', { button: 0 }));
		});

		cy.getFrameWindow().then(function(win) {
			helper.processToIdle(win);
		});

		helper.typeIntoInputField(helper.addressInputSelector, 'A2');
		cy.cGet('#sc_input_window.formulabar .ui-custom-textarea-text-layer')
			.should('have.text', expectedText);

		helper.typeIntoInputField(helper.addressInputSelector, 'A3');
		cy.cGet('#sc_input_window.formulabar .ui-custom-textarea-text-layer')
			.should('have.text', expectedText);
	});
});
