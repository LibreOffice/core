/* global describe it cy beforeEach require expect*/

var helper = require('../../common/helper');
var calcHelper = require('../../common/calc_helper');
var mobileHelper = require('../../common/mobile_helper');

describe(['tagmobile', 'tagnextcloud', 'tagproxy'], 'Calc spell checking menu.', function() {

	beforeEach(function() {
		// Calc starts with automatic spell checking off, so ask for it before
		// the document loads - the suggestions below need the marked word.
		localStorage.setItem('spreadsheet.spellOnline', true);
		helper.setupAndLoadDocument('calc/spellchecking.ods');

		// Click on edit button
		mobileHelper.enableEditingMobile();
	});

	function openContextMenu() {
		// Click and then long press on first cell
		cy.cGet('#document-canvas')
			.then(function(items) {
				expect(items).to.have.lengthOf(1);
				var XPos = items[0].getBoundingClientRect().left + 60;
				var YPos = items[0].getBoundingClientRect().top + 30;
				cy.cGet('body').rightclick(XPos, YPos);
			});

		cy.cGet('#mobile-wizard-content').should('be.visible');
	}

	it('Apply suggestion.', function() {
		helper.setDummyClipboardForCopy();
		openContextMenu();
		cy.cGet('body').contains('.context-menu-link', 'hello').click();

		calcHelper.assertSheetContents(['hello'], true);

		// We don't get the spell check context menu any more
		openContextMenu();
		cy.cGet('body').contains('.context-menu-link', 'Paste').should('be.visible');
	});

	it('Ignore all.', function() {
		helper.setDummyClipboardForCopy();
		openContextMenu();
		cy.cGet('body').contains('.context-menu-link', 'Ignore All').click();

		calcHelper.assertSheetContents(['helljo'], true);

		// We don't get the spell check context menu any more
		openContextMenu();
		cy.cGet('body').contains('.context-menu-link', 'Paste').should('be.visible');
	});
});
