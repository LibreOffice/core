/* global describe it cy beforeEach require expect*/

var helper = require('../../common/helper');
var mobileHelper = require('../../common/mobile_helper');
var calcHelper = require('../../common/calc_helper');

describe.skip(['tagmobile'], 'Calc focus tests', function() {

	beforeEach(function() {
		helper.setupAndLoadDocument('calc/focus.ods', false, false, undefined, 'vector=false');

		// Wait until the Formula-Bar is loaded.
		cy.cGet('.inputbar_container', {timeout : 10000});
	});

	it('Basic document focus.', function() {
		// Click on edit button
		mobileHelper.enableEditingMobile();

		// Body has the focus -> can't type in the document
		helper.assertFocus('tagName', 'BODY');

		// One tap on another cell -> no focus on the document
		calcHelper.clickOnFirstCell();

		// No focus
		helper.assertFocus('tagName', 'BODY');

		// Double tap on another cell gives the focus to the document
		cy.cGet('.spreadsheet-cell-resize-marker')
			.then(function(items) {
				expect(items).to.have.lengthOf(2);
				var XPos = Math.max(items[0].getBoundingClientRect().right, items[1].getBoundingClientRect().right) + 10;
				var YPos = Math.max(items[0].getBoundingClientRect().top, items[1].getBoundingClientRect().top) - 10;
				cy.cGet('body').dblclick(XPos, YPos);
			});

		// Document has the focus
		helper.assertFocus('className', 'clipboard');
	});

	it('Focus on second tap.', function() {
		// Click on edit button
		mobileHelper.enableEditingMobile();

		// Body has the focus -> can't type in the document
		helper.assertFocus('tagName', 'BODY');

		// One tap on a cell -> no document focus
		calcHelper.clickOnFirstCell();

		// No focus
		helper.assertFocus('tagName', 'BODY');

		// Second tap on the same cell
		calcHelper.clickOnFirstCell(false);

		// Document has the focus
		helper.assertFocus('className', 'clipboard');
	});

	it.skip('Formula-bar focus', function() {
		// Click on edit button
		mobileHelper.enableEditingMobile();

		// Body has the focus -> can't type in the document
		helper.assertFocus('tagName', 'BODY');

		helper.assertNoKeyboardInput();

		// Select the first cell to edit the same one.
		calcHelper.clickOnFirstCell();

		// No focus
		helper.assertFocus('tagName', 'BODY');

		// Click in the formula-bar.
		calcHelper.clickFormulaBar();
		helper.assertCursorAndFocus();

		// Type some text.
		var text1 = 'Hello from Calc';
		calcHelper.typeIntoFormulabar(text1);
		cy.cGet('#toolbar-up #acceptformula')
			.click();
		helper.assertNoKeyboardInput();

		// Select the first cell to edit the same one.
		calcHelper.clickOnFirstCell();

		// Check the text we typed.
		calcHelper.clickFormulaBar();
		helper.assertCursorAndFocus();
		calcHelper.typeIntoFormulabar('{ctrl}a');
		helper.expectTextForClipboard(text1);

		// Accept changes.
		cy.cGet('#toolbar-up #acceptformula')
			.click();
		helper.assertNoKeyboardInput();

		// Type some more text, at the end.
		cy.log('Appending text at the end.');
		calcHelper.clickOnFirstCell();
		calcHelper.clickFormulaBar();
		helper.assertCursorAndFocus();
		var text2 = ', this is a test.';
		calcHelper.typeIntoFormulabar(text2);
		// Validate.
		calcHelper.typeIntoFormulabar('{ctrl}a');
		helper.expectTextForClipboard(text1 + text2);
		// End editing.
		cy.cGet('#toolbar-up #acceptformula')
			.click();
		helper.assertNoKeyboardInput();
	});
});
