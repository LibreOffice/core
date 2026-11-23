/* -*- js-indent-level: 8 -*- */
/* global describe it cy require expect */

var helper = require('../../common/helper');

// The cells the engine selects in a table on a vector slide are the ones the table handles count on.
describe(['tagdesktop'], 'Cells selected by the engine in a table of a vector slide', function() {

	function geometry(win) {
		return win.app.sectionContainer.getSectionWithName(
			win.app.CSections.RenderGeometry.name).constructor;
	}

	// The table of the slide, a frame of three columns and three rows.
	function theTable(win) {
		var found;
		geometry(win).currentPart().objects.forEach(function(object) {
			if (object.id && object.objectKind === 35)
				found = object;
		});
		return found;
	}

	function selectionSection(win) {
		return win.app.sectionContainer.sections.find(function(section) {
			return section.name.indexOf('selected-object-') === 0;
		});
	}

	it('Delete on a column of a table selected whole deletes the table.', function() {
		cy.viewport(1920, 1080);
		var path = helper.setupDocument('impress/table_operation.odp');
		helper.loadDocument(path, true, undefined, undefined, 'vector=true');
		var win;
		cy.getFrameWindow().then(function(frame) {
			win = frame;
		});
		// The usual load checks wait for tiles, which a vector view does not have.
		cy.wrap(null).should(function() {
			expect(win.app.impress.partList).to.have.length.greaterThan(0);
			expect(theTable(win), 'the table').to.not.equal(undefined);
		});

		cy.then(function() {
			win.eval('GraphicSelection').selectObjects([theTable(win).id]);
		});
		cy.wrap(null).should(function() {
			expect(win.eval('Interaction').context().constructor.name).to.equal('ClassicTableContext');
		});

		// the engine selects all the cells, the client asked for none of them
		cy.then(function() {
			win.app.map.sendUnoCommand('.uno:SelectTable');
		});
		cy.wrap(null).should(function() {
			var area = win.app.map._docLayer._cellSelectionArea;
			var table = theTable(win);
			expect(area, 'the cells selected').to.exist;
			expect(area.x1).to.be.closeTo(table.transform[4], 20);
			expect(area.width).to.be.closeTo(table.transform[0], 20);
		});

		// with every column selected, Delete on one of them takes all, and the table with them
		cy.then(function() {
			selectionSection(win).travel.goTo('table.columnSelect.1');
		});
		cy.realPress('Delete');
		cy.wrap(null).should(function() {
			expect(theTable(win), 'the table').to.equal(undefined);
		});
	});
});
