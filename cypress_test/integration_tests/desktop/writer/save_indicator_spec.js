/* global describe it cy require */

var helper = require('../../common/helper');

describe(['tagdesktop'], 'Save indicator', function() {

	function editAndSave(fixture) {
		helper.setupAndLoadDocument(fixture);
		cy.cGet('[id^="save"].unotoolbutton').should('be.visible');

		helper.typeIntoDocument('hello');
		cy.cGet('[id^="save"].unotoolbutton').should('have.class', 'savemodified');

		helper.typeIntoDocument('{ctrl+s}');
	}

	it('reports a finished save on an ODF document', function() {
		editAndSave('writer/annotation.odt');
		cy.cGet('[id^="save"].unotoolbutton', { timeout: 20000 }).should('have.class', 'saved');
	});

	// The engine reports save progress only from the ODF filters, so the DOCX
	// indicator has to come from the result of the save itself.
	it('reports a finished save on a DOCX document', function() {
		editAndSave('writer/testfile.docx');
		cy.cGet('[id^="save"].unotoolbutton', { timeout: 20000 }).should('have.class', 'saved');
	});
});
