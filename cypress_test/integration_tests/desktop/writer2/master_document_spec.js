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
	});
});
