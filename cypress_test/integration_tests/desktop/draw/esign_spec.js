/* global describe it cy expect require */

var helper = require('../../common/helper');

describe(['tagdesktop'], 'Electronic sign operations.', function() {

	it('Create an electronic signature.', { env: { 'pdf-view': true } }, function() {
		// Given a document that can be signed:
		helper.setupAndLoadDocument('draw/esign.pdf', /*isMultiUser=*/false, /*copyCertificates=*/true, undefined, 'vector=false');

		let sendHashResult;
		cy.fixture('fixtures/eideasy-send-hash.json').then((result) => {
			sendHashResult = result;
		});
		let getSignatureResult;
		cy.fixture('fixtures/eideasy-get-signature.json').then((result) => {
			getSignatureResult = result;
		});
		cy.getFrameWindow().then(function(win) {
			const sendUnoCommand = cy.stub(win.app.map, 'sendUnoCommand');
			sendUnoCommand.withArgs('.uno:PrepareSignature').as('sendHash').callsFake((commandName, args) => {
				expect(args.body.signature_redirect).to.satisfy(url => url.endsWith('/cool/signature'));
				// File name is like esign-Create-an-electronic-signature--0wvs9.pdf
				expect(args.body.files[0].fileName).to.match(/^esign.*pdf$/i);
				win.app.map.fire('commandresult', {commandName: '.uno:PrepareSignature', success: true, result: sendHashResult});
			});
			sendUnoCommand.withArgs('.uno:DownloadSignature').as('getSignature').callsFake(() => {
				win.app.map.fire('commandresult', {commandName: '.uno:DownloadSignature', success: true, result: getSignatureResult});
			});
			// Call the original sendUnoCommand() for other commands
			sendUnoCommand.callThrough();
		});
		cy.getFrameWindow()
			.then(function(win) {
				cy.stub(win, 'open').as('windowOpen');
			});

		// When signing that document:
		// Start signing:
		cy.cGet('#menu-insert').click();
		cy.cGet('#menu-insert-signatureline').click();
		// Finish electronic signing:
		cy.cGet('#snackbar-container #button').click();
		cy.get('@sendHash').should('be.called');
		cy.cGet('#ESignatureDialog #ok.ui-pushbutton-wrapper').click();
		cy.get('@windowOpen').should('be.called');
		const response = {
			type: "SUCCESS",
		};
		cy.window().then(win => {
			const app = win['0'].app;
			const eSignature = app.map.eSignature;
			eSignature.handleSigned(response);
		});
		cy.get('@getSignature').should('be.called');
		// Close the dialog showing the just created signature:
		cy.cGet('#DigitalSignaturesDialog #close.ui-pushbutton-wrapper button').click();

		// Then make sure the document now has a (test / "not OK") signature:
		cy.cGet('[id^="signstatus"] div').should('have.class', 'sign_not_ok');
	});
});
