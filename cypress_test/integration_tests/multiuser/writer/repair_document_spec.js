/* global describe it cy beforeEach require */

var helper = require('../../common/helper');
var repairHelper = require('../../common/repair_document_helper');

describe.skip(['tagmultiuser'], 'Repair Document', function() {

	beforeEach(function() {
		helper.setupAndLoadDocument('writer/repair_doc.odt',true, false, undefined, 'vector=false');
	});

	function repairDoc(frameId1, frameId2) {
		cy.cSetActiveFrame(frameId1);
		cy.cGet('#document-container').click();

		helper.typeIntoDocument('Hello World');

		cy.wait(2000);

		cy.cSetActiveFrame(frameId2);
		repairHelper.rollbackPastChange('Typing: “World”');
		cy.cGet('#document-container').click();
		cy.wait(500);
		helper.selectAllText();
		helper.expectTextForClipboard('Hello \n');

		cy.cSetActiveFrame(frameId1);
		cy.cGet('#document-container').click();
		helper.selectAllText();
		helper.expectTextForClipboard('Hello \n');
	}

	it('Repair by user-2', function() {
		repairDoc('#iframe1', '#iframe2');
	});

	it('Repair by user-1', function() {
		repairDoc('#iframe2', '#iframe1');
	});
});
