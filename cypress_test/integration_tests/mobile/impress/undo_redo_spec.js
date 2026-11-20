/* global describe it cy beforeEach require */

var helper = require('../../common/helper');
var mobileHelper = require('../../common/mobile_helper');
var impressHelper = require('../../common/impress_helper');
var repairHelper = require('../../common/repair_document_helper');

describe.skip(['tagmobile', 'tagnextcloud', 'tagproxy'], 'Editing Operations', function() {

	beforeEach(function() {
		helper.setupAndLoadDocument('impress/undo_redo.odp', false, false, undefined, 'vector=false');

		// Click on edit button
		mobileHelper.enableEditingMobile();

		impressHelper.selectTextShapeInTheCenter();

		cy.cGet('g.leaflet-control-buttons-disabled svg').dblclick({force:true});

		cy.wait(1000);

		helper.typeIntoDocument('Hello World');
	});

	function undo() {
		cy.cGet('path.leaflet-interactive').dblclick();

		//if we don't wait tests in CLI is failing
		cy.wait(3000);

		cy.cGet('#toolbar-up #undo').click();

		helper.selectAllText();

		cy.wait(1000);

		helper.expectTextForClipboard('Hello Worl');
	}

	it('Undo', function() {
		undo();
	});


	it('Redo',function() {
		undo();

		cy.cGet('#toolbar-up #redo').click();

		helper.selectAllText();

		cy.wait(1000);

		helper.expectTextForClipboard('Hello World');
	});

	it.skip('Repair Document', function() {
		// End text edit, so 'Hello World' text is added to Undo stack
		helper.typeIntoDocument('{esc}');

		// Overwrite the text in the shape with text 'Overwrite'
		impressHelper.selectTextShapeInTheCenter();
		impressHelper.selectTextOfShape();
		cy.wait(1000);
		helper.typeIntoDocument('Overwrite');
		helper.typeIntoDocument('{esc}');

		repairHelper.rollbackPastChange('Undo', undefined, true);

		// Check the text in the shape reverted to "Hello World"
		impressHelper.selectTextShapeInTheCenter();
		impressHelper.selectTextOfShape();
		cy.wait(1000);
		helper.expectTextForClipboard('Hello World');
	});
});
