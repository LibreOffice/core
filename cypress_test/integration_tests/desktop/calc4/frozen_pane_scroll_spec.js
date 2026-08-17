/* global describe it cy require beforeEach expect */

var helper = require('../../common/helper');
var calcHelper = require('../../common/calc_helper');

describe(['tagdesktop'], 'Calc scrolling with a frozen row', function() {

	beforeEach(function() {
		helper.setupAndLoadDocument('calc/empty-selections.ods');
		cy.getFrameWindow().then((win) => {
			this.win = win;
		});
	});

	it('Ctrl+Home resets the scrollable pane below the frozen row', function() {
		const win = this.win;

		// Give the sheet enough rows that Ctrl+End has to scroll the view.
		calcHelper.enterCellAddressAndConfirm(win, 'A300');
		helper.typeIntoDocument('last used row{enter}');

		// Freeze row 1 as a header, with the view still at the top.
		calcHelper.enterCellAddressAndConfirm(win, 'A2');
		cy.then(function() {
			win.app.map.sendUnoCommand('.uno:FreezePanesRow');
		});
		cy.getFrameWindow().should(function(w) {
			expect(w.app.calc.splitCoordinate.y, 'frozen row split').to.be.greaterThan(0);
		});
		cy.getFrameWindow().then(function(w) { return helper.processToIdle(w); });

		// Whatever residual scroll navigating to A2 left in place before the
		// freeze, Ctrl+End must move well past it for the rest of this test
		// to actually exercise scrolling back.
		let initialY;
		cy.getFrameWindow().then(function(w) {
			initialY = w.app.activeDocument.activeLayout.viewedRectangle.pY1;
		});

		helper.typeIntoDocument('{ctrl}{end}');
		cy.getFrameWindow().then(function(w) { return helper.processToIdle(w); });
		cy.cGet(helper.addressInputSelector).invoke('val').should('contain', '300');

		// Ctrl+End should have actually scrolled the pane down, or the rest
		// of this test would pass without the fix in place.
		cy.getFrameWindow().should(function(w) {
			expect(w.app.activeDocument.activeLayout.viewedRectangle.pY1, 'scrolled down')
				.to.be.greaterThan(initialY + 200);
		});

		helper.typeIntoDocument('{ctrl}{home}');
		cy.getFrameWindow().then(function(w) { return helper.processToIdle(w); });
		cy.cGet(helper.addressInputSelector).should('have.value', 'A1');

		// Without the fix, the pane below the frozen row stays scrolled to
		// where Ctrl+End left it, instead of returning to the top with the
		// cursor.
		cy.getFrameWindow().should(function(w) {
			expect(w.app.activeDocument.activeLayout.viewedRectangle.pY1, 'scroll after Ctrl+Home').to.equal(0);
		});
	});
});
