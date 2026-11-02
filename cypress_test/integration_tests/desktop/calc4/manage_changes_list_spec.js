/* global describe it cy beforeEach require expect */

var helper = require('../../common/helper');

describe(['tagdesktop'], 'Manage Changes list', function () {

	beforeEach(function () {
		helper.setupAndLoadDocument('calc/empty-selections.ods');
		cy.getFrameWindow().then(function (win) {
			this.win = win;
		}.bind(this));
	});

	function recordTenChanges(win) {
		cy.getFrameWindow().then(function (frameWindow) {
			frameWindow.app.map.sendUnoCommand('.uno:TraceChangeMode');
		});
		helper.processToIdle(win);
		helper.waitForMapState('.uno:TraceChangeMode', 'true');

		helper.typeIntoInputField(helper.addressInputSelector, 'A1');
		var text = '';
		for (var i = 1; i <= 10; i++)
			text += 'v' + i + '{enter}';
		helper.typeIntoDocument(text);
		helper.processToIdle(win);
	}

	function openManageChanges(win) {
		cy.getFrameWindow().then(function (frameWindow) {
			frameWindow.app.map.sendUnoCommand('.uno:AcceptChanges');
		});
		cy.cGet('#calcchanges').should('exist');
		helper.processToIdle(win);
	}

	it('the list shows the rows the dialog asks for, under their headings', function () {
		recordTenChanges(this.win);
		openManageChanges(this.win);
		cy.cGet('#calcchanges > .ui-treeview-entry').should('have.length', 10);

		// The dialog asks for eight rows, so eight changes are visible without scrolling.
		cy.cGet('#calcchanges').should(function (list) {
			var listRect = list[0].getBoundingClientRect();
			var whole = Array.from(list[0].querySelectorAll(':scope > .ui-treeview-entry'))
				.filter(function (row) {
					var rowRect = row.getBoundingClientRect();
					return rowRect.top >= listRect.top && rowRect.bottom <= listRect.bottom + 1;
				});
			expect(whole.length).to.be.at.least(8);
		});

		// Each heading starts where the cells of its column start.
		cy.cGet('#calcchanges').should(function (list) {
			var headings = list[0].querySelector(':scope > .ui-treeview-headers').children;
			var cells = list[0].querySelector(':scope > .ui-treeview-entry').children;
			expect(headings.length).to.equal(cells.length);
			for (var i = 0; i < headings.length; i++) {
				var headingLeft = headings[i].getBoundingClientRect().left;
				var cellLeft = cells[i].getBoundingClientRect().left;
				expect(Math.abs(headingLeft - cellLeft), 'column ' + i).to.be.below(2);
			}
		});
	});
});
