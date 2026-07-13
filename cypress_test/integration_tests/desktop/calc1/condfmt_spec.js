/* global describe it require cy beforeEach expect */

var helper = require('../../common/helper');
var desktopHelper = require('../../common/desktop_helper');

describe(['tagdesktop'], 'Conditional Format Dialog Tests', function() {

	beforeEach(function() {
		helper.setupAndLoadDocument('calc/condfmt.ods');
		desktopHelper.switchUIToNotebookbar();
		cy.viewport(1920,1080);
		cy.getFrameWindow().then((win) => {
			this.win = win;
			helper.processToIdle(win);
		});
	});

	it('JSDialog conditional formatting', function() {
		cy.cGet('#toolbar-up #Home .unoConditionalFormatMenu:visible').click();

		desktopHelper.getDropdown('home-conditional-format-menu').click();
		cy.cGet('body').contains('.ui-combobox-entry.jsdialog.ui-grid-cell > span', 'Manage...').click();

		cy.cGet('body').contains('.ui-pushbutton.jsdialog', 'Edit').click();
		cy.cGet('body').contains('.ui-pushbutton.jsdialog', 'Down').should('be.visible');
		cy.cGet('body').contains('.ui-pushbutton.jsdialog', 'Add').click();
		const span = cy.cGet('div.ui-grid.ui-grid-cell')
			.contains('span.jsdialog.static-label.ui-text', 'Condition 5');
		span.should('have.length', 1);
		span.should('have.prop', 'tagName', 'SPAN');
		span.should('be.visible');
		const topDiv = span.parent().parent();
		topDiv.should('have.length', 1);
		topDiv.should('have.prop', 'tagName', 'DIV');
		topDiv.should('be.visible');
		// Second row(div), inside that third column(div), inside that first part(div).
		const input = topDiv.children().eq(1).children().eq(2).children().eq(0).find('input.ui-edit.jsdialog');
		input.should('be.visible');
		input.should('have.length', 1);
		input.type('1331');
		// Without the fix only the first char '1' will be in the input box.
		input.should('have.value', '1331');
	});
});
