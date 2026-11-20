/* global describe it cy beforeEach require */

var helper = require('../../common/helper');

describe.skip(['tagmultiuser'], 'Navigator follow the change of document', function() {

	beforeEach(function() {
		helper.setupAndLoadDocument('writer/paragraph_prop.odt',true, false, undefined, 'vector=false');
	});

	it('Create Heading, Table, then modify, and delete', function() {
		// Both user open Navigator
		cy.cSetActiveFrame('#iframe1');
		cy.cGet('#menu-view').click();
		cy.cGet('#menu-navigator').click();
		cy.cSetActiveFrame('#iframe2');
		cy.cGet('#menu-view').click();
		cy.cGet('#menu-navigator').click();

		// User 1 Change 'text' style to heading 1
		cy.cSetActiveFrame('#iframe1');
		cy.cGet('#styles .ui-combobox-button').click();
		cy.cGet('[id$="-dropdown"].modalpopup').contains('span', 'Heading 1').click();

		// Create a Table
		cy.cGet('#menu-table').click();
		cy.cGet('#menu-table').contains('Insert Table...').click();
		cy.cGet('#lokdialog-3-canvas').type('{enter}');
		cy.cGet('#contenttree').contains('.jsdialog.sidebar.ui-treeview-cell-text', 'Tables').parent().prev().click();

		// User 2 check if the heading and Table is visible
		cy.cSetActiveFrame('#iframe2');
		cy.cGet('#contenttree').contains('.jsdialog.sidebar.ui-treeview-cell-text', 'Tables').parent().prev().click();
		cy.cGet('#contenttree').contains('.jsdialog.sidebar.ui-treeview-cell-text', 'text').should('be.visible');
		cy.cGet('#contenttree').contains('.jsdialog.sidebar.ui-treeview-cell-text', 'Table1').should('be.visible');

		// User 2 change heading
		helper.typeIntoDocument('Changed');
		cy.cGet('#contenttree').contains('.jsdialog.sidebar.ui-treeview-cell-text', 'Changedtext').should('be.visible');

		// User 1 check if the heading text changed
		cy.cSetActiveFrame('#iframe1');
		cy.cGet('#document-container').click();
		cy.cGet('#contenttree').contains('.jsdialog.sidebar.ui-treeview-cell-text', 'Changedtext').should('be.visible');

		// User 1 delete heading
		cy.cGet('#document-container').click();
		cy.cGet('#styles .ui-combobox-button').click();
		cy.cGet('[id$="-dropdown"].modalpopup').contains('span', 'Title').click();

		// User 2 check if the heading removed from navigator
		cy.cSetActiveFrame('#iframe2');
		cy.cGet('#document-container').click();
		cy.cGet('#contenttree').contains('.jsdialog.sidebar.ui-treeview-cell-text', 'Changedtext').should('not.exist');
	});
});
