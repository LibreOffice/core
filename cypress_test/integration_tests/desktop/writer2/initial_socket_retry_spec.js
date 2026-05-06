/* global describe it require cy Cypress */

var helper = require('../../common/helper');

describe(['tagdesktop'], 'Initial WebSocket connection retry', function() {

	it('retries on transient WebSocket failure during initial load', function() {
		var newFilePath = helper.setupDocument('writer/undo_redo.odt');

		// Build the document URL with simulateError param that
		// makes Socket.ts close the first WebSocket after it opens,
		// simulating the transient connection failure in T50464.
		var URI = '/browser/' + Cypress.env('WSD_VERSION_HASH') + '/debug.html'
			+ '?lang=en-US'
			+ '&file_path=' + Cypress.env('DATA_WORKDIR') + newFilePath
			+ '&simulateError=socket:initialClose';

		cy.cSetActiveFrame('#coolframe');

		cy.visit(URI, {
			onBeforeLoad: function(win) {
				win.addEventListener('error', function(event) {
					Cypress.log({
						name: 'error:',
						message: (event.error.message ? event.error.message : 'no message'),
					});
				});
			},
		});

		// With the fix the first WebSocket closes but the client
		// retries and the document loads on the second attempt.
		// Without the fix this times out.
		//
		// Only check that the document actually loaded. The full
		// documentChecks() also waits for notebookbar and sidebar
		// initialization which can be disrupted by the socket
		// retry flow (the notebookbar container is replaced during
		// re-initialization but the initialized flag is not reset).
		var timeout = Cypress.config('defaultCommandTimeout') * 2.0;
		cy.cGet('#document-canvas', {timeout: timeout})
			.should('be.visible');
		cy.cGet('#map', {timeout: timeout})
			.should('have.class', 'initialized');
	});
});
