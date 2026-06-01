/* global describe cy beforeEach it require */

var helper = require('../../common/helper');
var impressHelper = require('../../common/impress_helper');

// The table row/column operations are also reachable from the notebookbar
// (see table_operation_spec.js). These tests cover the right-click context
// menu path, which is gated by the presentation allowlist in
// Definitions.MenuCommands.ts.
describe(['tagdesktop'], 'Table context menu operations', function() {

	beforeEach(function() {
		helper.setupAndLoadDocument('impress/table_operation.odp');
		cy.viewport(1920, 1080);

		cy.getFrameWindow().then(function(win) {
			this.win = win;
		});
	});

	function selectFullTable(win) {
		impressHelper.selectTableInTheCenter(win);

		cy.cGet('.table-row-resize-marker').should('have.length', 3);
		cy.cGet('.table-column-resize-marker').should('have.length', 3); // One is invisible but it is included here.
	}

	// Right-click in the table cell at the center of the slide to open the
	// table context menu.
	function openTableContextMenu() {
		cy.cGet('#document-container').then(function(items) {
			var rect = items[0].getBoundingClientRect();
			var x = (rect.left + rect.right) / 2;
			var y = (rect.top + rect.bottom) / 2;
			cy.cGet('body').rightclick(x, y);
		});

		cy.cGet('#jsd-context-menu-dropdown-overlay').should('be.visible');
	}

	// Open a context-menu submenu by its label and return the requested entry
	// from the submenu dropdown that pops up. The submenu opens on hover (see
	// the mouseover handler in Widget.Combobox.js) as a separate dropdown
	// overlay whose id is derived from the parent menu id. We match on the
	// ui-has-menu class so we target the submenu entry rather than a leaf entry
	// that happens to share the label (e.g. the injected ".uno:Delete").
	function contextSubMenuItem(submenuText, itemText) {
		cy.cGet('#jsd-context-menu-dropdown-overlay .ui-combobox-entry.ui-has-menu')
			.contains('span', submenuText)
			.trigger('mouseover');
		return cy.cGet('[id^="jsd-context-menu-"][id$="-dropdown-overlay"]')
			.contains('.ui-combobox-entry.jsdialog.ui-grid-cell span', itemText);
	}

	it('Context menu offers Insert and Delete submenus', function() {
		selectFullTable(this.win);
		openTableContextMenu();

		cy.cGet('#jsd-context-menu-dropdown-overlay .ui-combobox-entry.ui-has-menu')
			.contains('span', 'Insert').should('exist');
		cy.cGet('#jsd-context-menu-dropdown-overlay .ui-combobox-entry.ui-has-menu')
			.contains('span', 'Delete').should('exist');
	});

	it('Insert Row Above', function() {
		selectFullTable(this.win);
		openTableContextMenu();

		contextSubMenuItem('Insert', 'Insert Row Above').click();

		cy.cGet('.table-row-resize-marker').should('have.length', 4);
	});

	it('Insert Column Before', function() {
		selectFullTable(this.win);
		openTableContextMenu();

		contextSubMenuItem('Insert', 'Insert Column Before').click();

		cy.cGet('.table-column-resize-marker').should('have.length', 4);
	});

	it('Delete Row', function() {
		selectFullTable(this.win);
		openTableContextMenu();

		contextSubMenuItem('Delete', 'Delete Row').click();

		cy.cGet('.table-row-resize-marker').should('have.length', 2);
	});

	it('Delete Column', function() {
		selectFullTable(this.win);

		// Add a column first so there is still a column left to assert on after
		// the deletion.
		openTableContextMenu();
		contextSubMenuItem('Insert', 'Insert Column Before').click();
		cy.cGet('.table-column-resize-marker').should('have.length', 4);

		openTableContextMenu();
		contextSubMenuItem('Delete', 'Delete Column').click();
		cy.cGet('.table-column-resize-marker').should('have.length', 3);
	});
});
