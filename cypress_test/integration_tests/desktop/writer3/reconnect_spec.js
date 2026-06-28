/* global describe it cy beforeEach require expect */

var helper = require('../../common/helper');
var desktopHelper = require('../../common/desktop_helper');

describe(['tagdesktop'], 'WebSocket reconnection', function () {

	beforeEach(function () {
		helper.setupAndLoadDocument('writer/writer-edit.fodt');
		cy.getFrameWindow().then((win) => {
			this.win = win;
		});
	});

	it('Page position is preserved after WebSocket reconnection', function () {
		desktopHelper.assertVisiblePage(1, 1, 12);

		// Navigate to the last page
		helper.typeIntoDocument('{ctrl}{End}');
		desktopHelper.assertVisiblePage(12, 12, 12);

		// Close the raw WebSocket to trigger automatic reconnection
		let preDisconnectY1;
		cy.then(() => {
			preDisconnectY1 = this.win.app.activeDocument.activeLayout.viewedRectangle.y1;
			expect(preDisconnectY1).to.be.greaterThan(0);
			this.win.app.socket.socket.close();
		});

		// Can't use processToIdle with the socket closed
		cy.wait(500);

		// Wait for reconnection to complete
		cy.cGet('#document-canvas').should('be.visible');
		cy.getFrameWindow().its('app.socket._reconnecting')
			.should('eq', false);

		// The reload after reconnection reports the document size in steps,
		// and a size-suppression timer holds the old size until those reports
		// stop. Wait for that timer so the view has settled before reading
		// its position, rather than catching it mid-reload.
		cy.then(() => {
			helper.waitForTimers(this.win, 'reconnectfilesize');
			helper.processToIdle(this.win);
		});

		// Verify the page position is preserved after reconnection
		desktopHelper.assertVisiblePage(12, 12, 12);

		cy.cGet('#document-canvas').click(200, 200);
		cy.then(() => {
			helper.processToIdle(this.win);
		});

		desktopHelper.assertVisiblePage(12, 12, 12);
		cy.then(() => {
			const DRIFT_TOLERANCE_TWIPS = 5000;
			var postClickY1 = this.win.app.activeDocument.activeLayout.viewedRectangle.y1;
			var drift = Math.abs(postClickY1 - preDisconnectY1);
			expect(drift).to.be.lessThan(DRIFT_TOLERANCE_TWIPS);
		});
	});
});
