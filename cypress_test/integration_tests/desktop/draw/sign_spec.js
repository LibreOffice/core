/* global describe it cy require */

var helper = require('../../common/helper');

describe(['tagdesktop'], 'Signature operations.', function() {

	it('Create a visual signature.', { env: { 'pdf-view': true } }, function() {
		// Given a document that can be signed:
		helper.setupAndLoadDocument('draw/sign.pdf', /*isMultiUser=*/false, /*copyCertificates=*/true);

		cy.wait(1000); // wait for resize after the first tile is received

		// When visually signing that document:
		cy.cGet('#menu-insert').click();
		// Insert signature line/shape. The signing certificate comes from the
		// session and is put on the view: there is only one cert+key pair, so
		// no certificate chooser appears, the signature line is inserted with
		// that certificate directly.
		cy.cGet('#menu-insert-signatureline').click();
		// Make sure the signature line has handles, so it can be moved/resized:
		cy.cGet('#test-div-shapeHandlesSection').should('exist');
		cy.cGet('#SelectCertificateDialog').should('not.exist');
		// Finish signing:
		cy.cGet('#menu-file').click();
		cy.cGet('#menu-signature').click();

		// Then make sure the resulting signature is valid:
		cy.cGet('[id^="signstatus"] div').should('have.class', 'sign_ok');
	});
});
