/* global describe it cy beforeEach require */

var helper = require('../../common/helper');
var desktopHelper = require('../../common/desktop_helper');

describe(['tagmultiuser'], 'User list dropdown', function() {

	beforeEach(function() {
		helper.setupAndLoadDocument('calc/following.ods', true);
		desktopHelper.switchUIToNotebookbar();
		cy.cSetActiveFrame('#iframe1');
	});

	it('Can reopen the user list after a zoom closes it', function() {
		// open the user list dropdown
		cy.cGet('#userListSummaryButton').click();
		cy.cGet('#userlist-dropdown').should('exist');

		// a zoom while the dropdown is open closes it (e.g. wheel or keyboard
		// zoom, which does not go through the dropdown overlay)
		cy.getFrameWindow().then(function(win) {
			win.app.map.fire('zoomend');
		});
		cy.cGet('#userlist-dropdown').should('not.exist');

		// the dropdown must open again on the next click
		cy.cGet('#userListSummaryButton').click();
		cy.cGet('#userlist-dropdown').should('exist');
	});
});
