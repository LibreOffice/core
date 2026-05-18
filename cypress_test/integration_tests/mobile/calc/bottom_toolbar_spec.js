/* global describe it cy require beforeEach */

var helper = require('../../common/helper');
var calcHelper = require('../../common/calc_helper');
var mobileHelper = require('../../common/mobile_helper');

describe(['tagmobile', 'tagnextcloud', 'tagproxy'], 'Interact with bottom toolbar.', function() {

	beforeEach(function() {
		helper.setupAndLoadDocument('calc/bottom_toolbar.ods');

		// Click on edit button
		mobileHelper.enableEditingMobile();

		cy.cGet('#toolbar-down').should('exist');

		calcHelper.clickOnFirstCell();

		cy.getFrameWindow().then(function(win) {
			this.win = win;
		});

		// Wait for the cell state to fully switch
		cy.cGet('#toolbar-down .unoBold').should('not.have.attr', 'disabled');
	});

	function getTextEndPosForFirstCell() {
		calcHelper.dblClickOnFirstCell();

		helper.getCursorPos('left', 'currentTextEndPos')

		mobileHelper.getCompactIcon('AcceptFormula').should('not.be.disabled').click();

		cy.cGet('.cursor-overlay .blinking-cursor').should('not.exist');
	}

	it('Apply bold.', function() {
		helper.setDummyClipboardForCopy();
		helper.processToIdle(this.win);
		mobileHelper.getCompactIcon('Bold').should('not.be.disabled').click();
		calcHelper.selectEntireSheet();
		helper.copy();
		cy.cGet('#copy-paste-container table td b').should('exist');
	});

	it('Apply italic.', function() {
		helper.setDummyClipboardForCopy();
		helper.processToIdle(this.win);
		mobileHelper.getCompactIcon('Italic').should('not.be.disabled').click();
		calcHelper.selectEntireSheet();
		helper.copy();
		cy.cGet('#copy-paste-container table td i').should('exist');
	});

	it('Apply underline.', function() {
		helper.setDummyClipboardForCopy();
		helper.processToIdle(this.win);
		mobileHelper.getCompactIcon('Underline').should('not.be.disabled').click();
		calcHelper.selectEntireSheet();
		helper.copy();
		cy.cGet('#copy-paste-container table td u').should('exist');
	});

	it.skip('Apply strikeout.', function() {
		mobileHelper.getCompactIcon('Strikeout').should('not.be.disabled').click();
		calcHelper.selectEntireSheet();
		cy.cGet('#copy-paste-container table td s').should('exist');
	});

	it('Apply font color.', function() {
		helper.setDummyClipboardForCopy();
		helper.processToIdle(this.win);
		cy.cGet('#toolbar-down #fontcolor').should('not.be.disabled').click();
		mobileHelper.selectFromColorPalette(0, 5);
		calcHelper.selectEntireSheet();
		helper.copy();
		cy.cGet('#copy-paste-container table td font').should('have.attr', 'color', '#00FF00');
	});

	it('Apply highlight color.', function() {
		helper.setDummyClipboardForCopy();
		helper.processToIdle(this.win);
		cy.cGet('#toolbar-down #backcolor').should('not.be.disabled').click();
		mobileHelper.selectFromColorPalette(0, 5);
		calcHelper.selectEntireSheet();
		helper.copy();
		cy.cGet('#copy-paste-container table td').should('have.attr', 'bgcolor', '#00FF00');
	});

	it.skip('Merge cells', function() {
		// Select 100 cells in first row
		calcHelper.selectCellsInRange('A1:CV1');
		// Despite the selection is there, merge cells needs more time here.
		cy.wait(1000);
		cy.cGet('#toolbar-down #togglemergecells').click();
		calcHelper.selectCellsInRange('A1:CV1');
		cy.cGet('#copy-paste-container table td').should('have.attr', 'colspan', '100');
	});

	it.skip('Enable text wrapping.', function() {
		helper.initAliasToNegative('originalTextEndPos');
		getTextEndPosForFirstCell();
		cy.get('@currentTextEndPos').as('originalTextEndPos');
		cy.get('@currentTextEndPos').should('be.greaterThan', 0);
		calcHelper.clickOnFirstCell();
		cy.cGet('#toolbar-down #wraptext').click();

		// We use the text position as indicator
		cy.waitUntil(function() {
			getTextEndPosForFirstCell();

			return cy.get('@currentTextEndPos')
				.then(function(currentTextEndPos) {
					return cy.get('@originalTextEndPos')
						.then(function(originalTextEndPos) {
							return originalTextEndPos > currentTextEndPos;
						});
				});
		});
	});

	it('Hides the bottom toolbar when the screen is too small', function() {
		// By default in edit mode, the bottom toolbar is shown...
		cy.cGet('#toolbar-down').should('be.visible');
		cy.cGet('#spreadsheet-toolbar').should('be.visible');

		// ...even when we are in landscape mode...
		cy.viewport('iphone-6', 'landscape');
		cy.cGet('#toolbar-down').should('be.visible');
		cy.cGet('#spreadsheet-toolbar').should('be.visible');

		// ...but not if our onscreen keyboard is shown. Here simulated by setting our height just under the limit
		// ...there's no way to know exactly how big the size will be on different devices - particularly when taking into account browser UI - but 150px is still enough to edit the document so it's kind of OK
		cy.viewport(667, 149);
		cy.cGet('#toolbar-down').should('not.be.visible');
		cy.cGet('#spreadsheet-toolbar').should('not.be.visible');

		// ...and closing the keyboard should make it appear again
		cy.viewport('iphone-6', 'landscape');
		cy.cGet('#toolbar-down').should('be.visible');
		cy.cGet('#spreadsheet-toolbar').should('be.visible');
	});
});
