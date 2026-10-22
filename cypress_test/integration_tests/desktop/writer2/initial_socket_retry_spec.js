/* global describe it require cy Cypress expect */

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

		// The first WebSocket closes, the client retries, and the document loads on
		// the second socket. Without the retry this times out.
		cy.cGet('#document-canvas').should('be.visible');
		cy.getFrameWindow().should(function(win) {
			expect(win.app.map._docLoaded, 'document loaded').to.equal(true);
		});
	});
});
