/* global describe it cy require beforeEach */
var helper = require('../../common/helper');
var calcHelper = require('../../common/calc_helper');
var mobileHelper = require('../../common/mobile_helper');

describe.skip(['tagmobile', 'tagnextcloud', 'tagproxy'], 'Row Column Operation', function() {

	beforeEach(function() {
		helper.setupAndLoadDocument('calc/row_column_operation.ods', false, false, undefined, 'vector=false');

		mobileHelper.enableEditingMobile();

		calcHelper.assertSheetContents(['Hello','Hi','World','Bye']);

		calcHelper.clickOnFirstCell(true,false);
	});

	function selectOption(submenu, option) {
		cy.cGet('#toolbar-hamburger').click();
		cy.cGet('.menu-entry-icon.sheetmenu').parent().click();
		cy.cGet('body').contains('.menu-entry-with-icon', submenu).parent().click();

		if (typeof option !== 'undefined') {
			cy.cGet('body').contains('.menu-entry-with-icon', option).parent().click();
		}
	}

	it('Insert/Delete row' , function() {
		//Insert row above
		selectOption('Insert Rows', 'Rows Above');

		calcHelper.assertSheetContents(['','','Hello','Hi','World','Bye']);
		//delete row
		calcHelper.clickOnFirstCell(true, false);

		selectOption('Delete Rows');

		calcHelper.assertSheetContents(['Hello','Hi','World','Bye']);

		//insert row below
		calcHelper.clickOnFirstCell(true, false);

		selectOption('Insert Rows', 'Rows Below');

		calcHelper.assertSheetContents(['Hello','Hi','','','World','Bye']);
	});

	it('Insert/Delete Column', function() {
		//insert column before
		selectOption('Insert Columns', 'Columns Before');

		calcHelper.assertSheetContents(['','Hello','Hi','','World','Bye']);
		calcHelper.clickOnFirstCell(true, false);

		//delete column
		selectOption('Delete Columns');

		calcHelper.assertSheetContents(['Hello','Hi','World','Bye']);
		calcHelper.clickOnFirstCell(true,false);

		//insert column after
		selectOption('Insert Columns', 'Columns After');

		calcHelper.assertSheetContents(['Hello','','Hi','World','','Bye']);
	});
});
