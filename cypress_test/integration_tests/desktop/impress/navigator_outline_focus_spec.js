/* global describe it cy beforeEach expect require */

var helper = require('../../common/helper');

describe(['tagdesktop'], 'Navigator outline focus', function() {

	beforeEach(function() {
		helper.setupAndLoadDocument('impress/navigator.odp');
		cy.getFrameWindow().then(function(win) {
			this.win = win;
		}.bind(this));
	});

	it('clicking a shape entry keeps focus on it instead of the first slide', function() {
		cy.cGet('#tab-navigator').click();
		cy.cGet('#NavigatorPanel #tree').contains('.ui-treeview-cell-text', 'Slide 2').should('be.visible');
		helper.processToIdle(this.win);

		cy.cGet('#NavigatorPanel #tree').contains('.ui-treeview-cell-text', 'Slide 2').click();
		helper.processToIdle(this.win);
		cy.cGet('#SlideStatus').should('have.text', 'Slide 2 of 4');
		helper.waitForTimers(this.win, 'clicktimer');

		// A shape on another slide: core moves to that slide and sends the tree
		// again with the shape selected below its slide.
		cy.cGet('#NavigatorPanel #tree').contains('.ui-treeview-cell-text', 'Object 2').click();
		helper.processToIdle(this.win);
		cy.cGet('#SlideStatus').should('have.text', 'Slide 4 of 4');

		cy.getFrameWindow().should(function(win) {
			var tabStops = win.document.querySelectorAll('#NavigatorPanel #tree .ui-treeview-entry[tabindex="0"]');
			expect(tabStops).to.have.length(1);
			expect(tabStops[0].textContent).to.contain('Object 2');
			expect(win.document.activeElement).to.equal(tabStops[0]);
		});
	});
});
