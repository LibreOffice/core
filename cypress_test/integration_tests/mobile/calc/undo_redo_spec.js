/* global describe it cy beforeEach require */

var helper = require('../../common/helper');
var mobileHelper = require('../../common/mobile_helper');
var calcHelper = require('../../common/calc_helper');
var repairHelper = require('../../common/repair_document_helper');

describe.skip(['tagmobile', 'tagnextcloud', 'tagproxy'], 'Editing Operations', function() {

	beforeEach(function() {
		helper.setupAndLoadDocument('calc/undo_redo.ods', false, false, undefined, 'vector=false');

		// Click on edit button
		mobileHelper.enableEditingMobile();
	});

	function undo() {
		cy.wait(1000);

		calcHelper.dblClickOnFirstCell();

		helper.typeIntoDocument('Hello World');

		cy.cGet('#toolbar-up #acceptformula').click();

		//if we don't wait tests in CLI is failing
		cy.wait(3000);

		cy.cGet('#toolbar-up #undo').click();

		calcHelper.dblClickOnFirstCell();

		helper.typeIntoDocument('{ctrl}{a}');

		helper.textSelectionShouldNotExist();

		cy.cGet('#toolbar-up #acceptformula').click();
	}

	it('Undo', function() {
		undo();
	});

	it('Redo', function() {
		undo();

		cy.wait(3000);

		cy.cGet('#toolbar-up #redo').click();

		calcHelper.dblClickOnFirstCell();

		helper.selectAllText();

		helper.expectTextForClipboard('Hello World');
	});

	it('Repair Document', function() {
		calcHelper.dblClickOnFirstCell();

		helper.typeIntoDocument('Hello World');

		cy.wait(3000);

		cy.cGet('#toolbar-up #acceptformula').click();

		calcHelper.dblClickOnFirstCell();

		helper.clearAllText();

		helper.typeIntoDocument('Hello');

		cy.wait(3000);

		cy.cGet('#toolbar-up #acceptformula').click();

		repairHelper.rollbackPastChange('Undo', undefined, true);

		calcHelper.dblClickOnFirstCell();

		helper.selectAllText();

		helper.expectTextForClipboard('Hello World');
	});
});
