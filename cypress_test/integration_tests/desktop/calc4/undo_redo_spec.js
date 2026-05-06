/* global describe it cy beforeEach require */

var helper = require('../../common/helper');
var calcHelper = require('../../common/calc_helper');
var repairHelper = require('../../common/repair_document_helper');

describe(['tagdesktop', 'tagnextcloud', 'tagproxy'], 'Editing Operations', function() {

	beforeEach(function() {
		helper.setupAndLoadDocument('calc/undo_redo.ods');
	});

	function undo() {
		helper.typeIntoDocument('Hello World');
		helper.typeIntoDocument('{ctrl}z');
		helper.selectAllText();
		helper.copy();
		cy.cGet('#copy-paste-container pre').should('not.have.text', 'Hello World');
	}

	it('Undo', function() {
		helper.setDummyClipboardForCopy();
		undo();
	});

	it('Redo', function() {
		helper.setDummyClipboardForCopy();
		undo();
		helper.typeIntoDocument('{ctrl}y');
		helper.selectAllText();
		helper.copy();
		helper.expectTextForClipboard('Hello World');
	});

	it('Repair Document', function() {
		helper.setDummyClipboardForCopy();
		helper.typeIntoDocument('Hello World');
		helper.typeIntoDocument('{enter}');
		calcHelper.dblClickOnFirstCell();
		helper.clearAllText();
		helper.typeIntoDocument('Hello');
		helper.typeIntoDocument('{enter}');
		repairHelper.rollbackPastChange('Undo', /* mobile */ false, /* nb */ true);
		calcHelper.dblClickOnFirstCell();
		helper.selectAllText();
		helper.copy();
		helper.expectTextForClipboard('Hello World');
	});
});
