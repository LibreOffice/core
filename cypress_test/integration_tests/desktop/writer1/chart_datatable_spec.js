/* global describe it cy beforeEach require */

var helper = require('../../common/helper');
var desktopHelper = require('../../common/desktop_helper');

describe(['tagdesktop', 'tagnextcloud', 'tagproxy'], 'Chart Data Table dialog', function() {

	beforeEach(function() {
		helper.setupAndLoadDocument('writer/chart_wizard.odt');
		desktopHelper.switchUIToNotebookbar();
		desktopHelper.selectZoomLevel('70', false);

		insertChart();
		openDataTableDialog();
	});

	function insertChart() {
		// Insert a default chart via the notebookbar Insert tab.
		// Use getNbIconArrow to expand the overflow dropdown first.
		cy.cGet('.notebookbar #Insert-tab-label').click();
		desktopHelper.getNbIconArrow('InsertGraphic', 'Insert').click();
		cy.cGet('#Insert-container .unoInsertObjectChart button').click();

		// Chart must be in edit mode (shape handles visible).
		cy.cGet('#test-div-shapeHandlesSection').should('exist');

		// The Chart tab should appear in the notebookbar.
		cy.cGet('#Chart-tab-label').should('be.visible');
	}

	function openDataTableDialog() {
		// Open the Data Table dialog via uno command while in chart edit mode.
		cy.getFrameWindow().then(function(win) {
			win.app.map.sendUnoCommand('.uno:DiagramData');
		});
		cy.cGet('.lokdialog_container').should('be.visible');
		cy.cGet('#ChartDataDialog').should('be.visible');
	}

	function closeDataTableDialog() {
		cy.cGet('#ChartDataDialog #close').click();
		cy.cGet('#ChartDataDialog').should('not.exist');
	}

	function getDataGrid() {
		return cy.cGet('#ChartDataDialog #datagrid');
	}

	function getToolbar() {
		return cy.cGet('#ChartDataDialog #toolbar');
	}

	function getEntries() {
		return cy.cGet('#ChartDataDialog #datagrid .ui-treeview-entry');
	}

	function getHeaders() {
		return cy.cGet('#ChartDataDialog #datagrid .ui-treeview-header');
	}

	it('Dialog opens with correct structure', function() {
		// The data grid (treeview) must be present.
		getDataGrid().should('be.visible');

		// Toolbar must be present with all buttons.
		getToolbar().should('be.visible');

		// The default chart has 4 data rows.
		getEntries().should('have.length', 4);

		// Close button must work.
		closeDataTableDialog();
	});

	it('Column headers are present', function() {
		// The default chart has data columns with headers.
		// First column is the row number column, followed by category + data series.
		getHeaders().should('have.length.greaterThan', 1);

		closeDataTableDialog();
	});

	it('Cell selection works', function() {
		// Click on the first data row.
		getEntries().first().click();
		getEntries().first().should('have.class', 'selected');

		// Click on the last data row.
		getEntries().last().click();
		getEntries().last().should('have.class', 'selected');

		closeDataTableDialog();
	});

	it('Insert row', function() {
		// Count initial rows.
		getEntries().should('have.length', 4);

		// Select a row first.
		getEntries().first().click();

		// Click Insert Row toolbar button.
		cy.cGet('#ChartDataDialog #InsertRow').click();

		// Should now have 5 rows.
		getEntries().should('have.length', 5);

		closeDataTableDialog();
	});

	it('Delete row', function() {
		getEntries().should('have.length', 4);

		// Select a row.
		getEntries().first().click();

		// Click Remove Row toolbar button.
		cy.cGet('#ChartDataDialog #RemoveRow').click();

		// Should now have 3 rows.
		getEntries().should('have.length', 3);

		closeDataTableDialog();
	});

	it('Insert series column', function() {
		// Count initial headers.
		getHeaders().then(function($headers) {
			var initialCount = $headers.length;

			// Select a cell in a data column.
			getEntries().first().click();

			// Click Insert Column (Insert Series).
			cy.cGet('#ChartDataDialog #InsertColumn').click();

			// Should now have one more header.
			getHeaders().should('have.length', initialCount + 1);
		});

		closeDataTableDialog();
	});

	it('Delete series column', function() {
		getHeaders().then(function($headers) {
			var initialCount = $headers.length;

			// Select a cell in a data column.
			getEntries().first().click();

			// Click Remove Column (Delete Series).
			cy.cGet('#ChartDataDialog #RemoveColumn').click();

			// Should now have one fewer header.
			getHeaders().should('have.length', initialCount - 1);
		});

		closeDataTableDialog();
	});

	it('Move series left', function() {
		// Get the data value from first row, second data series column (gridcell index 2).
		getEntries().first().find('[role="gridcell"]').eq(2)
			.find('.ui-treeview-cell-text-content').invoke('text').then(function(valueBefore) {
				// Select a cell in that column.
				getEntries().first().find('[role="gridcell"]').eq(2).click();

				// Click Move Left.
				cy.cGet('#ChartDataDialog #MoveLeftColumn').click();

				// The value should now be at position 1 (moved one column left).
				getEntries().first().find('[role="gridcell"]').eq(1)
					.find('.ui-treeview-cell-text-content').should('have.text', valueBefore);
			});

		closeDataTableDialog();
	});

	it('Move series right', function() {
		// Get the data value from first row, first data series column (gridcell index 1).
		getEntries().first().find('[role="gridcell"]').eq(1)
			.find('.ui-treeview-cell-text-content').invoke('text').then(function(valueBefore) {
				// Select a cell in that column.
				getEntries().first().find('[role="gridcell"]').eq(1).click();

				// Click Move Right.
				cy.cGet('#ChartDataDialog #MoveRightColumn').click();

				// The value should now be at position 2 (moved one column right).
				getEntries().first().find('[role="gridcell"]').eq(2)
					.find('.ui-treeview-cell-text-content').should('have.text', valueBefore);
			});

		closeDataTableDialog();
	});

	it('Move row up', function() {
		// Get the Categories cell text from the second row (gridcell index 0 = Categories).
		getEntries().eq(1).find('[role="gridcell"]').eq(0)
			.find('.ui-treeview-cell-text-content').invoke('text').then(function(textBefore) {
				// Select the second row.
				getEntries().eq(1).click();

				// Click Move Up.
				cy.cGet('#ChartDataDialog #MoveUpRow').click();

				// The category text that was in row 1 should now be in row 0.
				getEntries().eq(0).find('[role="gridcell"]').eq(0)
					.find('.ui-treeview-cell-text-content').should('have.text', textBefore);
			});

		closeDataTableDialog();
	});

	it('Move row down', function() {
		// Get the Categories cell text from the first row (gridcell index 0 = Categories).
		getEntries().eq(0).find('[role="gridcell"]').eq(0)
			.find('.ui-treeview-cell-text-content').invoke('text').then(function(textBefore) {
				// Select the first row.
				getEntries().eq(0).click();

				// Click Move Down.
				cy.cGet('#ChartDataDialog #MoveDownRow').click();

				// The category text that was in row 0 should now be in row 1.
				getEntries().eq(1).find('[role="gridcell"]').eq(0)
					.find('.ui-treeview-cell-text-content').should('have.text', textBefore);
			});

		closeDataTableDialog();
	});

	it('Inline cell editing', function() {
		// Click a data cell to select it first, then double-click to edit.
		getEntries().first().find('[role="gridcell"]').eq(1).click();
		getEntries().first().find('[role="gridcell"]').eq(1).dblclick();

		// An input element should appear inside the entry for inline editing.
		getEntries().first().find('input').should('be.visible');

		// Type a new value.
		getEntries().first().find('input').clear().type('42.5{enter}');

		// The input should disappear after pressing enter.
		getEntries().first().find('input').should('not.exist');

		// The cell should now contain the new value.
		getEntries().first().find('[role="gridcell"]').eq(1)
			.find('.ui-treeview-cell-text-content').should('contain', '42.5');

		closeDataTableDialog();
	});

	it('Series color indicators are visible', function() {
		// Each data series header should have a color indicator bar.
		cy.cGet('#ChartDataDialog #datagrid .ui-treeview-header-color').should('have.length.greaterThan', 0);

		// Color indicators should have a background color set.
		cy.cGet('#ChartDataDialog #datagrid .ui-treeview-header-color').first()
			.should('have.css', 'background-color')
			.and('not.eq', 'rgba(0, 0, 0, 0)');

		closeDataTableDialog();
	});

	it('Close button applies changes', function() {
		// Edit a cell value.
		getEntries().first().find('[role="gridcell"]').eq(1).click();
		getEntries().first().find('[role="gridcell"]').eq(1).dblclick();
		getEntries().first().find('input').should('be.visible');
		getEntries().first().find('input').clear().type('99.9{enter}');

		// Close the dialog.
		closeDataTableDialog();

		// Re-open the dialog to verify the change persisted.
		openDataTableDialog();
		getEntries().first().find('[role="gridcell"]').eq(1)
			.find('.ui-treeview-cell-text-content').should('contain', '99.9');

		closeDataTableDialog();
	});
});
