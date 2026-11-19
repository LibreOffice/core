/* global describe it cy beforeEach require */

const helper = require('../../common/helper');

// Shift+F5 reloads the page in a browser. Impress starts the slideshow from
// the current slide on it, as desktop Impress does.
describe(['tagdesktop'], 'Impress keys that reload the browser', function() {

	beforeEach(function() {
		helper.setupAndLoadDocument('impress/slide_navigation.odp');
		// A reload replaces the window and loses this.
		cy.getFrameWindow().then(function(win) {
			win.noReloadMarker = true;
			cy.stub(win.app.dispatcher, 'dispatch').as('dispatch');
		});
	});

	it('Shift+F5 presents from the current slide', function() {
		cy.realPress(['Shift', 'F5']);
		cy.get('@dispatch').should('have.been.calledWith', 'presentation-currentslide');
		cy.getFrameWindow().its('noReloadMarker').should('equal', true);
	});
});
