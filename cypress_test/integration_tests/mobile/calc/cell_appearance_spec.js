/* global describe it cy beforeEach require expect*/

var helper = require('../../common/helper');
var calcHelper = require('../../common/calc_helper');
var mobileHelper = require('../../common/mobile_helper');

describe(['tagmobile', 'tagnextcloud', 'tagproxy'], 'Change cell appearance.', function() {

	beforeEach(function() {
		helper.setupAndLoadDocument('calc/cell_appearance.ods', false, false, undefined, 'vector=false');

		// Click on edit button
		mobileHelper.enableEditingMobile();
	});

	function openAppearencePanel() {
		mobileHelper.openMobileWizard();

		cy.cGet('#ScCellAppearancePropertyPanel, .ScCellAppearancePropertyPanel').click();

		cy.cGet('body').contains('.menu-entry-with-icon', 'Background Color')
			.should('be.visible');
	}

	function openAppearencePanelOnFirstCell() {
		calcHelper.clickOnFirstCell();

		openAppearencePanel();
	}

	function openAppearencePanelOnAllCells() {
		calcHelper.selectEntireSheet();

		openAppearencePanel();
	}

	it('Apply background color', function() {
		helper.setDummyClipboardForCopy();
		openAppearencePanelOnFirstCell();

		cy.cGet('#BackgroundColor').click();

		mobileHelper.selectFromColorPicker('#BackgroundColor', 2);

		// Check that the color is shown as selected
		cy.cGet('#BackgroundColor .color-sample-selected')
			.should('have.attr', 'style', 'background-color: rgb(255, 0, 0);');

		calcHelper.selectEntireSheet();
		helper.copy();

		cy.cGet('#copy-paste-container table td')
			.should('have.attr', 'bgcolor', '#FF0000');
	});

	it('Apply left border', function() {
		helper.setDummyClipboardForCopy();
		openAppearencePanelOnFirstCell();

		cy.cGet('#border-2').click();

		calcHelper.selectEntireSheet();
		helper.copy();

		cy.cGet('#copy-paste-container table td')
			.should('have.attr', 'style', 'border-left: 1px solid #000000');
	});

	it('Remove cell border', function() {
		helper.setDummyClipboardForCopy();
		openAppearencePanelOnFirstCell();

		// First add left border
		cy.cGet('#border-2').click();

		// Close mobile wizard gracefully now to avoid sporadic
		// failures when reopening it later
		mobileHelper.closeMobileWizard();

		calcHelper.selectEntireSheet();
		helper.copy();

		cy.cGet('#copy-paste-container table td')
			.should('have.attr', 'style', 'border-left: 1px solid #000000');

		// Then remove it
		openAppearencePanelOnFirstCell();

		cy.cGet('#border-1').click();

		calcHelper.selectEntireSheet();
		helper.copy();

		cy.cGet('#copy-paste-container table td')
			.should('not.have.attr', 'style');
	});

	it('Apply right border', function() {
		helper.setDummyClipboardForCopy();
		openAppearencePanelOnFirstCell();

		cy.cGet('#border-3').click();

		calcHelper.selectEntireSheet();
		helper.copy();

		cy.cGet('#copy-paste-container table td')
			.should('have.attr', 'style', 'border-right: 1px solid #000000');
	});

	it('Apply left and right border', function() {
		helper.setDummyClipboardForCopy();
		openAppearencePanelOnFirstCell();

		cy.cGet('#border-4').click();

		calcHelper.selectEntireSheet();
		helper.copy();

		cy.cGet('#copy-paste-container table td')
			.should('have.attr', 'style', 'border-left: 1px solid #000000; border-right: 1px solid #000000');
	});

	it('Apply top border', function() {
		helper.setDummyClipboardForCopy();
		openAppearencePanelOnFirstCell();

		cy.cGet('#border-5').click();

		calcHelper.selectEntireSheet();
		helper.copy();

		cy.cGet('#copy-paste-container table td')
			.should('have.attr', 'style', 'border-top: 1px solid #000000');
	});

	it('Apply bottom border', function() {
		helper.setDummyClipboardForCopy();
		openAppearencePanelOnFirstCell();

		cy.cGet('#border-6').click();

		calcHelper.selectEntireSheet();
		helper.copy();

		cy.cGet('#copy-paste-container table td')
			.should('have.attr', 'style', 'border-bottom: 1px solid #000000');
	});

	it('Apply top and bottom border', function() {
		helper.setDummyClipboardForCopy();
		openAppearencePanelOnFirstCell();

		cy.cGet('#border-7').click();

		calcHelper.selectEntireSheet();
		helper.copy();

		cy.cGet('#copy-paste-container table td')
			.should('have.attr', 'style', 'border-top: 1px solid #000000; border-bottom: 1px solid #000000');
	});

	it('Apply border for all sides', function() {
		helper.setDummyClipboardForCopy();
		openAppearencePanelOnFirstCell();

		cy.cGet('#border-8').click();

		calcHelper.selectEntireSheet();
		helper.copy();

		cy.cGet('#copy-paste-container table td')
			.should('have.attr', 'style', 'border-top: 1px solid #000000; border-bottom: 1px solid #000000; border-left: 1px solid #000000; border-right: 1px solid #000000');
	});

	it.skip('Apply horizontal borders for multiple cells', function() {
		openAppearencePanelOnAllCells();

		cy.cGet('#border-9').click();

		calcHelper.selectEntireSheet();

		cy.cGet('#copy-paste-container table td')
			.should(function(cells) {
				expect(cells).to.have.lengthOf(4);
				for (var i = 0; i < cells.length; i++) {
					expect(cells[i]).to.have.attr('style', 'border-top: 1px solid #000000; border-bottom: 1px solid #000000');
				}
			});
	});

	it.skip('Apply horizontal inner borders and vertical outer borders', function() {
		openAppearencePanelOnAllCells();

		cy.cGet('#border-10').click();

		calcHelper.selectEntireSheet();

		cy.cGet('#copy-paste-container table td')
			.should(function(cells) {
				expect(cells).to.have.lengthOf(4);
				for (var i = 0; i < cells.length; i++) {
					if (i == 0 || i == 2)
						expect(cells[i]).to.have.attr('style', 'border-top: 1px solid #000000; border-bottom: 1px solid #000000; border-left: 1px solid #000000');
					else
						expect(cells[i]).to.have.attr('style', 'border-top: 1px solid #000000; border-bottom: 1px solid #000000');
				}
			});
	});

	it.skip('Apply vertical inner borders and horizontal outer borders', function() {
		openAppearencePanelOnAllCells();

		cy.cGet('#border-11').click();

		calcHelper.selectEntireSheet();

		cy.cGet('#copy-paste-container table td')
			.should(function(cells) {
				expect(cells).to.have.lengthOf(4);
				for (var i = 0; i < cells.length; i++) {
					if (i == 0 || i == 1)
						expect(cells[i]).to.have.attr('style', 'border-top: 1px solid #000000; border-left: 1px solid #000000; border-right: 1px solid #000000');
					else
						expect(cells[i]).to.have.attr('style', 'border-left: 1px solid #000000; border-right: 1px solid #000000');
				}
			});
	});

	it.skip('Apply all inner and outer borders', function() {
		openAppearencePanelOnAllCells();

		cy.cGet('#border-12').click();

		calcHelper.selectEntireSheet();

		cy.cGet('#copy-paste-container table td')
			.should(function(cells) {
				expect(cells).to.have.lengthOf(4);
				for (var i = 0; i < cells.length; i++) {
					expect(cells[i]).to.have.attr('style', 'border-top: 1px solid #000000; border-bottom: 1px solid #000000; border-left: 1px solid #000000; border-right: 1px solid #000000');
				}
			});
	});

	it.skip('Apply border color', function() {
		openAppearencePanelOnFirstCell();

		// Apply left border first
		cy.cGet('#border-2').click();

		// Then apply border color
		cy.cGet('#FrameLineColor')
			.should('not.have.class','disabled');

		cy.cGet('#FrameLineColor > .ui-header')
			.click();

		mobileHelper.selectFromColorPicker('#FrameLineColor', 3);

		// Check that the color is shown as selected
		cy.cGet('#FrameLineColor > .ui-header .color-sample-selected')
			.should('have.attr', 'style', 'background-color: rgb(255, 153, 0);');

		calcHelper.selectEntireSheet();

		cy.cGet('#copy-paste-container table td')
			.should('have.attr', 'style', 'border-left: 1px solid #ff9900');
	});
});
