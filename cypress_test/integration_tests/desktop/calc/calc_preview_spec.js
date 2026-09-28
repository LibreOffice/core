/* -*- js-indent-level: 8 -*- */
/* global describe it cy beforeEach require */

let helper = require('../../common/helper');

describe(['tagdesktop', 'tagpreview'], 'Calc preview', function() {
	beforeEach(function() {
		helper.setupAndPreviewDocument('calc/testfile.xlsx');
	});

	it('shows the preview of the spreadsheet', function() {
		cy.cGet('#navigation-sidebar').should('not.be.visible');
		cy.cGet('#notespanel-dock-wrapper').should('not.be.visible');
		cy.cGet('#sidebar-dock-wrapper').should('not.be.visible');
		cy.cGet('#aichat-dock-wrapper').should('not.be.visible');

		// check the formulabar doesn't exist.
		// caveat: if the selector changes we don't detect it.
		const formulaBar = '#sc_input_window .ui-custom-textarea-text-layer';
		cy.cGet(formulaBar).should('not.exist');
	});
});
