/* global describe it cy expect require */

var helper = require('../../common/helper');
var calcHelper = require('../../common/calc_helper');
var desktopHelper = require('../../common/desktop_helper');

describe(['tagdesktop'], 'Change cell appearance.', { testIsolation: false }, function() {

	desktopHelper.shareDocumentAcrossTests('calc/cell_appearance.ods', {
		notebookbar: true,
		viewport: [1920, 1080],
	});

	it('Apply background color', function() {
		helper.setDummyClipboardForCopy();
		calcHelper.clickOnFirstCell();
		cy.cGet('#Home').click();
		cy.cGet('#Home-container .unoBackgroundColor .arrowbackground').click();
		desktopHelper.selectColorFromPalette('BF0041');
		calcHelper.selectEntireSheet();
		helper.copy();
		cy.cGet('#copy-paste-container table td').should('have.attr', 'bgcolor', '#BF0041');
	});

	it('Apply left border', function() {
		helper.setDummyClipboardForCopy();
		calcHelper.clickOnFirstCell();
		cy.cGet('#Home-container .unoSetBorderStyle .arrowbackground').click();
		cy.cGet('.ui-dialog-content').should('be.visible');
		helper.getMenuEntry(1).click();
		calcHelper.selectEntireSheet();
		helper.copy();
		cy.cGet('#copy-paste-container table td').should('have.attr', 'style', 'border-left: 1px solid #000000');
	});

	it('Remove cell border', function() {
		helper.setDummyClipboardForCopy();
		calcHelper.clickOnFirstCell();
		// First add left border
		cy.cGet('.notebookbar .unoSetBorderStyle .arrowbackground').click();
		cy.cGet('.ui-dialog-content').should('be.visible');
		helper.getMenuEntry(1).click();
		helper.copy();
		calcHelper.selectEntireSheet();
		cy.cGet('#copy-paste-container table td').should('have.attr', 'style', 'border-left: 1px solid #000000');
		// Then remove it
		calcHelper.clickOnFirstCell();
		cy.cGet('.notebookbar .unoSetBorderStyle .arrowbackground').click();
		cy.cGet('.ui-dialog-content').should('be.visible');
		helper.getMenuEntry(0).click();
		helper.copy();
		calcHelper.selectEntireSheet();
		cy.cGet('#copy-paste-container table td').should('not.have.attr', 'style');
	});

	it('Apply right border', function() {
		helper.setDummyClipboardForCopy();
		calcHelper.clickOnFirstCell();
		cy.cGet('.notebookbar .unoSetBorderStyle .arrowbackground').click();
		cy.cGet('.ui-dialog-content').should('be.visible');
		helper.getMenuEntry(2).click();
		calcHelper.selectEntireSheet();
		helper.copy();
		cy.cGet('#copy-paste-container table td').should('have.attr', 'style', 'border-right: 1px solid #000000');
	});

	it('Apply left and right border', function() {
		helper.setDummyClipboardForCopy();
		calcHelper.clickOnFirstCell();
		cy.cGet('.notebookbar .unoSetBorderStyle .arrowbackground').click();
		cy.cGet('.ui-dialog-content').should('be.visible');
		helper.getMenuEntry(3).click();
		helper.copy();
		calcHelper.selectEntireSheet();
		cy.cGet('#copy-paste-container table td').should('have.attr', 'style', 'border-left: 1px solid #000000; border-right: 1px solid #000000');
	});

	it('Apply top border', function() {
		helper.setDummyClipboardForCopy();
		calcHelper.clickOnFirstCell();
		cy.cGet('.notebookbar .unoSetBorderStyle .arrowbackground').click();
		cy.cGet('.ui-dialog-content').should('be.visible');
		helper.getMenuEntry(4).click();
		helper.copy();
		calcHelper.selectEntireSheet();
		cy.cGet('#copy-paste-container table td').should('have.attr', 'style', 'border-top: 1px solid #000000');
	});

	it('Apply bottom border', function() {
		helper.setDummyClipboardForCopy();
		calcHelper.clickOnFirstCell();
		cy.cGet('.notebookbar .unoSetBorderStyle .arrowbackground').click();
		cy.cGet('.ui-dialog-content').should('be.visible');
		helper.getMenuEntry(5).click();
		helper.copy();
		calcHelper.selectEntireSheet();
		cy.cGet('#copy-paste-container table td').should('have.attr', 'style', 'border-bottom: 1px solid #000000');
	});

	it('Apply top and bottom border', function() {
		helper.setDummyClipboardForCopy();
		calcHelper.clickOnFirstCell();
		cy.cGet('.notebookbar .unoSetBorderStyle .arrowbackground').click();
		cy.cGet('.ui-dialog-content').should('be.visible');
		helper.getMenuEntry(6).click();
		helper.copy();
		calcHelper.selectEntireSheet();
		cy.cGet('#copy-paste-container table td').should('have.attr', 'style', 'border-top: 1px solid #000000; border-bottom: 1px solid #000000');
	});

	it('Apply border for all sides', function() {
		helper.setDummyClipboardForCopy();
		calcHelper.clickOnFirstCell();
		cy.cGet('.notebookbar .unoSetBorderStyle .arrowbackground').click();
		cy.cGet('.ui-dialog-content').should('be.visible');
		helper.getMenuEntry(7).click();
		helper.copy();
		calcHelper.selectEntireSheet();
		cy.cGet('#copy-paste-container table td')
			.should('have.attr', 'style', 'border-top: 1px solid #000000; border-bottom: 1px solid #000000; border-left: 1px solid #000000; border-right: 1px solid #000000');
	});

	it('Apply horizontal borders for multiple cells', function() {
		calcHelper.selectEntireSheet();
		// Click on the one in notebookbar (not sidebar).
		cy.cGet('.notebookbar .unoSetBorderStyle .arrowbackground').click();
		cy.cGet('.ui-dialog-content').should('be.visible');
		helper.getMenuEntry(8).click();
		calcHelper.selectEntireSheet();

		// copy-paste container is not stable for now.
		//cy.cGet('#copy-paste-container table td').should(function(cells) {
		//		expect(cells).to.have.lengthOf(4);
		//		for (var i = 0; i < cells.length; i++) {
		//			expect(cells[i]).to.have.attr('style', 'border-top: 1px solid #000000; border-bottom: 1px solid #000000');
		//		}
		//	});
	});

	it('Apply horizontal inner borders and vertical outer borders', function() {
		calcHelper.selectEntireSheet();
		cy.cGet('.notebookbar .unoSetBorderStyle .arrowbackground').click();
		cy.cGet('.ui-dialog-content').should('be.visible');
		helper.getMenuEntry(9).click();
		calcHelper.selectEntireSheet();
		//cy.cGet('#copy-paste-container table td')
		//	.should(function(cells) {
		//		expect(cells).to.have.lengthOf(4);
		//		for (var i = 0; i < cells.length; i++) {
		//			if (i == 0 || i == 2)
		//				expect(cells[i]).to.have.attr('style', 'border-top: 1px solid #000000; border-bottom: 1px solid #000000; border-left: 1px solid #000000');
		//			else
		//				expect(cells[i]).to.have.attr('style', 'border-top: 1px solid #000000; border-bottom: 1px solid #000000');
		//		}
		//	});
	});

	it('Apply vertical inner borders and horizontal outer borders', function() {
		calcHelper.selectEntireSheet();
		cy.cGet('.notebookbar .unoSetBorderStyle .arrowbackground').click();
		cy.cGet('.ui-dialog-content').should('be.visible');
		helper.getMenuEntry(10).click();
		calcHelper.selectEntireSheet();
		//cy.cGet('#copy-paste-container table td')
		//	.should(function(cells) {
		//		expect(cells).to.have.lengthOf(4);
		//		for (var i = 0; i < cells.length; i++) {
		//			if (i == 0 || i == 1)
		//				expect(cells[i]).to.have.attr('style', 'border-top: 1px solid #000000; border-left: 1px solid #000000; border-right: 1px solid #000000');
		//			else
		//				expect(cells[i]).to.have.attr('style', 'border-left: 1px solid #000000; border-right: 1px solid #000000');
		//		}
		//	});
	});

	it('Apply all inner and outer borders', function() {
		calcHelper.selectEntireSheet();
		cy.cGet('.notebookbar .unoSetBorderStyle .arrowbackground').click();
		cy.cGet('.ui-dialog-content').should('be.visible');
		helper.getMenuEntry(11).click();
		calcHelper.selectEntireSheet();
		//cy.cGet('#copy-paste-container table td')
		//	.should(function(cells) {
		//		expect(cells).to.have.lengthOf(4);
		//		for (var i = 0; i < cells.length; i++) {
		//			expect(cells[i]).to.have.attr('style', 'border-top: 1px solid #000000; border-bottom: 1px solid #000000; border-left: 1px solid #000000; border-right: 1px solid #000000');
		//		}
		//	});
	});

	it('Line color and style are disabled until a border exists', function() {
		calcHelper.clickOnFirstCell();

		// A cell with no border has no line to recolor or restyle, so core
		// reports the line style command as disabled and both entries stay
		// disabled.
		helper.waitForMapState('.uno:LineStyle', 'disabled');
		cy.cGet('.notebookbar .unoSetBorderStyle .arrowbackground').click();
		cy.cGet('.ui-dialog-content').should('be.visible');
		cy.cGet('body').contains('.ui-combobox-entry', 'Line color').should('have.class', 'disabled');
		cy.cGet('body').contains('.ui-combobox-entry', 'Line style').should('have.class', 'disabled');

		// Give the cell an outer border on every side.
		helper.getMenuEntry(7).click();
		cy.cGet('.ui-dialog-content').should('not.exist');

		// Now that a border exists, both entries can be used.
		calcHelper.clickOnFirstCell();
		helper.waitForMapState('.uno:LineStyle', 'enabled');
		cy.cGet('.notebookbar .unoSetBorderStyle .arrowbackground').click();
		cy.cGet('.ui-dialog-content').should('be.visible');
		cy.cGet('body').contains('.ui-combobox-entry', 'Line color').should('not.have.class', 'disabled');
		cy.cGet('body').contains('.ui-combobox-entry', 'Line style').should('not.have.class', 'disabled');
	});

	// The colour stays on the border dropdown for the rest of the file, so this
	// test comes after the ones that expect black borders.
	it('Apply border color', function() {
		helper.setDummyClipboardForCopy();
		calcHelper.clickOnFirstCell();
		// Apply left border first
		cy.cGet('.notebookbar .unoSetBorderStyle .arrowbackground').click();
		cy.cGet('.ui-dialog-content').should('be.visible');
		helper.getMenuEntry(1).click();

		// Wait for popup to close before clicking next button
		cy.cGet('.ui-dialog-content').should('not.exist');

		// Then apply border color
		cy.cGet('.unoFrameLineColor .arrowbackground').click();
		desktopHelper.selectColorFromPalette('BF0041');
		helper.copy();
		calcHelper.selectEntireSheet();

		cy.cGet('#copy-paste-container table td')
			.should('have.attr', 'style', 'border-left: 1px solid #bf0041');
	});

	// The width stays on the border dropdown for the rest of the file, so this
	// test comes last, after the ones that expect the one pixel default.
	it('Apply a thicker line style from the border dropdown', function() {
		helper.setDummyClipboardForCopy();
		calcHelper.clickOnFirstCell();

		// Give the cell a border on every side so a line style change has
		// something to thicken.
		cy.cGet('.notebookbar .unoSetBorderStyle .arrowbackground').click();
		cy.cGet('.ui-dialog-content').should('be.visible');
		helper.getMenuEntry(7).click();
		cy.cGet('.ui-dialog-content').should('not.exist');

		// The default border is one pixel wide. The colour is whatever the
		// dropdown carries, and this test is about the width.
		helper.copy();
		calcHelper.selectEntireSheet();
		cy.cGet('#copy-paste-container table td')
			.should('have.attr', 'style')
			.and('match', /border-top: 1px/);

		// Pick the extra thick width from the Line style submenu of the same dropdown.
		// Hovering the entry opens its submenu.
		calcHelper.clickOnFirstCell();
		// The Line style entry is only usable once core reports the line style
		// command as enabled for the reselected cell, so wait for that before
		// opening the dropdown.
		helper.waitForMapState('.uno:LineStyle', 'enabled');
		cy.cGet('.notebookbar .unoSetBorderStyle .arrowbackground').click();
		cy.cGet('.ui-dialog-content').should('be.visible');
		cy.cGet('body').contains('.ui-combobox-entry', 'Line style').trigger('mouseover');
		cy.cGet('body').contains('.ui-combobox-entry', 'Extra thick (4.50 pt)').should('be.visible').click();

		// The border grows past the one pixel default once the line style is applied.
		helper.copy();
		calcHelper.selectEntireSheet();
		cy.cGet('#copy-paste-container table td')
			.should('have.attr', 'style')
			.and((style) => {
				const match = style.match(/border-top: (\d+)px/);
				expect(match, 'top border width is present').to.not.be.null;
				expect(parseInt(match[1], 10)).to.be.greaterThan(1);
			});
	});
});
