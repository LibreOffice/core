/* global describe it cy beforeEach require */

var helper = require('../../common/helper');
var calcHelper = require('../../common/calc_helper');
var desktopHelper = require('../../common/desktop_helper');

describe(['tagdesktop'], 'Change cell appearance.', function() {

	beforeEach(function() {
		helper.setupAndLoadDocument('calc/cell_appearance.ods');
		cy.viewport(1920,1080);
	});

	it('Apply background color', function() {
		helper.setDummyClipboardForCopy();
		desktopHelper.switchUIToNotebookbar();
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
		desktopHelper.switchUIToNotebookbar();
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
		desktopHelper.switchUIToNotebookbar();
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
		desktopHelper.switchUIToNotebookbar();
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
		desktopHelper.switchUIToNotebookbar();
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
		desktopHelper.switchUIToNotebookbar();
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
		desktopHelper.switchUIToNotebookbar();
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
		desktopHelper.switchUIToNotebookbar();
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
		desktopHelper.switchUIToNotebookbar();
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
		desktopHelper.switchUIToNotebookbar();
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
		desktopHelper.switchUIToNotebookbar();
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
		desktopHelper.switchUIToNotebookbar();
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
		desktopHelper.switchUIToNotebookbar();
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

	it('Apply border color', function() {
		helper.setDummyClipboardForCopy();
		desktopHelper.switchUIToNotebookbar();
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
});
