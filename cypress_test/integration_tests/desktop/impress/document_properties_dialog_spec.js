/* global describe it cy beforeEach expect require */

var helper = require('../../common/helper');

describe(['tagdesktop', 'tagnextcloud', 'tagproxy'], 'Document Properties dialog tests', function() {
	beforeEach(function() {
		helper.setupAndLoadDocument('impress/help_dialog.odp');

		cy.getFrameWindow().then((win) => {
			this.win = win;
		});

		cy.then(() => {
			return helper.processToIdle(this.win);
		});

		cy.then(() => {
			this.win.app.map.sendUnoCommand('.uno:SetDocumentProperties');
		});

		cy.cGet('.jsdialog-window').should('exist');

		cy.then(() => {
			return helper.processToIdle(this.win);
		});
	});

	/*
	 * The resolution box sits in a narrow column, next to a long label and the "ppi"
	 * unit, so it ends up at its minimum width. A combobox spends part of its width
	 * on the dropdown button, and the resolution has to stay readable in what is left.
	 */
	it('shows the picked image resolution in full', function() {
		cy.cGet('#image-preferred-dpi-checkbutton input').click();

		cy.cGet('#image-preferred-dpi-combobox .ui-combobox-button').click();
		cy.cGet('.ui-combobox-entry').contains('600').click();

		cy.cGet('#image-preferred-dpi-combobox input')
			.should('have.value', '600')
			.should(function (field) {
				// The field scrolls only when the text does not fit into it.
				expect(field[0].scrollWidth).to.be.at.most(field[0].clientWidth);
			});
	});
});
