/* global describe it cy beforeEach require expect */

var helper = require('../../common/helper');
var calcHelper = require('../../common/calc_helper');

describe(['tagdesktop', 'tagnextcloud', 'tagproxy'], 'Scroll while extending a whole-row selection', function() {

	beforeEach(function() {
		helper.setupAndLoadDocument('calc/autofill.fods');
		cy.getFrameWindow().then(function(win) {
			this.win = win;
		}.bind(this));

		// Work on a fresh empty sheet.
		calcHelper.selectOptionFromContextMenu('Insert sheet before this');
		cy.getFrameWindow().then(function(win) {
			calcHelper.assertAddressAfterIdle(win, 'A1');
		});
	});

	it('The view follows the last row of the selection when Shift+Down adds rows', function() {
		var win = this.win;

		// Click the row header of a row near the bottom of the view.
		cy.cGet('[id="test-div-row header"]')
			.then(function(items) {
				expect(items).to.have.lengthOf(1);
				var bounds = items[0].getBoundingClientRect();
				cy.cGet('body').click((bounds.left + bounds.right) / 2, bounds.bottom - 30);
			});
		helper.processToIdle(win);

		// A single whole row is selected and the sheet is still shown from its top.
		cy.cGet(helper.addressInputSelector).should('have.prop', 'value').and('match', /^A(\d+):[A-Z]+\1$/);
		cy.getFrameWindow().should(function(frameWindow) {
			expect(frameWindow.app.map.wholeRowSelected, 'whole row selected').to.be.true;
			expect(frameWindow.app.activeDocument.activeLayout.viewedRectangle.pY1, 'view top')
				.to.equal(0);
		});

		helper.assertFocus('className', 'clipboard');
		for (var i = 0; i < 15; ++i)
			helper.typeIntoDocument('{shift}{downArrow}');
		helper.processToIdle(win);

		// The selection now ends below the area that was first shown. The view scrolls down
		// so that its last row is in sight, and it does not scroll to the right.
		cy.cGet(helper.addressInputSelector).should('have.prop', 'value').and('match', /^A(\d+):[A-Z]+(\d+)$/);
		cy.getFrameWindow().should(function(frameWindow) {
			var selection = frameWindow.app.map._docLayer._cellSelectionArea;
			var viewed = frameWindow.app.activeDocument.activeLayout.viewedRectangle;
			expect(viewed.pY1, 'view top').to.be.greaterThan(0);
			expect(selection.pY2, 'selection bottom').to.be.at.most(viewed.pY2);
			expect(selection.pY2, 'selection bottom').to.be.greaterThan(viewed.pY1);
			expect(viewed.pX1, 'view left').to.equal(0);
		});
	});
});
