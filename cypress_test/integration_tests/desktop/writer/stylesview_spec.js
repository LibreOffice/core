/* global describe it cy require beforeEach */

var helper = require('../../common/helper');
var desktopHelper = require('../../common/desktop_helper');

describe(['tagdesktop'], 'Stylesview Iconview Tests', function() {
	// check expand button visibility and click on it
	const openExpander = () => {
		cy.cGet('#stylesview-iconview-list-expand-button').should('exist').should('be.visible');
		cy.cGet('#stylesview-iconview-list-expand-button').click();
		desktopHelper.getDropdown('stylesview-iconview-list').should('exist');
		cy.cGet('.jsdialog #stylesview').should('exist').should('be.visible');
	}

	beforeEach(function() {
		cy.viewport(1920, 1080);
		helper.setupAndLoadDocument('writer/styles.odt');
		desktopHelper.switchUIToNotebookbar();
		desktopHelper.sidebarToggle();
		cy.cGet('.notebookbar #stylesview').should('exist').should('be.visible');
	});

	it('Scroll Up/Down Buttons', function() {
		cy.cGet('#stylesview-iconview-list-scroll-up').should('exist').should('be.visible');
		cy.cGet('#stylesview-iconview-list-scroll-down').should('exist').should('be.visible');
		cy.cGet('#stylesview_0').should('exist').should('be.visible');

		cy.cGet('#stylesview-iconview-list-scroll-down').click();
		cy.cGet('#stylesview_0').should('exist').should('not.be.visible');
		cy.cGet('#stylesview-iconview-list-scroll-up').click();
		cy.cGet('#stylesview_0').should('exist').should('be.visible');
	});

	it('Expander Button', function() {
		openExpander();
		cy.cGet('.jsdialog #stylesview_59').should('exist').should('be.visible'); // Contents 9
		cy.cGet('.jsdialog #stylesview_60').should('exist').should('not.be.visible');
	});

	it('Open Styles Sidebar Button', function() {
		openExpander();

		// open sidebar
		cy.cGet('#format-style-list-dialog-button').should('exist').should('be.visible');
		cy.cGet('#format-style-list-dialog-button').click();

		// close dropdown on button click
		desktopHelper.getDropdown('stylesview').should('not.exist');

		cy.cGet('#StyleListDeck').should('exist').should('be.visible');
	});

	it.only('Resize', function() {
		// the dropdown should close on resize
		openExpander();
		cy.cGet('#stylesview-iconview-list-dropdown').should('exist').should('be.visible');
		cy.viewport(650, 1080);
		cy.cGet('#stylesview-iconview-list-dropdown').should('not.exist');

		// re-open after the resize
		openExpander();

		// with reduced height:
		// - only a few rows should be visible.
		// - the open styles sidebar button should be visible.
		cy.viewport(650, 454);
		cy.cGet('.jsdialog #stylesview_4').should('exist').should('be.visible');
		cy.cGet('.jsdialog #stylesview_5').should('exist').should('not.be.visible');
		cy.cGet('#format-style-list-dialog-button').should('exist').should('be.visible');
	});
});
