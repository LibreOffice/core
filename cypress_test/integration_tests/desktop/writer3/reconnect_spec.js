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

	it('Keeps the view when the connection drops without a close frame', function () {
		// Put the cursor on the last page. A view that came back as a new
		// session would start on the first page, so the cursor tells the two
		// apart.
		helper.typeIntoDocument('{ctrl}{End}');
		desktopHelper.assertVisiblePage(12, 12, 12);
		cy.getFrameWindow().should(function (win) {
			expect(win.app.file.textCursor.rectangle.y1).to.be.greaterThan(0);
		});

		let viewIdBefore;
		let cursorY1Before;
		let connectCountBefore;
		let tokenBefore;
		cy.then(() => {
			viewIdBefore = this.win.app.map._docLayer._viewId;
			cursorY1Before = this.win.app.file.textCursor.rectangle.y1;
			connectCountBefore = this.win.app.socket._connectCount;
			// The server gave the page a token after the first load.
			tokenBefore = this.win.reconnectToken;
			expect(tokenBefore).to.match(/^[0-9A-F]{64}$/);
			// The server closes the socket without a close frame, as a bad
			// proxy would, so the session is parked rather than closed.
			this.win.app.socket.sendMessage('debugdropconnection');
		});

		// No processToIdle while the socket is down. The reconnect is finished
		// when one more socket has been opened, the status has arrived, and the
		// idle handler has sent useractive, since outgoing messages are dropped
		// until it has.
		cy.getFrameWindow().should(function (win) {
			expect(win.app.socket._connectCount).to.equal(connectCountBefore + 1);
			expect(win.app.socket._reconnecting).to.equal(false);
			expect(win.app.socket.connected()).to.equal(true);
			expect(win.app.idleHandler._active).to.equal(true);
			expect(win.app.socket.socket.url).to.contain('ReconnectToken=' + tokenBefore);
			// The reattached view got a new token for the next reconnect.
			expect(win.reconnectToken).to.match(/^[0-9A-F]{64}$/);
			expect(win.reconnectToken).to.not.equal(tokenBefore);
		});
		cy.cGet('#document-canvas').should('be.visible');
		cy.then(() => {
			helper.waitForTimers(this.win, 'reconnectfilesize');
			helper.processToIdle(this.win);
		});

		// The session was reattached, so the view id did not change. The tile layer was
		// rebuilt and fetches tiles again once it knows its canonical view id.
		cy.getFrameWindow().should(function (win) {
			expect(win.app.map._docLayer._viewId).to.equal(viewIdBefore);
			expect(win.app.map._docLayer._canonicalIdInitialized).to.equal(true);
		});

		// The cursor rectangle is not cleared on close, so typing makes core
		// report where the cursor really is. The reattached view kept its
		// cursor in the kit, so it is on the same line. A view that had
		// started over would be near the top of the document, many pages away.
		helper.typeIntoDocument('x');
		cy.then(() => {
			helper.processToIdle(this.win);
		});
		cy.getFrameWindow().should(function (win) {
			const ONE_LINE_TWIPS = 1000;
			const drift = Math.abs(win.app.file.textCursor.rectangle.y1 - cursorY1Before);
			expect(drift).to.be.lessThan(ONE_LINE_TWIPS);
		});
		desktopHelper.assertVisiblePage(12, 12, 12);
	});
});
