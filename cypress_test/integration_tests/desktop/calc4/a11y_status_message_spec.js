/* global describe it cy before expect require */

const helper = require('../../common/helper');
const desktopHelper = require('../../common/desktop_helper');
const a11yHelper = require('../../common/a11y_helper');

// A message with nothing to press has no widget to hand the focus to, so the
// dialog takes it itself. Without that the reader is never told the message is
// there, and Escape goes to the document instead of closing it.
describe(['tagdesktop'], 'Status message accessibility', { testIsolation: false }, function () {
	let win;

	before(function () {
		cy.viewport(1920, 1080);
		// The sidecar advertises EnableShare with nothing behind it, which is
		// what puts the button in the File tab and leaves the postMessage
		// handshake unanswered.
		helper.setupAndLoadDocument('calc/switch.ods', false, true);

		cy.getFrameWindow().then(function (frameWindow) {
			win = frameWindow;
		});

		desktopHelper.switchUIToNotebookbar();
		cy.cGet('#File-tab-label').click();
		cy.cGet('#File-container').should('be.visible');
	});

	function shareControl() {
		return win.document.querySelector(
			'#File-container [modelId*="shareas"], #File-container .shareas');
	}

	it('a message with no buttons is announced and closes on Escape', function () {
		cy.wrap(null).should(function () {
			expect(shareControl(), 'the Share button the host advertised')
				.to.not.equal(null);
		});

		// The sidecar buys the button, not the silence behind it: debug.html
		// answers the handshake that the whitebox host leaves hanging, so the
		// one thing the harness cannot be is a host that never replies.
		cy.then(function () {
			win.WOPIPostmessageReady = false;
			shareControl().click();
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
	});
});
