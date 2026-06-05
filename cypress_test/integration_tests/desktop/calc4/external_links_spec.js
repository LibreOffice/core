/* global describe it cy beforeEach require */

var helper = require('../../common/helper');

describe(['tagdesktop'], 'External link agree snackbar', function () {
	beforeEach(function () {
		// the document holds back its external references until the user agrees
		helper.setupAndLoadDocument('calc/external-links.fods');
	});

	it('offers to agree and asks core to release the links on Allow', function () {
		cy.getFrameWindow().then(function (win) {
			cy.spy(win.app.socket, 'sendMessage').as('sendMessage');
		});

		cy.cGet('#snackbar-container #label')
			.should('have.text', 'This document has links to external content.');
		cy.cGet('#snackbar-container #button').should('have.text', 'Allow');

		cy.cGet('#snackbar-container #button').click();

		// Allow tells core the user agreed and dismisses the prompt
		cy.get('@sendMessage').should('be.calledWith', 'allowlinkupdate');
		cy.cGet('#snackbar-container').should('not.exist');
	});

	it('keeps the links blocked when dismissed', function () {
		cy.getFrameWindow().then(function (win) {
			cy.spy(win.app.socket, 'sendMessage').as('sendMessage');
		});

		cy.cGet('#snackbar-container #snackbar-dismiss-button').click();

		// dismissing closes the prompt without agreeing, so no release is sent
		cy.cGet('#snackbar-container').should('not.exist');
		cy.get('@sendMessage').should('not.be.calledWith', 'allowlinkupdate');
	});
});
