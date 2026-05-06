/* global describe it cy require beforeEach */
var helper = require('../../common/helper');
var calcHelper = require('../../common/calc_helper');
var desktopHelper = require('../../common/desktop_helper');

describe(['tagdesktop'], 'Row Column Operation', function() {
	beforeEach(function() {
		helper.setupAndLoadDocument('calc/row_column_operation.ods');
		desktopHelper.switchUIToNotebookbar();
		cy.viewport(1920,1080);
		cy.getFrameWindow().then(function(frameWindow) {
			this.win = frameWindow;
		});
		helper.setDummyClipboardForCopy();
		calcHelper.assertSheetContents(['Hello','Hi','World','Bye'], true);
		calcHelper.clickOnFirstCell(true,false);
	});

	it('Insert/Delete row' , function() {
		//Insert row above
		desktopHelper.getNbIcon('InsertColumnsBefore', 'Home').click();
		helper.processToIdle(this.win);

		//calcHelper.assertSheetContents(['','','Hello','Hi','World','Bye']);
		//delete row
		calcHelper.clickOnFirstCell(true, false);

		desktopHelper.getNbIcon('DeleteRows', 'Home').click();
		helper.processToIdle(this.win);
		//calcHelper.assertSheetContents(['Hello','Hi','World','Bye']);

		//insert row below
		calcHelper.clickOnFirstCell(true, false);
		desktopHelper.getNbIcon('InsertRowsAfter', 'Home').click();
		//calcHelper.assertSheetContents(['Hello','Hi','','','World','Bye']);
	});

	it('Insert/Delete Column', function() {
		//insert column before
		desktopHelper.getNbIcon('InsertColumnsBefore', 'Home').click();
		helper.processToIdle(this.win);
		//calcHelper.assertSheetContents(['','Hello','Hi','','World','Bye']);
		calcHelper.clickOnFirstCell(true, false);

		//delete column
		desktopHelper.getNbIcon('DeleteColumns', 'Home').click();
		helper.processToIdle(this.win);
		//calcHelper.assertSheetContents(['Hello','Hi','World','Bye']);
		calcHelper.clickOnFirstCell(true, false);

		//insert column after
		desktopHelper.getNbIcon('InsertColumnsAfter', 'Home').click();
		//calcHelper.assertSheetContents(['Hello','','Hi','World','','Bye']);
	});
});
