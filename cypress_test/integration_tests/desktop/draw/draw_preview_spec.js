/* -*- js-indent-level: 8 -*- */
/* global describe it cy beforeEach require */

let helper = require('../../common/helper');

describe(['tagdesktop', 'tagpreview'], 'Drawing preview', function() {
	beforeEach(function() {
		helper.setupAndPreviewDocument('draw/navigator.odg');
	});

	it('shows the preview of the drawing', function() {
		cy.cGet('#navigation-sidebar').should('not.be.visible');
		cy.cGet('#notespanel-dock-wrapper').should('not.be.visible');
		cy.cGet('#sidebar-dock-wrapper').should('not.be.visible');
		cy.cGet('#aichat-dock-wrapper').should('not.be.visible');
	});
});
