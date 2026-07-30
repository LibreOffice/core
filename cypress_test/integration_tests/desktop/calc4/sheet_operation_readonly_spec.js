/* global describe it cy expect require */

var helper = require('../../common/helper');

describe(['tagdesktop'], 'Sheet operations in a read-only view.', function () {

	function sendRename(newName) {
		cy.getFrameWindow().then(function (win) {
			win.app.socket.sendMessage(
				'uno .uno:Name {"Name":{"type":"string","value":"' + newName + '"},'
				+ '"Index":{"type":"unsigned short","value":1}}');
			helper.processToIdle(win);
		});
	}

	function loadReadOnly() {
		var filePath = helper.setupDocument('calc/sheet_operation.ods');
		helper.loadDocument(filePath, true, undefined, undefined, 'permission=readonly');
		cy.cGet('#document-canvas').should('be.visible');
		cy.cGet('#spreadsheet-tab0').should('have.text', 'Sheet1');
		cy.getFrameWindow().then(function (win) {
			helper.processToIdle(win);
		});
	}

	it('A read-only view cannot rename a sheet', function () {
		loadReadOnly();

		sendRename('Renamed');

		cy.cGet('#spreadsheet-tab0').should('have.text', 'Sheet1');
	});

	it('A read-only view cannot turn sheet grid lines off, but still learns their setting', function () {
		loadReadOnly();

		// The viewer has to be told whether the sheet draws grid lines, or the
		// canvas falls back to drawing them whatever the document says.
		cy.getFrameWindow().should(function (win) {
			expect(win.app.map['stateChangeHandler']
				.getItemValue('.uno:ToggleSheetGrid')).to.equal('true');
		});

		cy.getFrameWindow().then(function (win) {
			win.app.socket.sendMessage('uno .uno:ToggleSheetGrid');
			return helper.processToIdle(win);
		});

		cy.getFrameWindow().should(function (win) {
			expect(win.app.map._docLayer._sheetGrid).to.be.true;
		});
	});

	it('An editable view still renames a sheet', function () {
		helper.setupAndLoadDocument('calc/sheet_operation.ods');
		cy.cGet('#spreadsheet-tab0').should('have.text', 'Sheet1');

		sendRename('Renamed');

		cy.cGet('#spreadsheet-tab0').should('have.text', 'Renamed');
	});
});
