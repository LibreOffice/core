/* -*- js-indent-level: 8 -*- */
/* global describe it require cy beforeEach expect */

var helper = require('../../common/helper');

describe(['tagdesktop'], 'Calc outline group controls when opening scrolled', function() {

	beforeEach(function() {
		// group_outline_scrolled.fods has a column group (columns 31 to 33) and
		// a row group (rows 100 to 104) placed away from the top-left corner,
		// and is saved scrolled so both groups sit in the opening view. On load
		// the geometry first arrives for the top of the sheet, then the view
		// jumps to the saved position; the group controls must appear there
		// without any edit.
		helper.setupAndLoadDocument('calc/group_outline_scrolled.fods');
		cy.getFrameWindow().then(function(win) {
			helper.processToIdle(win);
		});
	});

	// Number of group entries the section has collected for the current view.
	function collectedGroupCount(section) {
		if (!section || !section._groups)
			return 0;

		return section._groups.reduce(function(total, level) {
			if (!level)
				return total;
			return total + Object.keys(level).length;
		}, 0);
	}

	it('Draws the row and column group controls without an edit', function() {
		cy.getFrameWindow().should(function(win) {
			var rowGroup = win.app.sectionContainer.getSectionWithName(
				win.app.CSections.RowGroup.name);
			var columnGroup = win.app.sectionContainer.getSectionWithName(
				win.app.CSections.ColumnGroup.name);

			expect(rowGroup, 'row group section is present').to.exist;
			expect(columnGroup, 'column group section is present').to.exist;

			expect(collectedGroupCount(rowGroup),
				'row groups collected for the opening view').to.be.greaterThan(0);
			expect(collectedGroupCount(columnGroup),
				'column groups collected for the opening view').to.be.greaterThan(0);
		});
	});

	it('Drops the group controls after scrolling to the top', function() {
		// Scrolling to the origin is a view change with no geometry message.
		// The groups then lie past the visible range, so the collection must
		// empty out on the scroll alone rather than keeping the stale entries.
		cy.getFrameWindow().then(function(win) {
			win.app.activeDocument.activeLayout.scrollTo(0, 0);
			helper.processToIdle(win);
		});

		cy.getFrameWindow().should(function(win) {
			var rowGroup = win.app.sectionContainer.getSectionWithName(
				win.app.CSections.RowGroup.name);
			var columnGroup = win.app.sectionContainer.getSectionWithName(
				win.app.CSections.ColumnGroup.name);

			expect(collectedGroupCount(rowGroup),
				'row groups cleared at the top of the sheet').to.equal(0);
			expect(collectedGroupCount(columnGroup),
				'column groups cleared at the top of the sheet').to.equal(0);
		});
	});
});
