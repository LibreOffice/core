/* global describe it cy expect require */

var helper = require('../../common/helper');

describe(['tagdesktop'], 'Navigator slide-sorter focus', function() {
	it('reopening the navigator focuses the current slide so arrows navigate', function() {
		helper.setupAndLoadDocument('impress/navigator.odp');

		// close the navigation panel, which reveals the floating navigator icon
		cy.cGet('.close-navigation-button').click();
		cy.cGet('#navigation-sidebar').should('not.have.class', 'visible');

		// reopen it via the floating icon
		cy.cGet('#floating-navigator').click();
		cy.cGet('#navigation-sidebar').should('be.visible');

		// focus lands on the current slide in the sorter, not back in the document
		cy.getFrameWindow().should(function(win) {
			expect(win.document.activeElement).to.have.class('preview-img-currentpart');
			expect(win.app.map._docLayer._preview.partsFocused).to.be.true;
		});

		// so the arrow keys move to the next slide
		cy.getFrameWindow().then(function(win) {
			win.document.activeElement.dispatchEvent(
				new win.KeyboardEvent('keydown', { key: 'ArrowDown', keyCode: 40, which: 40, bubbles: true }));
		});
		cy.getFrameWindow().should(function(win) {
			expect(win.app.map._docLayer._selectedPart).to.equal(1);
		});
	});
});
