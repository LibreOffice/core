/* global describe  cy beforeEach it expect require */

var helper = require('../../common/helper');
var desktopHelper = require('../../common/desktop_helper');
var impressHelper = require('../../common/impress_helper');

describe(['tagdesktop'], 'Table operations', function() {

	beforeEach(function() {
		helper.setupAndLoadDocument('impress/table_operation.odp');
		cy.viewport(1920,1080);

		desktopHelper.switchUIToNotebookbar();

		cy.getFrameWindow().then(function(win) {
			this.win = win;
		});
	});

	function retriggerNewSvgForTableInTheCenter() {
		impressHelper.removeShapeSelection();

		helper.typeIntoDocument('{ctrl}{a}');
	}

	function selectFullTable(win) {
		impressHelper.selectTableInTheCenter(win);

		cy.cGet('.table-row-resize-marker')
			.should('have.length', 3);
		cy.cGet('.table-column-resize-marker')
			.should('have.length', 3); // One is invisible but it is included here.
	}

	it('Insert Row Before', function() {
		selectFullTable(this.win);
		desktopHelper.getNbIcon('InsertRowsBefore', 'Table').click();
		cy.cGet('.table-row-resize-marker').should('have.length', 4);
		retriggerNewSvgForTableInTheCenter();
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
		retriggerNewSvgForTableInTheCenter();

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

		retriggerNewSvgForTableInTheCenter();

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

		retriggerNewSvgForTableInTheCenter();

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

		retriggerNewSvgForTableInTheCenter();

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

		retriggerNewSvgForTableInTheCenter();

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

		retriggerNewSvgForTableInTheCenter();

		cy.cGet('.table-column-resize-marker')
			.should('not.exist');

		cy.cGet('#document-container g.Page g')
			.should('not.exist');
	});

	it('Merge Row', function() {
		selectFullTable(this.win);

		cy.cGet('.table-row-resize-marker')
			.should('have.length', 3);

		desktopHelper.getNbIcon('EntireRow', 'Table').click();
		desktopHelper.getNbIcon('MergeCells', 'Table').should('not.be.disabled').click();

		retriggerNewSvgForTableInTheCenter();

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
		selectFullTable(this.win);

		cy.cGet('.table-row-resize-marker')
			.should('have.length', 3);

		desktopHelper.getNbIcon('EntireColumn', 'Table').click();
		desktopHelper.getNbIcon('MergeCells', 'Table').should('not.be.disabled').click();

		retriggerNewSvgForTableInTheCenter();

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
