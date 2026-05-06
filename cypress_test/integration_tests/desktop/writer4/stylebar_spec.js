/* global describe it cy beforeEach require */

var helper = require('../../common/helper');

describe(['tagdesktop'], 'Test style sidebar', function() {

	beforeEach(function() {
		cy.viewport(1920,1080);
		helper.setupAndLoadDocument('writer/stylebar.odt');

		cy.getFrameWindow().then((win) => {
			this.win = win;
		});

		// wait for notebookbar load
		cy.cGet('#stylesview .ui-iconview-entry img').should('exist');

		cy.cGet('#Format-tab-label').click();
		cy.cGet('#toolbar-up [id^="format-style-dialog"] button:visible').click();
		cy.cGet('#StyleListDeck').should('exist').should('be.visible');

		renderEntry('Complimentary Close');

		cy.viewport(1000,660);

		getEntry('Complimentary Close'); // check render exists

		cy.getFrameWindow().then((win) => {
			this.win = win;
			helper.processToIdle(win);
		});
	});

	/// finds rendered entry or text one and scrolls into view to trigger observer action
	function renderEntry(text) {
		cy.cGet('#treeview .ui-treeview-cell-text [textContent="' + text + '"], #treeview img.ui-treeview-custom-render[alt="' + text + '"]',
			{ timeout: 20000 }).should('exist').scrollIntoView();
	}

	/// finds rendered entry
	function getEntry(text) {
		return cy.cGet('#treeview img.ui-treeview-custom-render[alt="' + text + '"]', { timeout: 20000 })
			.should('exist');
	}

	it('Style sidebar updates rendered preview on added style', function() {
		getEntry('Complimentary Close').click();

		helper.processToIdle(this.win); // stabilize
		cy.cGet('#sidebar-dock-wrapper').compareSnapshot('style_initial', 0.07);

		// open context menu and "new" dialog
		getEntry('Complimentary Close').rightclick();
		cy.cGet('#__MENU__').should('exist');
		cy.cGet('#__MENU__ .ui-treeview-cell-text-content').contains('New').click();

		// add new style
		cy.cGet('[id^="TemplateDialog"].jsdialog').should('exist');
		cy.cGet('.button-primary').click();
		cy.cGet('[id^="TemplateDialog"].jsdialog').should('not.exist');
		helper.processToIdle(this.win); // stabilize

		// check image after style was added
		getEntry('Complimentary Close').parent().parent().parent().parent()
			.find('.ui-treeview-expander-column').should('exist').click();

		helper.processToIdle(this.win); // stabilize
		cy.cGet('#sidebar-dock-wrapper').compareSnapshot('style_added', 0.07);
	});

	it('Style sidebar context menu on node with spaces', function() {
		getEntry('Complimentary Close').click();
		getEntry('Complimentary Close').rightclick();

		cy.cGet('#__MENU__').should('exist');

		// visually check position and renders
		helper.processToIdle(this.win);
		cy.cGet('#sidebar-dock-wrapper').compareSnapshot('style_sidebar_context_menu', 0.1);
	});
});
