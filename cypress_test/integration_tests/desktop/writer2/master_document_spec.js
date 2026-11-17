/* -*- js-indent-level: 8 -*- */
/* global describe it cy require */

const desktopHelper = require('../../common/desktop_helper');

describe(['tagdesktop', 'tagnextcloud', 'tagproxy'], 'Master document', { testIsolation: false }, function() {

	desktopHelper.shareDocumentAcrossTests('writer/master_document.odm', {
		notebookbar: true,
		viewport: [1920, 1080],
	});

	it('offers the formats a master document can be saved in', function() {
		cy.cGet('#File-tab-label').click();
		cy.cGet('#File-container .unodownloadas button').click();

		// its own format comes with the formats that hold the content of its linked sections
		cy.cGet('.downloadas-odm-submenu-icon').should('exist');
		cy.cGet('.downloadas-odt-submenu-icon').should('exist');
		cy.cGet('.downloadas-docx-submenu-icon').should('exist');
		cy.cGet('.downloadas-doc-submenu-icon').should('exist');
		cy.cGet('.downloadas-rtf-submenu-icon').should('exist');
		cy.cGet('.exportepub-submenu-icon').should('exist');
		cy.cGet('.downloadas-md-submenu-icon').should('exist');
		cy.cGet('body').type('{esc}');
	});

	it('lists the parts of a master document on a tab of the Navigator', function() {
		// a master document opens with the Navigator
		cy.cGet('#tab-master').should('be.visible');

		// the headings are on the Outline tab, the parts of the document on the Master View tab
		cy.cGet('#tab-navigator').click();
		cy.cGet('#NavigatorPanel #contenttree').should('be.visible');
		cy.cGet('#NavigatorPanel #globaltree').should('not.be.visible');

		cy.cGet('#tab-master').click();
		cy.cGet('#NavigatorPanel #contenttree').should('not.be.visible');
		cy.cGet('#NavigatorPanel #globaltree').should('be.visible');
		cy.cGet('#NavigatorPanel #globaltree').should('contain.text', 'Present');

		// the Update menu of its toolbar opens in the browser, and a choice closes it
		cy.cGet('#NavigatorPanel #global #update .arrowbackground').click();
		cy.cGet('#__MENU___updateall').should('be.visible');
		cy.cGet('#__MENU___updateall').click();
		cy.cGet('#__MENU___updateall').should('not.exist');

		// the update finds the file of a part missing, and the list shows it as a broken link
		cy.cGet('#NavigatorPanel #globaltree .ui-treeview-entry.ui-treeview-entry-textcolor')
			.should('contain.text', 'Missing');

		// so it opens again
		cy.cGet('#NavigatorPanel #global #update .arrowbackground').click();
		cy.cGet('#__MENU___updateall').should('be.visible');
		cy.cGet('body').type('{esc}');
	});
});
