/* global describe it cy beforeEach require */

var helper = require('../../common/helper');
var mobileHelper = require('../../common/mobile_helper');
var impressHelper = require('../../common/impress_helper');

describe(['tagmobile', 'tagnextcloud', 'tagproxy'], 'Apply paragraph properties on selected shape.', function() {

	beforeEach(function() {
		helper.setupAndLoadDocument('impress/apply_paragraph_props_shape.odp');

		mobileHelper.enableEditingMobile();

		impressHelper.selectTextShapeInTheCenter();
	});

	function triggerNewSVG() {
		mobileHelper.closeMobileWizard();

		cy.wait(1000);

		impressHelper.triggerNewSVGForShapeInTheCenter();

		cy.wait(1000);
	}

	function openParagraphPropertiesPanel() {
		mobileHelper.openMobileWizard();

		cy.cGet('#ParaPropertyPanel').click();

		cy.cGet('#ParaPropertyPanel .unoParaLeftToRight').should('be.visible');
	}

	function openListsPropertiesPanel() {
		mobileHelper.openMobileWizard();

		cy.cGet('#ListsPropertyPanel').click();

		cy.cGet('#ListsPropertyPanel .unoDefaultBullet').should('be.visible');
	}

	it.skip('Apply left/right alignment on text shape.', function() {
		cy.cGet('#document-container g.Page .TextParagraph .TextPosition')
			.should('have.attr', 'x', '1400');

		// Set right alignment first
		openParagraphPropertiesPanel();

		cy.cGet('.unoRightPara').click();

		triggerNewSVG();

		cy.cGet('#document-container g.Page .TextParagraph .TextPosition')
			.should('have.attr', 'x', '23586');

		// Set left alignment
		openParagraphPropertiesPanel();

		cy.cGet('.unoLeftPara').click();

		triggerNewSVG();

		cy.cGet('#document-container g.Page .TextParagraph .TextPosition')
			.should('have.attr', 'x', '1400');
	});

	it.skip('Apply center alignment on text shape.', function() {
		cy.cGet('#document-container g.Page .TextParagraph .TextPosition')
			.should('have.attr', 'x', '1400');

		openParagraphPropertiesPanel();

		cy.cGet('.unoCenterPara').click();

		triggerNewSVG();

		cy.cGet('#document-container g.Page .TextParagraph .TextPosition')
			.should('have.attr', 'x', '12493');
	});

	it.skip('Apply justified alignment on text shape.', function() {
		cy.cGet('#document-container g.Page .TextParagraph .TextPosition')
			.should('have.attr', 'x', '1400');

		// Set right alignment first
		openParagraphPropertiesPanel();

		cy.cGet('.unoRightPara').click();

		triggerNewSVG();

		cy.cGet('#document-container g.Page .TextParagraph .TextPosition')
			.should('have.attr', 'x', '23586');

		// Then set justified alignment
		openParagraphPropertiesPanel();

		cy.cGet('.unoJustifyPara').click();

		triggerNewSVG();

		cy.cGet('#document-container g.Page .TextParagraph .TextPosition')
			.should('have.attr', 'x', '1400');
	});

	it.skip('Set top/bottom alignment on text shape.', function() {
		cy.cGet('#document-container g.Page .TextParagraph .TextPosition')
			.should('have.attr', 'y', '4834');

		// Set bottom alignment first
		openParagraphPropertiesPanel();

		cy.cGet('.unoCellVertBottom').click();

		triggerNewSVG();

		cy.cGet('#document-container g.Page .TextParagraph .TextPosition')
			.should('have.attr', 'y', '10811');

		// Then set top alignment
		openParagraphPropertiesPanel();

		cy.cGet('.unoCellVertTop').click();

		triggerNewSVG();

		cy.cGet('#document-container g.Page .TextParagraph .TextPosition')
			.should('have.attr', 'y', '4834');
	});

	it.skip('Apply center vertical alignment on text shape.', function() {
		cy.cGet('#document-container g.Page .TextParagraph .TextPosition')
			.should('have.attr', 'y', '4834');

		openParagraphPropertiesPanel();

		cy.cGet('.unoCellVertCenter').click();

		triggerNewSVG();

		cy.cGet('#document-container g.Page .TextParagraph .TextPosition')
			.should('have.attr', 'y', '7822');
	});

	it('Apply default bulleting on text shape.', function() {
		// We have no bulleting by default
		cy.cGet('#document-container g.Page .BulletChars')
			.should('not.exist');

		openListsPropertiesPanel();

		cy.cGet('#ListsPropertyPanel .unoDefaultBullet').click();

		triggerNewSVG();

		cy.cGet('#document-container g.Page .BulletChars')
			.should('exist');
	});

	it('Apply default numbering on text shape.', function() {
		// We have no bulleting by default
		cy.cGet('#document-container g.Page .SVGTextShape tspan')
			.should('not.have.attr', 'ooo:numbering-type');

		openListsPropertiesPanel();

		cy.cGet('#ListsPropertyPanel .unoDefaultNumbering').click();

		triggerNewSVG();

		cy.cGet('#document-container g.Page .SVGTextShape tspan')
			.should('have.attr', 'ooo:numbering-type', 'number-style');
	});

	it.skip('Apply spacing above on text shape.', function() {
		cy.cGet('#document-container g.Page .TextParagraph:nth-of-type(2) tspan')
			.should('have.attr', 'y', '6600');

		openParagraphPropertiesPanel();

		helper.typeIntoInputField('#aboveparaspacing input', '2', true, false);

		triggerNewSVG();

		cy.cGet('#document-container g.Page .TextParagraph:nth-of-type(2) tspan')
			.should('have.attr', 'y', '11180');
	});

	it.skip('Apply spacing below on text shape.', function() {
		cy.cGet('#document-container g.Page .TextParagraph:nth-of-type(2) tspan')
			.should('have.attr', 'y', '6600');

		openParagraphPropertiesPanel();

		helper.typeIntoInputField('#belowparaspacing input', '2', true, false);

		triggerNewSVG();

		cy.cGet('#document-container g.Page .TextParagraph:nth-of-type(2) tspan')
			.should('have.attr', 'y', '11180');
	});
});
