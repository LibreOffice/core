/* global describe it cy beforeEach require expect */

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

		var preDisconnectY1;
		cy.getFrameWindow().then(function (win) {
			preDisconnectY1 = win.app.activeDocument.activeLayout.viewedRectangle.y1;
			expect(preDisconnectY1).to.be.greaterThan(0);
		});

		// Close the raw WebSocket to trigger automatic reconnection
		cy.getFrameWindow().then(function (win) {
			win.app.socket.socket.close();
		});

		// Can't use processToIdle with the socket closed
		cy.wait(500);

		cy.cGet('#document-canvas').should('be.visible');
		cy.getFrameWindow().its('app.socket._reconnecting')
			.should('eq', false);
		cy.getFrameWindow().then(function (win) {
			helper.processToIdle(win);
		});

		desktopHelper.assertVisiblePage(12, 12, 12);

		cy.cGet('#document-canvas').click(200, 200);
		cy.getFrameWindow().then(function (win) {
			helper.processToIdle(win);
		});

		desktopHelper.assertVisiblePage(12, 12, 12);
		cy.getFrameWindow().then(function (win) {
			var postClickY1 = win.app.activeDocument.activeLayout.viewedRectangle.y1;
			var drift = Math.abs(postClickY1 - preDisconnectY1);
			expect(drift).to.be.lessThan(preDisconnectY1 / 2);
		});
	});
});
