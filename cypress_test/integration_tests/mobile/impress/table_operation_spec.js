/* global describe it cy beforeEach expect require */

var helper = require('../../common/helper');
var mobileHelper = require('../../common/mobile_helper');
var impressHelper = require('../../common/impress_helper');

describe.skip(['tagmobile', 'tagnextcloud', 'tagproxy'], 'Table Operation', function() {

	beforeEach(function() {
		helper.setupAndLoadDocument('impress/table_operation.odp');

		mobileHelper.enableEditingMobile();

		cy.getFrameWindow().then(function(win) {
			this.win = win;
		});
	});

	function retriggerNewSvgForTableInTheCenter() {
		impressHelper.removeShapeSelection();

		helper.typeIntoDocument('{ctrl}{a}');
	}

	function selectFullTable() {
		helper.typeIntoDocument('{ctrl}{a}');

		impressHelper.selectTableInTheCenter(this.win);

		cy.cGet('.table-row-resize-marker').should('have.length', 3);
		cy.cGet('.table-column-resize-marker').should('have.length', 2);
		cy.cGet('text.SVGTextShape').click({force: true});
		cy.wait(1000);
	}

	function clickOnTableOperation(operation) {
		mobileHelper.openHamburgerMenu();
		cy.cGet('.menu-entry-icon.tablemenu').parent().click();
		cy.cGet('.menu-entry-icon.' + operation).parent().click();
	}

	it('Insert Before', function() {
		selectFullTable();

		clickOnTableOperation('insertrowsbefore');

		cy.cGet('.table-row-resize-marker')
			.should('have.length', 4);

		retriggerNewSvgForTableInTheCenter();

		cy.cGet('#document-container g.Page g')
			.should('have.class', 'com.sun.star.drawing.TableShape');

		//assert the number of cells
		cy.cGet('g.Page path[fill^="rgb"]')
			.should(function(cells) {
				expect(cells).to.have.lengthOf(8);
			});

		//assert the text position
		cy.cGet('#document-container g.Page .TextParagraph .TextPosition')
			.should('have.attr', 'x', '7290');

		cy.cGet('#document-container g.Page .TextParagraph .TextPosition')
			.should('have.attr', 'y', '6643');
	});


	it('Insert Row After', function() {
		selectFullTable();

		clickOnTableOperation('insertrowsafter');

		cy.cGet('.table-row-resize-marker')
			.should('have.length', 4);

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

	it.skip('Insert column before.', function() {
		selectFullTable();

		helper.typeIntoDocument('{downarrow}');

		clickOnTableOperation('deleterows');

		cy.cGet('.table-row-resize-marker')
			.should('have.length', 2);

		clickOnTableOperation('insertcolumnsbefore');

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
			.should('have.attr', 'x', '14339');

		cy.cGet('#document-container g.Page .TextParagraph .TextPosition')
			.should('have.attr', 'y', '5597');
	});

	it('Insert column after.', function() {
		selectFullTable();

		helper.typeIntoDocument('{downarrow}');

		clickOnTableOperation('deleterows');

		cy.cGet('.table-row-resize-marker').should('have.length', 2);

		clickOnTableOperation('insertcolumnsafter');

		cy.cGet('.table-column-resize-marker').should('have.length', 3);

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

	it('Delete row.', function() {
		selectFullTable();

		clickOnTableOperation('deleterows');

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

		cy.cGet('#document-container g.Page .TextParagraph .TextPosition')
			.should('not.exist');
	});

	it('Delete Column.', function() {
		selectFullTable();

		clickOnTableOperation('insertcolumnsbefore');

		cy.cGet('.table-column-resize-marker')
			.should('have.length', 3);

		clickOnTableOperation('deletecolumns');

		cy.cGet('.table-column-resize-marker')
			.should('have.length', 2);

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

	it('Merge Row', function() {
		selectFullTable();

		cy.cGet('.table-row-resize-marker')
			.should('have.length', 3);

		clickOnTableOperation('entirerow');

		cy.cGet('.leaflet-control-buttons-disabled svg').should('exist');

		clickOnTableOperation('mergecells');

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
		selectFullTable();

		cy.cGet('.table-row-resize-marker')
			.should('have.length', 3);

		clickOnTableOperation('entirecolumn');

		cy.cGet('.leaflet-control-buttons-disabled svg').should('exist');

		clickOnTableOperation('mergecells');

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

	it('Delete Table', function() {
		selectFullTable();

		clickOnTableOperation('deletetable');

		retriggerNewSvgForTableInTheCenter();

		cy.cGet('.table-column-resize-marker')
			.should('not.exist');

		cy.cGet('#document-container g.Page g')
			.should('not.exist');
	});
	//TODO: add split cells tests
	//bug: https://github.com/CollaboraOnline/online/issues/3962
});
