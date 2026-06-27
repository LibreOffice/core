/* global describe it cy require */

var helper = require('../../common/helper');
var desktopHelper = require('../../common/desktop_helper');

describe(['tagdesktop'], 'Iconview Dark Mode Inversion Tests', function() {

	it('Transition options panel should invert in dark mode', function() {
		helper.setupAndLoadDocument('impress/slideshow.odp');
		desktopHelper.switchUIToNotebookbar();

		// Open transition tab
		cy.cGet('#Transition-tab-label').click();
		cy.cGet('#Transition-tab-label').should('have.class', 'selected');

		// Enable dark mode
		cy.cGet('#toggledarktheme').click({force: true});

		// Wait for dark theme to apply
		cy.cGet('body').parent().should('have.attr', 'data-theme', 'dark');

		// Transition icons should be inverted
		cy.cGet('#transitions_icons .ui-iconview-entry img')
			.should('have.css', 'filter', 'invert(1)');

		// Disable dark mode
		cy.cGet('#toggledarktheme').click({force: true});
		cy.cGet('body').parent().should('not.have.attr', 'data-theme', 'dark');

		// Transition icons should not be inverted
		cy.cGet('#transitions_icons .ui-iconview-entry img')
			.should('not.have.css', 'filter', 'invert(1)');
	});

	it('Paragraph Styles panel should NOT invert in dark mode', function() {
		helper.setupAndLoadDocument('writer/styles.odt');
		desktopHelper.switchUIToNotebookbar();

		// Enable dark mode
		cy.cGet('#toggledarktheme').click({force: true});
		cy.cGet('body').parent().should('have.attr', 'data-theme', 'dark');

		// Paragraph styles icons should not be inverted
		cy.cGet('.notebookbar #stylesview .ui-iconview-entry img').should('be.visible');
		
		cy.cGet('.notebookbar #stylesview .ui-iconview-entry img')
			.should('not.have.css', 'filter', 'invert(1)');
	});
});
