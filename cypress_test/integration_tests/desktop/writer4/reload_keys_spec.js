/* global describe it cy beforeEach require expect */

const helper = require('../../common/helper');

// F5, Shift+F5 and Ctrl+R reload the page in a browser. In the document they
// are shortcuts, so they must do that instead.
describe(['tagdesktop'], 'Writer keys that reload the browser', function() {

	beforeEach(function() {
		helper.setupAndLoadDocument('writer/notebookbar.odt');
		// A reload replaces the window and loses this.
		cy.getFrameWindow().then(function(win) { win.noReloadMarker = true; });
	});

	function assertNotReloaded() {
		cy.getFrameWindow().its('noReloadMarker').should('equal', true);
	}

	it('F5 toggles the navigator', function() {
		cy.getFrameWindow().then(function(win) {
			const wasVisible = win.app.map.navigator.isNavigationPanelVisible();

			cy.realPress('F5');

			cy.getFrameWindow().should(function(w) {
				expect(w.app.map.navigator.isNavigationPanelVisible()).to.equal(!wasVisible);
			});
		});
		assertNotReloaded();
	});

	it('Shift+F5 does not reload', function() {
		cy.realPress(['Shift', 'F5']);
		helper.typeIntoDocument('x');
		helper.waitForMapState('.uno:ModifiedStatus', 'true');
		assertNotReloaded();
	});

	it('Ctrl+R aligns the paragraph right', function() {
		cy.realPress(['Control', 'r']);
		helper.waitForMapState('.uno:RightPara', 'true');
		assertNotReloaded();
	});
});
