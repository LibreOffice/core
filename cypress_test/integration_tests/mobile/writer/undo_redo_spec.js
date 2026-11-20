/* global describe it cy beforeEach require */

var helper = require('../../common/helper');
var mobileHelper = require('../../common/mobile_helper');
var repairHelper = require('../../common/repair_document_helper');

describe.skip(['tagmobile', 'tagnextcloud', 'tagproxy'], 'Editing Operations', function() {

	beforeEach(function() {
		helper.setupAndLoadDocument('writer/undo_redo.odt', false, false, undefined, 'vector=false');
		// Click on edit button
		mobileHelper.enableEditingMobile();
	});

	function undo() {
		helper.typeIntoDocument('Hello World');
		//if we don't wait tests in CI is failing
		cy.wait(1000);
		cy.cGet('#toolbar-up #undo').click();
		helper.selectAllText();
		helper.expectTextForClipboard('Hello \n');
	}

	it('Undo', function() {
		undo();
	});

	it('Redo',function() {
		undo();
		cy.cGet('#toolbar-up #redo').click();
		helper.selectAllText();
		helper.expectTextForClipboard('Hello World');
	});

	it('Repair Document', function() {
		helper.typeIntoDocument('Hello World');
		repairHelper.rollbackPastChange('Typing: “World”', undefined, true);
		helper.selectAllText();
		helper.expectTextForClipboard('Hello \n');
	});
});
