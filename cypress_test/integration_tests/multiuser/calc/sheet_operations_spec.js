/* global describe it cy beforeEach require */

var helper = require('../../common/helper');

describe.skip(['tagmultiuser'], 'Multiuser sheet operations', function() {

	beforeEach(function() {
		helper.setupAndLoadDocument('calc/sheet_operations.ods',true);
	});

	function testInsertDelete(frameId1, frameId2) {
		// We have one sheet by default
		//assert for user-1/2
		cy.cSetActiveFrame(frameId1);
		cy.cGet('.spreadsheet-tab').should('have.length', 1);
		cy.cGet('#spreadsheet-tab0').should('have.text', 'Sheet1');

		//assert for user-1/2
		cy.cSetActiveFrame(frameId2);
		cy.cGet('.spreadsheet-tab').should('have.length', 1);
		cy.cGet('#spreadsheet-tab0').should('have.text', 'Sheet1');

		// Add one more sheet
		cy.cSetActiveFrame(frameId1);
		cy.cGet('#insertsheet').click();

		//assert for user-1/2
		cy.cGet('.spreadsheet-tab').should('have.length', 2);
		cy.cGet('#spreadsheet-tab1').should('have.text', 'Sheet2');

		//assert for user-1/2
		cy.cSetActiveFrame(frameId2);
		cy.cGet('.spreadsheet-tab').should('have.length', 2);
		cy.cGet('#spreadsheet-tab1').should('have.text', 'Sheet2');
		cy.wait(2000);

		//user-1/2 removes it
		cy.cGet('#spreadsheet-tab0').rightclick();
		cy.cGet('body').contains('.ui-combobox-entry.jsdialog.ui-grid-cell', 'Delete Sheet...').click();
		cy.cGet('#delete-sheet-modal-response').click();

		//assert for user-1/2
		cy.cGet('.spreadsheet-tab').should('have.length', 1);
		cy.cGet('#spreadsheet-tab0').should('have.text', 'Sheet2');

		//assert for user-1/2
		cy.cSetActiveFrame(frameId1);
		cy.cGet('.spreadsheet-tab').should('have.length', 1);
		cy.cGet('#spreadsheet-tab0').should('have.text', 'Sheet2');
	}
	it('user-1 insert and user-2 delete sheet.', function() {
		testInsertDelete('#iframe1', '#iframe2');
	});

	it('user-2 insert and user-1 delete sheet', function() {
		testInsertDelete('#iframe2', '#iframe1');
	});

});

describe(['tagmultiuser'], 'Check overlays after tab switching/operations', function() {

	beforeEach(function() {
		helper.setupAndLoadDocument('calc/cell_cursor_overlay.ods',true);
	});

	it('Check cell cursor overlay bounds after switching tab', function () {
		cy.cSetActiveFrame('#iframe1');
		cy.cGet('#spreadsheet-tab1').click();
		cy.cGet(helper.addressInputSelector).should('have.prop', 'value', 'A14');

		cy.cGet('#test-div-OwnCellCursor').should('exist');

		cy.cSetActiveFrame('#iframe2');
		// Check that cell cursor have the same bounds in both views
		cy.cGet('#spreadsheet-tab1').click();
		cy.cGet(helper.addressInputSelector).should('have.prop', 'value', 'A14');
		cy.cGet('#test-div-OwnCellCursor').should('exist');
	});

	it('Check cell view cursor overlay bounds after switching tab', function () {
		cy.cSetActiveFrame('#iframe1');
		cy.cGet('#spreadsheet-tab1').click();

		cy.cGet('#test-div-OwnCellCursor').should('exist');

		cy.cSetActiveFrame('#iframe2');
		cy.cGet('#spreadsheet-tab1').click();

		// check that cell cursor and cell view cursor from first view have the same bounds
		// since we don't know if view 0 has been loaded in frame1 or in frame2 we look for an element
		// starting with 'test-div-overlay-cell-view-cursor' and ending with 'border-0'.

		// TODO: re-enable this after using the same class for view-cellcursors.

		//cy.cframe()
		//	.find('[id^="test-div-overlay-cell-view-cursor"][id$="border-0"]')
		//	.then(elements => {
		//		expect(elements.length).to.be.equals(1);
		//		const id = elements[0].id;
		//		// cy.log('id: ' + id);
		//		helper.overlayItemHasBounds('#' + id, cellA14Bounds);
		//	});
	});

	it.skip('Check cell view cursor overlay bounds after inserting a new tab', function () {
		cy.cSetActiveFrame('#iframe1');
		cy.cGet(helper.addressInputSelector).should('have.prop', 'value', 'D8');

		cy.cSetActiveFrame('#iframe2');
		helper.typeIntoInputField(helper.addressInputSelector, 'F6');
		cy.cGet(helper.addressInputSelector).should('have.prop', 'value', 'F6');

		cy.cSetActiveFrame('#iframe1');
		cy.cGet('#insertsheet').click();

		// check that there is no cell view cursor;
		// in fact in the other view the new sheet has never been selected
		// since we don't know if view 0 has been loaded in frame1 or in frame2 we test for both.

		// TODO: re-enable this after using the same class for view-cellcursors.

		//cy.cGet('#test-div-overlay-cell-view-cursor-0-border-0').should('not.exist');
		//cy.cGet('#test-div-overlay-cell-view-cursor-1-border-0').should('not.exist');
	});

});

