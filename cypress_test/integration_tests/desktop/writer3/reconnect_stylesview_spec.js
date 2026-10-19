/* global describe it cy beforeEach require expect */

var helper = require('../../common/helper');
var desktopHelper = require('../../common/desktop_helper');

describe(['tagdesktop'], 'Style gallery after WebSocket reconnection', function () {

	beforeEach(function () {
		helper.setupAndLoadDocument('writer/writer-edit.fodt');
		desktopHelper.switchUIToNotebookbar();
		cy.getFrameWindow().then((win) => {
			this.win = win;
		});
	});

	it('Gallery previews are requested once the reloaded document is ready', function () {
		// The Styles deck has engine-rendered entries of its own, so the reload
		// after the reconnection builds both it and the gallery on the engine side.
		cy.then(() => {
			this.win.app.socket.sendMessage('uno .uno:SidebarDeck.StyleListDeck');
		});
		cy.cGet('#StyleListDeck').should('be.visible');
		helper.processToIdle(this.win);
		helper.waitForOnDemandRenders(this.win);

		// Every gallery entry shows its preview before the connection drops.
		cy.cGet('#stylesview .ui-iconview-entry').should('have.length.greaterThan', 0);
		cy.cGet('#stylesview .ui-iconview-ondemand-placeholder').should('not.exist');

		// Record the render requests that go out while the document is not loaded,
		// then close the raw WebSocket to trigger the automatic reconnection.
		cy.then(() => {
			var win = this.win;
			win.reconnected = false;
			win.app.map.on('statusindicator', (e) => {
				if (e.statusType === 'reconnected') win.reconnected = true;
			});
			win.earlyRenderRequests = [];
			var sendMessage = win.app.socket.sendMessage.bind(win.app.socket);
			win.app.socket.sendMessage = function (msg) {
				if (typeof msg === 'string' && msg.indexOf('render_entry') >= 0 && !win.app.map._docLoaded)
					win.earlyRenderRequests.push(msg);
				return sendMessage(msg);
			};
			win.app.socket.socket.close();
		});

		// The reconnection has loaded the document again.
		cy.getFrameWindow().should((win) => {
			expect(win.reconnected).to.equal(true);
			expect(win.app.socket._reconnecting).to.equal(false);
		});
		helper.processToIdle(this.win);
		helper.waitForOnDemandRenders(this.win);

		// A request made while the new view was still being built on the engine
		// side reaches no widget there and is dropped, so none may go out then.
		cy.getFrameWindow().should((win) => {
			expect(win.earlyRenderRequests).to.be.empty;
		});

		// And every gallery entry shows its preview again.
		cy.cGet('#stylesview .ui-iconview-entry').should('have.length.greaterThan', 0);
		cy.cGet('#stylesview .ui-iconview-ondemand-placeholder').should('not.exist');
	});
});
