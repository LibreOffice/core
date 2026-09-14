/* global describe it cy before expect require */

const helper = require('../../common/helper');
const a11yHelper = require('../../common/a11y_helper');

// A message with nothing to press has no widget to hand the focus to, so the
// dialog takes it itself. Without that the reader is never told the message is
// there, and Escape goes to the document instead of closing it.
describe(['tagdesktop'], 'Status message accessibility', { testIsolation: false }, function () {
	let win;
	let ready;

	before(function () {
		cy.viewport(1920, 1080);
		helper.setupAndLoadDocument('calc/switch.ods');

		cy.getFrameWindow().then(function (frameWindow) {
			win = frameWindow;
		});
	});

	it('a message with no buttons is announced and closes on Escape', function () {
		// The message appears when the host advertised Share but never completed
		// the postMessage handshake, which is the state this forces.
		cy.then(function () {
			ready = win.WOPIPostmessageReady;
			win.WOPIPostmessageReady = false;
			win.app.map.openShare();
		});

		cy.cGet('.jsdialog-window.modalpopup').should('be.visible');

		cy.then(function () {
			const label = win.document.querySelector('#info-modal-label1');
			expect(label, 'the message the modal shows').to.not.equal(null);

			return a11yHelper.getFocusedAXNode().then(function (node) {
				expect(node.role, 'what the focus lands on').to.equal('dialog');
				expect(node.name, 'named after the message it carries')
					.to.equal(label.textContent.trim());
			});
		});

		cy.realPress('Escape');
		cy.cGet('.jsdialog-window.modalpopup').should('not.exist');

		cy.then(function () {
			win.WOPIPostmessageReady = ready;
		});
	});
});
