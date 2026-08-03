/* global describe  cy beforeEach it expect require */

var helper = require('../../common/helper');
var desktopHelper = require('../../common/desktop_helper');
var impressHelper = require('../../common/impress_helper');

describe(['tagdesktop'], 'Table operations', { testIsolation: false }, function() {

	desktopHelper.shareDocumentAcrossTests('impress/table_operation.odp', {
		notebookbar: true,
		viewport: [1920, 1080],
	});

	beforeEach(function() {
		cy.getFrameWindow().then(function(win) {
			this.win = win;
		});
	});

	function reselectWholeTable() {
		impressHelper.removeShapeSelection();

		helper.typeIntoDocument('{ctrl}{a}');

		cy.getFrameWindow().then(function(win) {
			helper.processToIdle(win);
		});
	}

	function retriggerNewSvgForTableInTheCenter(expectedCellCount) {
		// Reselect until the cell SVG shows the expected count, as a single
		// reselect can race the render and leave the old count on screen.
		helper.retryUntil(
			reselectWholeTable,
			function() {
				return cy.cGet('#document-container').then(function(overlay) {
					return overlay.find('g.Page path[fill^="rgb"]').length === expectedCellCount;
				});
			},
			{ errorMsg: 'table SVG never showed ' + expectedCellCount + ' cells' });
	}

	function selectFullTable(win) {
		// The two clicks below mean "select the table, then enter it", so they need a
		// slide with nothing selected: from a table that is already selected they land
		// one level deeper and no cell cursor appears.
		impressHelper.removeShapeSelection();

		impressHelper.selectTableInTheCenter(win);

		cy.cGet('.table-row-resize-marker')
			.should('have.length', 3);
		cy.cGet('.table-column-resize-marker')
			.should('have.length', 3); // One is invisible but it is included here.
	}

	// Click a Table tab icon while the item holding it is enabled, which is the
	// state that decides whether the click is acted on. An item that reads disabled
	// means core has no cell selection to work on, and the table selection at the
	// start of the next round is what brings it back.
	function clickTableIconWhenEnabled(unoCommand) {
		return desktopHelper.getNbItem(unoCommand, 'Table').then(function($item) {
			if ($item.attr('disabled'))
				return;

			desktopHelper.getNbIcon(unoCommand, 'Table').click();
		});
	}

	// Select a whole row or column of the table in the centre and merge it into one
	// cell.
	//
	// A merge shows up in the shape SVG only after the table is deselected and
	// selected again, so core's undo depth is what says the merge arrived. When it
	// did not arrive, the sequence is repeated from the table selection, because a
	// click that was not acted on leaves nothing else to wait for.
	function mergeEntireRowOrColumn(win, selectionCommand) {
		var stepsBefore = 0;

		desktopHelper.getUndoCount(win).then(function(count) {
			stepsBefore = count;
		});

		helper.retryUntil(
			function() {
				selectFullTable(win);

				clickTableIconWhenEnabled(selectionCommand);
				helper.processToIdle(win);

				clickTableIconWhenEnabled('MergeCells');
				helper.processToIdle(win);
			},
			function() {
				return desktopHelper.getUndoCount(win).then(function(count) {
					return count > stepsBefore;
				});
			},
			{ errorMsg: selectionCommand + ' and MergeCells never reached the undo stack' });
	}

	it('Insert Row Before', function() {
		selectFullTable(this.win);
		desktopHelper.getNbIcon('InsertRowsBefore', 'Table').click();
		cy.cGet('.table-row-resize-marker').should('have.length', 4);
		retriggerNewSvgForTableInTheCenter(8);
		cy.cGet('#document-container g.Page g').should('have.class', 'com.sun.star.drawing.TableShape');

		//assert the number of cells
		cy.cGet('g.Page path[fill^="rgb"]')
			.should(function(cells) {
				expect(cells).to.have.lengthOf(8);
			});

		//assert the text position
		cy.cGet('#document-container g.Page .TextParagraph .TextPosition').should('have.attr', 'x', '7290');

		cy.cGet('#document-container g.Page .TextParagraph .TextPosition').should('have.attr', 'y', '5597');
	});

	it('Insert Row After', function() {
		selectFullTable(this.win);
		desktopHelper.getNbIcon('InsertRowsAfter', 'Table').click();

		cy.cGet('.table-row-resize-marker').should('have.length', 4);
		retriggerNewSvgForTableInTheCenter(8);

		cy.cGet('#document-container g.Page g')
			.should('have.class', 'com.sun.star.drawing.TableShape');

		//assert the number of cells
		cy.cGet('g.Page path[fill^="rgb"]')
			.should(function(cells) {
				expect(cells).to.have.lengthOf(8);
			});

		cy.cGet('#document-container g.Page .TextParagraph .TextPosition')
			.should('have.attr', 'x', '7290');

		cy.cGet('#document-container g.Page .TextParagraph .TextPosition')
			.should('have.attr', 'y', '5597');
	});

	it('Insert column before.', function() {
		selectFullTable(this.win);
		desktopHelper.getNbIcon('InsertColumnsBefore', 'Table').click();

		cy.cGet('.table-column-resize-marker')
			.should('have.length', 4);

		retriggerNewSvgForTableInTheCenter(9);

		cy.cGet('#document-container g.Page g')
			.should('have.class', 'com.sun.star.drawing.TableShape');

		//assert the number of cells
		cy.cGet('g.Page path[fill^="rgb"]')
			.should(function(cells) {
				expect(cells).to.have.lengthOf(9);
			});

		cy.cGet('#document-container g.Page .TextParagraph .TextPosition')
			.should('have.attr', 'x', '14339');

		cy.cGet('#document-container g.Page .TextParagraph .TextPosition')
			.should('have.attr', 'y', '5597');
	});

	it('Insert column after.', function() {
		selectFullTable(this.win);
		desktopHelper.getNbIcon('InsertColumnsAfter', 'Table').click();

		cy.cGet('.table-column-resize-marker')
			.should('have.length', 4);

		retriggerNewSvgForTableInTheCenter(9);

		cy.cGet('#document-container g.Page g')
			.should('have.class', 'com.sun.star.drawing.TableShape');

		//assert the number of cells
		cy.cGet('g.Page path[fill^="rgb"]')
			.should(function(cells) {
				expect(cells).to.have.lengthOf(9);
			});

		cy.cGet('#document-container g.Page .TextParagraph .TextPosition')
			.should('have.attr', 'x', '7290');

		cy.cGet('#document-container g.Page .TextParagraph .TextPosition')
			.should('have.attr', 'y', '5597');
	});

	it('Delete row.', function() {
		selectFullTable(this.win);
		desktopHelper.getNbIcon('DeleteRows', 'Table').click();

		cy.cGet('.table-row-resize-marker')
			.should('have.length', 2);

		retriggerNewSvgForTableInTheCenter(4);

		cy.cGet('#document-container g.Page g')
			.should('have.class', 'com.sun.star.drawing.TableShape');

		//assert the number of cells
		cy.cGet('g.Page path[fill^="rgb"]')
			.should(function(cells) {
				expect(cells).to.have.lengthOf(4);
			});

		//cy.cGet('#document-container g.Page .TextParagraph .TextPosition').should('not.exist');
	});

	it('Delete Column.', function() {
		selectFullTable(this.win);
		desktopHelper.getNbIcon('InsertColumnsBefore', 'Table').click();

		cy.cGet('.table-column-resize-marker')
			.should('have.length', 4);

		desktopHelper.getNbIcon('DeleteColumns', 'Table').click();

		cy.cGet('.table-column-resize-marker')
			.should('have.length', 3);

		retriggerNewSvgForTableInTheCenter(6);

		cy.cGet('#document-container g.Page g')
			.should('have.class', 'com.sun.star.drawing.TableShape');

		//assert the number of cells
		cy.cGet('g.Page path[fill^="rgb"]')
			.should(function(cells) {
				expect(cells).to.have.lengthOf(6);
			});

		cy.cGet('#document-container g.Page .TextParagraph .TextPosition')
			.should('have.attr', 'x', '7290');

		cy.cGet('#document-container g.Page .TextParagraph .TextPosition')
			.should('have.attr', 'y', '5597');
	});

	it('Delete Table', function() {
		selectFullTable(this.win);
		desktopHelper.getNbIcon('DeleteTable', 'Table').click();

		retriggerNewSvgForTableInTheCenter(0);

		cy.cGet('.table-column-resize-marker')
			.should('not.exist');

		cy.cGet('#document-container g.Page g')
			.should('not.exist');
	});

	it('Merge Row', function() {
		mergeEntireRowOrColumn(this.win, 'EntireRow');

		retriggerNewSvgForTableInTheCenter(5);

		cy.cGet('#document-container g.Page g')
			.should('have.class', 'com.sun.star.drawing.TableShape');

		//assert the number of cells
		cy.cGet('g.Page path[fill^="rgb"]')
			.should(function(cells) {
				expect(cells).to.have.lengthOf(5);
			});

		cy.cGet('#document-container g.Page .TextParagraph .TextPosition')
			.should('have.attr', 'x', '7290');

		cy.cGet('#document-container g.Page .TextParagraph .TextPosition')
			.should('have.attr', 'y', '5597');
	});

	it('Merge Column', function() {
		mergeEntireRowOrColumn(this.win, 'EntireColumn');

		retriggerNewSvgForTableInTheCenter(4);

		cy.cGet('#document-container g.Page g')
			.should('have.class', 'com.sun.star.drawing.TableShape');

		//assert the number of cells
		cy.cGet('g.Page path[fill^="rgb"]')
			.should(function(cells) {
				expect(cells).to.have.lengthOf(4);
			});

		cy.cGet('#document-container g.Page .TextParagraph .TextPosition')
			.should('have.attr', 'x', '7290');

		cy.cGet('#document-container g.Page .TextParagraph .TextPosition')
			.should('have.attr', 'y', '5597');
	});

	it.skip('Split Cells', function() {
		// ToDo: Merge cells before calling split cells function.
		impressHelper.selectTableInTheCenter(this.win);

		cy.cGet('.table-row-resize-marker')
			.should('have.length', 4);

		desktopHelper.getNbIcon('SplitCell', 'Table').click();

		cy.cGet('#SplitCellsDialog').should('be.visible');

		cy.cGet('#SplitCellsDialog .ui-pushbutton.jsdialog.button-primary')
			.click();

		cy.cGet('.table-row-resize-marker')
			.should('have.length', 4);
	});
});
