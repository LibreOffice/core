/* global describe it cy beforeEach require expect */

var helper = require('../../common/helper');
var desktopHelper = require('../../common/desktop_helper');

describe(['tagmultiuser'], 'Sheet geometry after the watched sheet is deleted', function() {

	beforeEach(function() {
		helper.setupAndLoadDocument('calc/sheet_delete_geometry.fods', true);
		desktopHelper.switchUIToNotebookbar();
	});

	it('The view that lands on the next sheet fetches that sheet\'s geometry', function() {
		var win1, win2, wideWidth, defaultWidth;

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

		// The second user switches to Sheet2, which has the wide column A.
		cy.then(() => {
			defaultWidth = columnWidth(win2);
			win2.app.map.setPart(1);
			helper.processToIdle(win2);
		});
		cy.then(() => {
			wideWidth = columnWidth(win2);
			expect(wideWidth).to.be.greaterThan(defaultWidth);
			// A sheet geometry invalidation that never reaches the second user stands in
			// for a message lost on a bad connection, so only the status tells the client
			// that its sheet changed.
			var layer = win2.app.map._docLayer;
			layer._onMessageBeforeTest = layer._onMessage;
			layer._onMessage = function(textMsg, img) {
				if (typeof textMsg === 'string' && textMsg.startsWith('invalidatesheetgeometry:'))
					return;
				return layer._onMessageBeforeTest(textMsg, img);
			};
		});

		// The first user deletes Sheet2, the second user lands on Sheet3 at the same index.
		cy.then(() => {
			win1.app.socket.sendMessage('uno .uno:Remove ' + JSON.stringify({
				Index: { type: 'long', value: 2 }
			}));
			helper.processToIdle(win1);
		});
		cy.then(() => {
			helper.processToIdle(win2);
		});
		cy.then(() => {
			var layer = win2.app.map._docLayer;
			layer._onMessage = layer._onMessageBeforeTest;
			delete layer._onMessageBeforeTest;
			expect(win2.app.calc.partHashes.length).to.eq(2);
			expect(layer._selectedPart).to.eq(1);
			// The geometry now describes Sheet3, with its default column width.
			expect(layer.sheetGeometry.getPartHash()).to.eq(win2.app.calc.partHashes[1]);
			expect(columnWidth(win2)).to.eq(defaultWidth);
		});
	});
});
