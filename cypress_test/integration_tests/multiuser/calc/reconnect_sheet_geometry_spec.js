/* global describe it cy beforeEach require expect */

var helper = require('../../common/helper');
var desktopHelper = require('../../common/desktop_helper');

describe(['tagmultiuser'], 'Sheet geometry after a reconnection', function() {

	beforeEach(function() {
		helper.setupAndLoadDocument('calc/invalidations.ods', true);
		desktopHelper.switchUIToNotebookbar();
	});

	it('A column width changed while an invalidation was lost is fetched again after reconnecting', function() {
		var NEW_WIDTH_MM100 = 5000;
		var win1, win2, widthBefore, connectCountBefore;

		// The width of column A as the sheet geometry of a view knows it, in print twips.
		var columnWidth = function(win) {
			return win.app.map._docLayer.sheetGeometry
				.getColumnsGeometry().getElementDataAny(0, 'printtwips').size;
		};

		cy.cSetActiveFrame('#iframe1');
		cy.getFrameWindow().then((win) => {
			win1 = win;
			helper.processToIdle(win1);
		});
		cy.cSetActiveFrame('#iframe2');
		cy.getFrameWindow().then((win) => {
			win2 = win;
			helper.processToIdle(win2);
		});

		cy.then(() => {
			widthBefore = columnWidth(win2);
			expect(columnWidth(win1)).to.eq(widthBefore);
			// A sheet geometry invalidation that never reaches the second user stands in
			// for a message lost on a bad connection.
			var layer = win2.app.map._docLayer;
			layer._onMessageBeforeTest = layer._onMessage;
			layer._onMessage = function(textMsg, img) {
				if (typeof textMsg === 'string' && textMsg.startsWith('invalidatesheetgeometry:'))
					return;
				return layer._onMessageBeforeTest(textMsg, img);
			};
		});

		// The first user widens column A.
		cy.then(() => {
			win1.app.socket.sendMessage('uno .uno:ColumnWidth ' + JSON.stringify({
				ColumnWidth: { type: 'unsigned short', value: NEW_WIDTH_MM100 }
			}));
			helper.processToIdle(win1);
		});
		cy.then(() => {
			expect(columnWidth(win1)).to.be.greaterThan(widthBefore);
			helper.processToIdle(win2);
		});
		cy.then(() => {
			// The second user still has the old width, the invalidation did not reach it.
			expect(columnWidth(win2)).to.eq(widthBefore);
			var layer = win2.app.map._docLayer;
			layer._onMessage = layer._onMessageBeforeTest;
			delete layer._onMessageBeforeTest;
			// Closing the raw WebSocket triggers the automatic reconnection.
			connectCountBefore = win2.app.socket._connectCount;
			win2.app.socket.socket.close();
		});

		cy.cSetActiveFrame('#iframe2');
		cy.then(() => {
			helper.waitForReconnect(win2, connectCountBefore);
		});
		cy.cGet('#document-canvas').should('be.visible');
		cy.then(() => {
			helper.processToIdle(win2);
		});
		cy.then(() => {
			// The reconnected view fetched the geometry again and agrees with the first user.
			expect(columnWidth(win2)).to.eq(columnWidth(win1));
		});
	});
});
