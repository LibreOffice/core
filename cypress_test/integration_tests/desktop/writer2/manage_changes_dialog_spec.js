/* global describe it cy beforeEach require  */

var helper = require('../../common/helper');
var desktopHelper = require('../../common/desktop_helper');

describe(['tagdesktop', 'tagnextcloud', 'tagscreenshot'], 'Manage Changes Dialog', function () {

	beforeEach(function () {
		helper.setupAndLoadDocument('writer/manage_tracking_changes.odt');
		cy.getFrameWindow().then((win) => {
			this.win = win;
		});
	});

	it('Manage changes dialog visual test', function () {
		cy.cGet('.notebookbar #Review-tab-label').click();
		desktopHelper.getNbIconArrow('TrackChanges', 'Review').click();
		desktopHelper.getNbIcon('AcceptTrackedChanges', 'Review').click();
		cy.cGet('#AcceptRejectChangesDialog').should('be.visible');
		cy.cGet('#writerchanges .ui-treeview-entry.ui-treeview-expandable[aria-level="1"] .ui-treeview-expander-column').click();
		helper.processToIdle(this.win);
		cy.cGet('#writerchanges .ui-treeview-entry.ui-treeview-expandable[aria-level="2"][aria-expanded="false"] .ui-treeview-expander-column')
			.eq(0).click();
		helper.processToIdle(this.win);
		cy.cGet('#writerchanges .ui-treeview-entry.ui-treeview-expandable[aria-level="2"][aria-expanded="false"] .ui-treeview-expander-column')
			.eq(0).click();
		cy.cGet('#writerchanges').compareSnapshot('writer_manage_changes_tree', 0.1);
		// test sort feature
		cy.cGet('#writerchanges .ui-treeview-header-sort-icon').should('be.not.visible');
		cy.cGet('#writerchanges .ui-treeview-header-button').contains('Author').click();
		cy.cGet('#writerchanges .ui-treeview-header-sort-icon').should('be.visible');
		cy.cGet('#sortbycombobox option:selected').should('have.text', 'Author');
		cy.cGet('#writerchanges').compareSnapshot('writer_manage_changes_tree_sorted', 0.1);
	});
});
