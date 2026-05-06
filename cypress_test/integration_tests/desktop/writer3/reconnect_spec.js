/* global describe it cy beforeEach require Cypress */

var helper = require('../../common/helper');
var desktopHelper = require('../../common/desktop_helper');

describe(['tagdesktop'], 'WebSocket reconnection', function () {

	beforeEach(function () {
		helper.setupAndLoadDocument('writer/writer-edit.fodt');
	});

	it('Page position is preserved after WebSocket reconnection', function () {
		desktopHelper.assertVisiblePage(1, 1, 12);

		// Navigate to the last page
		helper.typeIntoDocument('{ctrl}{End}');
		desktopHelper.assertVisiblePage(12, 12, 12);

		// Close the raw WebSocket to trigger automatic reconnection
		cy.getFrameWindow().then(function (win) {
			win.app.socket.socket.close();
		});

		// Can't use processToIdle with the socket closed
		cy.wait(500);

		// Wait for reconnection to complete
		cy.cGet('#document-canvas').should('be.visible');
		cy.getFrameWindow().its('app.socket._reconnecting')
			.should('eq', false);
		cy.getFrameWindow().then(function (win) {
			helper.processToIdle(win);
		});

		// Verify the page position is preserved after reconnection
		desktopHelper.assertVisiblePage(12, 12, 12);
	});
});
