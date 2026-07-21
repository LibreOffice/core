/* -*- js-indent-level: 8 -*- */

/* global describe it cy require beforeEach */
var helper = require('../../common/helper');

describe(['tagdesktop'], 'JSDialog Help button test', function() {

	beforeEach(function() {
		helper.setupAndLoadDocument('calc/help_dialog.ods');
		cy.viewport(1920,1080);
	});

	// Skipped: help is disabled by default (empty HELP_URL), so the Help
	// button is hidden and no external help link popup appears. Revive this
	// once builds are configured with --with-help-url again.
	it.skip('JSDialog Help button opens online help', function() {
		// Open Validation dialog (has a Help button)
		cy.cGet('#Data-tab-label').click();
		cy.cGet('.unoValidation').click();

		// Click the Help button
		cy.cGet('#help').click();

		// The "External link" popup should appear with the help URL
		cy.cGet('#modal-dialog-openlink').should('be.visible');
		cy.cGet('#modal-dialog-openlink').should('contain.text', 'External link');
		cy.cGet('#info-modal-label2').should('contain.text', 'help.collaboraoffice.com');

		// Close it
		cy.cGet('#openlink-response').click();
	});
});
