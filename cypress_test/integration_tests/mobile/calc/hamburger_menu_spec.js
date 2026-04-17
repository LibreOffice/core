/* global describe it cy require expect */

var helper = require('../../common/helper');
var calcHelper = require('../../common/calc_helper');
var mobileHelper = require('../../common/mobile_helper');
var repairHelper = require('../../common/repair_document_helper');

describe.skip(['tagmobile'], 'Trigger hamburger menu options.', function() {

	it('Save', { defaultCommandTimeout: 60000 }, function() {
		var newFilePath = helper.setupAndLoadDocument('calc/hamburger_menu.ods');
		mobileHelper.enableEditingMobile();

		calcHelper.selectEntireSheet();

		cy.cGet('#copy-paste-container table td').should('contain.text', 'Textx');
		calcHelper.clickOnFirstCell(true, true);
		helper.selectAllText();
		helper.typeIntoDocument('new');
		calcHelper.selectEntireSheet();
		cy.cGet('#copy-paste-container table td').should('contain.text', 'new');
		mobileHelper.selectHamburgerMenuItem(['File', 'Save']);

		// Reopen the document and check content.
		helper.reloadDocument(newFilePath);

		mobileHelper.enableEditingMobile();
		calcHelper.selectEntireSheet();
		cy.cGet('#copy-paste-container table td').should('contain.text', 'new');
	});

	it('Print', function() {
		helper.setupAndLoadDocument('calc/hamburger_menu.ods');
		mobileHelper.enableEditingMobile();

		// A new window should be opened with the PDF.
		cy.getFrameWindow()
			.then(function(win) {
				cy.stub(win, 'open');
			});

		mobileHelper.selectHamburgerMenuItem(['File', 'Print']);

		cy.getFrameWindow().its('open').should('be.called');
	});

	it('Download as PDF', function() {
		helper.setupAndLoadDocument('calc/hamburger_menu.ods');
		mobileHelper.enableEditingMobile();

		mobileHelper.selectHamburgerMenuItem(['Download as', 'PDF Document (.pdf)']);
		mobileHelper.pressPushButtonOfDialog('Export');
		cy.cGet('iframe').should('have.attr', 'data-src').should('contain', 'download');
	});

	it('Download as ODS', function() {
		helper.setupAndLoadDocument('calc/hamburger_menu.ods');
		mobileHelper.enableEditingMobile();

		mobileHelper.selectHamburgerMenuItem(['Download as', 'ODF spreadsheet (.ods)']);
		cy.cGet('iframe').should('have.attr', 'data-src').should('contain', 'download');
	});

	it('Download as XLS', function() {
		helper.setupAndLoadDocument('calc/hamburger_menu.ods');
		mobileHelper.enableEditingMobile();

		mobileHelper.selectHamburgerMenuItem(['Download as', 'Excel 2003 Spreadsheet (.xls)']);
		cy.cGet('iframe').should('have.attr', 'data-src').should('contain', 'download');
	});

	it('Download as XLSX', function() {
		helper.setupAndLoadDocument('calc/hamburger_menu.ods');
		mobileHelper.enableEditingMobile();

		mobileHelper.selectHamburgerMenuItem(['Download as', 'Excel Spreadsheet (.xlsx)']);
		cy.cGet('iframe').should('have.attr', 'data-src').should('contain', 'download');
	});

	it('Undo/redo.', function() {
		helper.setupAndLoadDocument('calc/hamburger_menu.ods');
		mobileHelper.enableEditingMobile();

		// Type a new character
		calcHelper.clickOnFirstCell(true, true);
		cy.cGet('div.clipboard').type('{q}');
		calcHelper.selectEntireSheet();
		cy.cGet('#copy-paste-container table td').should('contain.text', 'q');

		// Undo
		mobileHelper.selectHamburgerMenuItem(['Edit', 'Undo']);

		cy.cGet(helper.addressInputSelector).should('have.prop', 'value', 'A1');

		calcHelper.selectEntireSheet();

		cy.cGet('#copy-paste-container table td').should('not.contain.text', 'q');

		// Redo
		mobileHelper.selectHamburgerMenuItem(['Edit', 'Redo']);

		cy.cGet(helper.addressInputSelector).should('have.prop', 'value', 'A1');
		calcHelper.selectEntireSheet();
		cy.cGet('#copy-paste-container table td').should('contain.text', 'q');
	});

	it('Repair Document', function() {
		helper.setupAndLoadDocument('calc/hamburger_menu.ods');
		mobileHelper.enableEditingMobile();

		// Type a new character
		calcHelper.clickOnFirstCell(true, true);
		cy.cGet('div.clipboard').type('{q}');

		calcHelper.selectEntireSheet();

		cy.cGet('#copy-paste-container table td').should('contain.text', 'q');

		// Revert one undo step via Repair
		repairHelper.rollbackPastChange('Undo', undefined, true);

		cy.cGet(helper.addressInputSelector).should('have.prop', 'value', 'A1');

		calcHelper.selectEntireSheet();

		cy.cGet('#copy-paste-container table td').should('not.contain.text', 'q');
	});

	it('Cut.', function() {
		helper.setupAndLoadDocument('calc/hamburger_menu.ods');
		mobileHelper.enableEditingMobile();

		calcHelper.selectEntireSheet();
		mobileHelper.selectHamburgerMenuItem(['Edit', 'Cut']);
		cy.cGet('#mobile-wizard-content-modal-dialog-copy_paste_warning-box').should('exist');
	});

	it('Copy.', function() {
		helper.setupAndLoadDocument('calc/hamburger_menu.ods');
		mobileHelper.enableEditingMobile();

		calcHelper.selectEntireSheet();
		mobileHelper.selectHamburgerMenuItem(['Edit', 'Copy']);
		cy.cGet('#mobile-wizard-content-modal-dialog-copy_paste_warning-box').should('exist');
	});

	it('Paste.', function() {
		helper.setupAndLoadDocument('calc/hamburger_menu.ods');
		mobileHelper.enableEditingMobile();

		calcHelper.selectEntireSheet();
		mobileHelper.selectHamburgerMenuItem(['Edit', 'Paste']);
		cy.cGet('#mobile-wizard-content-modal-dialog-copy_paste_warning-box').should('exist');
	});

	it('Select all.', function() {
		helper.setupAndLoadDocument('calc/hamburger_menu.ods');
		mobileHelper.enableEditingMobile();

		mobileHelper.selectHamburgerMenuItem(['Edit', 'Select All']);
		cy.cGet('#test-div-cell_selection_handle_start').should('exist');
		cy.cGet('#copy-paste-container table td').should('contain.text', 'Text');
	});

	it('Search some word.', function() {
		helper.setupAndLoadDocument('calc/hamburger_menu_search.ods');
		mobileHelper.enableEditingMobile();

		mobileHelper.selectHamburgerMenuItem(['Search']);
		// Search bar become visible
		cy.cGet('#mobile-wizard-content').should('not.be.empty');
		// Search for some word
		helper.typeIntoInputField('#searchterm', 'a');
		cy.cGet('#search').should('not.have.attr', 'disabled');
		cy.cGet('#search').click();
		// First cell should be selected
		cy.cGet(helper.addressInputSelector).should('have.prop', 'value', 'A1');
	});

	it('Sheet: insert row before.', function() {
		helper.setupAndLoadDocument('calc/hamburger_menu_sheet.ods');
		mobileHelper.enableEditingMobile();

		calcHelper.clickOnFirstCell();
		mobileHelper.selectHamburgerMenuItem(['Sheet', 'Insert Rows', 'Rows Above']);
		calcHelper.selectEntireSheet();
		cy.cGet('#copy-paste-container table tr').should('have.length', 3);
		cy.cGet('#copy-paste-container table tr td:nth-of-type(1)')
			.should(function(cells) {
				expect(cells).to.have.lengthOf(3);
				expect(cells[0]).to.have.text('');
				expect(cells[1]).to.have.text('1');
				expect(cells[2]).to.have.text('3');
			});
	});

	it('Sheet: insert row after.', function() {
		helper.setupAndLoadDocument('calc/hamburger_menu_sheet.ods');
		mobileHelper.enableEditingMobile();

		calcHelper.clickOnFirstCell();
		mobileHelper.selectHamburgerMenuItem(['Sheet', 'Insert Rows', 'Rows Below']);
		calcHelper.selectEntireSheet();
		cy.cGet('#copy-paste-container table tr').should('have.length', 3);
		cy.cGet('#copy-paste-container table tr td:nth-of-type(1)')
			.should(function(cells) {
				expect(cells).to.have.lengthOf(3);
				expect(cells[0]).to.have.text('1');
				expect(cells[1]).to.have.text('');
				expect(cells[2]).to.have.text('3');
			});
	});

	it('Sheet: insert column before.', function() {
		helper.setupAndLoadDocument('calc/hamburger_menu_sheet.ods');
		mobileHelper.enableEditingMobile();

		calcHelper.clickOnFirstCell();
		mobileHelper.selectHamburgerMenuItem(['Sheet', 'Insert Columns', 'Columns Before']);
		calcHelper.selectEntireSheet();
		cy.cGet('#copy-paste-container table tr').should('have.length', 2);
		cy.cGet('#copy-paste-container table tr:nth-of-type(1) td')
			.should(function(cells) {
				expect(cells).to.have.lengthOf(3);
				expect(cells[0]).to.have.text('');
				expect(cells[1]).to.have.text('1');
				expect(cells[2]).to.have.text('2');
			});
	});

	it('Sheet: insert column after.', function() {
		helper.setupAndLoadDocument('calc/hamburger_menu_sheet.ods');
		mobileHelper.enableEditingMobile();

		calcHelper.clickOnFirstCell();
		mobileHelper.selectHamburgerMenuItem(['Sheet', 'Insert Columns', 'Columns After']);
		calcHelper.selectEntireSheet();
		cy.cGet('#copy-paste-container table tr').should('have.length', 2);
		cy.cGet('#copy-paste-container table tr:nth-of-type(1) td')
			.should(function(cells) {
				expect(cells).to.have.lengthOf(3);
				expect(cells[0]).to.have.text('1');
				expect(cells[1]).to.have.text('');
				expect(cells[2]).to.have.text('2');
			});
	});

	it('Sheet: delete rows.', function() {
		helper.setupAndLoadDocument('calc/hamburger_menu_sheet.ods');
		mobileHelper.enableEditingMobile();

		calcHelper.clickOnFirstCell();
		mobileHelper.selectHamburgerMenuItem(['Sheet', 'Delete Rows']);
		calcHelper.selectEntireSheet();
		cy.cGet('#copy-paste-container table tr').should('have.length', 1);
		cy.cGet('#copy-paste-container table tr:nth-of-type(1) td')
			.should(function(cells) {
				expect(cells).to.have.lengthOf(2);
				expect(cells[0]).to.have.text('3');
			});
	});

	it('Sheet: delete columns.', function() {
		helper.setupAndLoadDocument('calc/hamburger_menu_sheet.ods');
		mobileHelper.enableEditingMobile();

		calcHelper.clickOnFirstCell();
		mobileHelper.selectHamburgerMenuItem(['Sheet', 'Delete Columns']);
		calcHelper.selectEntireSheet();
		cy.cGet('#copy-paste-container table tr').should('have.length', 2);
		cy.cGet('#copy-paste-container table tr:nth-of-type(1) td')
			.should(function(cells) {
				expect(cells).to.have.lengthOf(1);
				expect(cells[0]).to.have.text('2');
			});
	});

	it('Data: sort ascending.', function() {
		helper.setupAndLoadDocument('calc/hamburger_menu_sort.ods');
		mobileHelper.enableEditingMobile();

		// Sort the first column's data
		calcHelper.selectFirstColumn();
		mobileHelper.selectHamburgerMenuItem(['Data', 'Sort Ascending']);
		calcHelper.selectEntireSheet();
		cy.cGet('#copy-paste-container table tr').should('have.length', 4);
		cy.cGet('#copy-paste-container table td')
			.should(function(cells) {
				expect(cells).to.have.lengthOf(4);
				expect(cells[0]).to.have.text('1');
				expect(cells[1]).to.have.text('2');
				expect(cells[2]).to.have.text('3');
				expect(cells[3]).to.have.text('4');
			});
	});

	it('Data: sort descending.', function() {
		helper.setupAndLoadDocument('calc/hamburger_menu_sort.ods');
		mobileHelper.enableEditingMobile();

		// Sort the first column's data
		calcHelper.selectFirstColumn();
		mobileHelper.selectHamburgerMenuItem(['Data', 'Sort Descending']);
		calcHelper.selectEntireSheet();
		cy.cGet('#copy-paste-container table tr').should('have.length', 4);
		cy.cGet('#copy-paste-container table td')
			.should(function(cells) {
				expect(cells).to.have.lengthOf(4);
				expect(cells[0]).to.have.text('4');
				expect(cells[1]).to.have.text('3');
				expect(cells[2]).to.have.text('2');
				expect(cells[3]).to.have.text('1');
			});
	});

	it('Data: grouping / ungrouping.', function() {
		helper.setupAndLoadDocument('calc/hamburger_menu.ods');
		mobileHelper.enableEditingMobile();

		// Group first
		calcHelper.selectFirstColumn();
		mobileHelper.selectHamburgerMenuItem(['Data', 'Group and Outline', 'Group...']);
		cy.cGet('[id="test-div-column group"]').should('exist');
		// Then ungroup
		mobileHelper.selectHamburgerMenuItem(['Data', 'Group and Outline', 'Ungroup...']);
		cy.cGet('[id="test-div-column group"]').should('not.exist');
	});

	it('Data: remove grouping outline.', function() {
		helper.setupAndLoadDocument('calc/hamburger_menu.ods');
		mobileHelper.enableEditingMobile();

		// Group first
		calcHelper.selectFirstColumn();
		mobileHelper.selectHamburgerMenuItem(['Data', 'Group and Outline', 'Group...']);
		cy.cGet('[id="test-div-column group"]').should('exist');

		// Then remove outline
		mobileHelper.selectHamburgerMenuItem(['Data', 'Group and Outline', 'Remove Outline']);
		cy.cGet('[id="test-div-column group"]').should('not.exist');
	});

	it('Data: show / hide grouping details.', function() {
		helper.setupAndLoadDocument('calc/hamburger_menu.ods');
		mobileHelper.enableEditingMobile();

		// Group first
		calcHelper.selectFirstColumn();

		mobileHelper.selectHamburgerMenuItem(['Data', 'Group and Outline', 'Group...']);

		cy.cGet('[id="test-div-column group"]').should('exist');

		// Use selected content as indicator
		calcHelper.selectEntireSheet();

		cy.cGet('#copy-paste-container table').should('exist');

		// Hide details
		mobileHelper.selectHamburgerMenuItem(['Data', 'Group and Outline', 'Hide Details']);

		// First column is hidden -> no content
		calcHelper.selectEntireSheet();

		cy.cGet('#copy-paste-container table').should('not.exist');

		// Show details
		mobileHelper.selectHamburgerMenuItem(['Data', 'Group and Outline', 'Show Details']);

		// First column is visible again -> we have content again
		calcHelper.selectEntireSheet();

		cy.cGet('#copy-paste-container table').should('exist');
	});

	it('Check version information.', function() {
		helper.setupAndLoadDocument('calc/hamburger_menu.ods');
		mobileHelper.enableEditingMobile();

		mobileHelper.selectHamburgerMenuItem(['About']);

		cy.cGet('#mobile-wizard-content').should('exist');

		// Check the version
		cy.cGet('body').find('#coolwsd-version').should('exist');

		// Close about dialog
		cy.cGet('div.mobile-wizard.jsdialog-overlay.cancellable').click({force : true});
	});
});
