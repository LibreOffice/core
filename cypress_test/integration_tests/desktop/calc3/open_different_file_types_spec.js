/* global describe it cy require */
var helper = require('../../common/helper');
var calcHelper = require('../../common/calc_helper');
var desktopHelper = require('../../common/desktop_helper');

describe.skip(['tagdesktop', 'tagnextcloud', 'tagproxy'], 'Open different file types', function () {

	function assertData() {
		var expectedData = [
			'0', 'First Name', 'Last Name', 'Gender', 'Country', 'Age', 'Date', 'Id',
			'1', 'Dulce', 'Abril', 'Female', 'United States', '32', '15/10/2017', '1562',
			'2', 'Mara', 'Hashimoto', 'Female', 'Great Britain', '25', '16/08/2016', '1582',
			'3', 'Philip', 'Gent', 'Male', 'France', '36', '21/05/2015', '2587',
			'4', 'Kathleen', 'Hanner', 'Female', 'United States', '25', '15/10/2017', '3549',
		];
		calcHelper.assertSheetContents(expectedData);
	}

	it('Open xls file', { defaultCommandTimeout: 60000 }, function () {
		helper.setupAndLoadDocument('calc/testfile.xls');

		assertData();

		desktopHelper.insertImage();
	});

	it('Open xlsx file', { defaultCommandTimeout: 60000 }, function () {
		helper.setupAndLoadDocument('calc/testfile.xlsx');

		assertData();
	});

	it('Open csv file', { defaultCommandTimeout: 60000 }, function() {
		//to fit csv jsdialog in window
		cy.viewport(1280, 960);

		var newFilePath = helper.setupDocument('calc/testfile.csv');
		// Skip document check to click through import csv dialog first
		helper.loadDocument(newFilePath,true);

		cy.cGet('form.jsdialog-container.lokdialog_container').should('exist');
		cy.cGet('.ui-pushbutton.jsdialog.button-primary').click();

		//check doc is loaded
		helper.documentChecks();

		cy.cGet('#mobile-edit-button')
			.should('be.visible')
			.click();

		cy.cGet('#modal-dialog-switch-to-edit-mode-modal-yesbutton').click();

		assertData();
	});

	it('Open xlsb file', { defaultCommandTimeout: 60000 }, function() {
		desktopHelper.openReadOnlyFile('calc/testfile.xlsb');

		cy.cGet('#mobile-edit-button').should('be.visible').click();
		cy.cGet('#modal-dialog-switch-to-edit-mode-modal-overlay').should('be.visible');
		cy.cGet('#modal-dialog-switch-to-edit-mode-modal-label')
			.should('have.text', 'This document may contain formatting or content that cannot be saved in the current file format.');

		cy.cGet('#modal-dialog-switch-to-edit-mode-modal-yesbutton').should('have.text', 'Continue read only').click();
		cy.cGet('#PermissionMode').should('be.visible').should('have.text', ' Read-only ');
	});

	it('Open xlsm file', { defaultCommandTimeout: 60000 }, function() {
		helper.setupAndLoadDocument('calc/testfile.xlsm');

		assertData();

		desktopHelper.insertImage();
	});

	it('Open xltm file', { defaultCommandTimeout: 60000 }, function() {
		desktopHelper.openReadOnlyFile('calc/testfile.xltm');

		cy.cGet('#mobile-edit-button').should('not.be.visible');
	});

	it('Open xltx file', { defaultCommandTimeout: 60000 }, function() {
		desktopHelper.openReadOnlyFile('calc/testfile.xltm');

		cy.cGet('#mobile-edit-button').should('not.be.visible');
	});

	it('Open fods file', { defaultCommandTimeout: 60000 }, function() {
		helper.setupAndLoadDocument('calc/testfile.fods');

		//select all the content of doc
		assertData();

		desktopHelper.insertImage();
	});
});
