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
		// The style list fetches its preview images on demand. Those
		// round-trips can land after processToIdle returns and swap a
		// text placeholder for an image mid-capture. Wait for every
		// in-flight render_entry to be answered first.
		helper.waitForOnDemandRenders(this.win);
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
		// Expanding the entry rebuilds the treeview and re-requests the
		// preview images. Wait for them before capturing.
		helper.waitForOnDemandRenders(this.win);
		cy.cGet('#sidebar-dock-wrapper').compareSnapshot('style_added', 0.07);
	});

	it('Style sidebar context menu on node with spaces', function() {
		getEntry('Complimentary Close').click();
		getEntry('Complimentary Close').rightclick();

		cy.cGet('#__MENU__').should('exist');

		// visually check position and renders
		helper.processToIdle(this.win);
		helper.waitForOnDemandRenders(this.win);
		cy.cGet('#sidebar-dock-wrapper').compareSnapshot('style_sidebar_context_menu', 0.1);
	});

	// The styles deck was entered from the properties deck, so closing it
	// returns there instead of closing the sidebar.
	it('Close button returns to the properties deck', function() {
		// the panel content is shown without a collapse toggle
		cy.cGet('#StyleListDeck .ui-panel-content').should('be.visible');

		cy.cGet('#StyleListDeck .ui-panel-close-button').click();

		cy.cGet('#PropertyDeck').should('be.visible');
		cy.cGet('#sidebar-dock-wrapper').should('be.visible');
	});

	// The styles deck opened while no sidebar was shown, so closing it
	// returns to the view without a sidebar rather than to another deck.
	it('Close button closes the sidebar', function() {
		// the narrow viewport folds the styles button into the overflow menu,
		// so widen the window to keep it directly clickable
		cy.viewport(1920,1080);

		// the styles deck command toggles, so a second click closes the sidebar
		cy.cGet('#toolbar-up [id^="format-style-dialog"] button:visible').click();
		cy.cGet('#sidebar-dock-wrapper').should('not.be.visible');

		// reopen the styles deck while no sidebar is shown
		cy.cGet('#toolbar-up [id^="format-style-dialog"] button:visible').click();
		cy.cGet('#StyleListDeck').should('be.visible');

		cy.cGet('#StyleListDeck .ui-panel-close-button').click();

		cy.cGet('#sidebar-dock-wrapper').should('not.be.visible');
	});

	// The close button follows the last open, not a stale origin. The styles
	// deck is first opened from the properties deck, closed back to it, then
	// reopened while no sidebar is shown. That second close must close the
	// whole sidebar rather than returning to the properties deck.
	it('Close button follows the last open, not an earlier one', function() {
		cy.viewport(1920,1080);

		// opened from the properties deck, so the close button returns there
		cy.cGet('#StyleListDeck .ui-panel-close-button').click();
		cy.cGet('#PropertyDeck').should('be.visible');

		// close the whole sidebar, then reopen the styles deck from a closed one
		cy.cGet('#toolbar-up [id^="format-style-dialog"] button:visible').click();
		cy.cGet('#StyleListDeck').should('be.visible');
		cy.cGet('#toolbar-up [id^="format-style-dialog"] button:visible').click();
		cy.cGet('#sidebar-dock-wrapper').should('not.be.visible');

		cy.cGet('#toolbar-up [id^="format-style-dialog"] button:visible').click();
		cy.cGet('#StyleListDeck').should('be.visible');

		// opened while no sidebar was shown, so the close button closes it
		cy.cGet('#StyleListDeck .ui-panel-close-button').click();
		cy.cGet('#sidebar-dock-wrapper').should('not.be.visible');
	});
});
