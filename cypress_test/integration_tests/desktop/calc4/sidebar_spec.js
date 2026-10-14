/* global describe it cy beforeEach require */

var helper = require('../../common/helper');

describe(['tagdesktop'], 'Sidebar tests', function() {

	beforeEach(function() {
		helper.setupAndLoadDocument('calc/cell_cursor.ods');
		cy.viewport(1920,1080);
		cy.getFrameWindow().then(function(win) {
			this.win = win;
		}.bind(this));
	});

	it('Style panel more button switches to the styles deck', function() {
		cy.cGet('#StyleListDeck').should('not.exist');

		cy.cGet('.StylesPropertyPanel .ui-expander-icon-right button').click();

		cy.cGet('#StyleListDeck').should('be.visible');
	});

	// The styles deck lists the cell styles with a rendered preview per entry.
	// Right-clicking a style opens a menu that offers only the actions that
	// apply to it. "Default" is in use, not hidden and not user defined, so the
	// menu holds New and Edit Style and nothing else.
	it('Style context menu lists only the actions that apply', function() {
		cy.cGet('.StylesPropertyPanel .ui-expander-icon-right button').click();
		cy.cGet('#StyleListDeck').should('be.visible');

		// the style list fetches its preview images on demand after the deck opens
		helper.processToIdle(this.win);
		helper.waitForOnDemandRenders(this.win);

		// the entry starts as a text placeholder and becomes an image once rendered
		cy.cGet('#treeview .ui-treeview-cell-text [textContent="Default"], #treeview img.ui-treeview-custom-render[alt="Default"]')
			.should('exist').scrollIntoView();
		cy.cGet('#treeview img.ui-treeview-custom-render[alt="Default"]').should('exist').rightclick();

		cy.cGet('#__MENU__').should('exist');
		cy.cGet('#__MENU__').contains('.ui-treeview-cell-text-content', 'New').should('exist');
		cy.cGet('#__MENU__').contains('.ui-treeview-cell-text-content', 'Edit Style').should('exist');
		cy.cGet('#__MENU__ .ui-treeview-entry').should('have.length', 2);
		cy.cGet('#__MENU__ .ui-treeview-entry.disabled').should('not.exist');
	});
});
