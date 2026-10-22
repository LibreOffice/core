/* global describe it cy beforeEach require */

var helper = require('../../common/helper');
var impressHelper = require('../../common/impress_helper');
const desktopHelper = require('../../common/desktop_helper');

describe(['tagdesktop', 'tagnextcloud', 'tagproxy'], 'Apply paragraph properties on selected shape.', function() {

	beforeEach(function() {
		helper.setupAndLoadDocument('impress/apply_paragraph_props_text.odp');
		desktopHelper.switchUIToCompact();

		cy.cGet('.close-navigation-button').click();
		cy.cGet('#navigator-sidebar').should('not.exist');
		cy.getFrameWindow().then((win) => {
			this.win = win;
		});
	});

	function selectText(win) {
		impressHelper.triggerNewSVGForShapeInTheCenter();
		impressHelper.selectTextOfShape();
		helper.processToIdle(win);
	}

	it.skip('Apply horizontal alignment on selected text.', function() {
		selectText();
		cy.cGet('#document-container g.Page .TextParagraph .TextPosition')
			.should('have.attr', 'x', '1400');

		// Set right alignment
		cy.cGet('#rightpara').click();
		cy.wait(500);

		selectText();
		cy.cGet('#document-container g.Page .TextParagraph .TextPosition[x="23583"], #document-container g.Page .TextParagraph .TextPosition[x="23584"]')
			.should('exist');

		// Set left alignment
		cy.cGet('#leftpara').click();
		cy.wait(500);

		selectText();
		cy.cGet('#document-container g.Page .TextParagraph .TextPosition')
			.should('have.attr', 'x', '1400');

		// Set centered alignment
		cy.cGet('#centerpara').click();
		cy.wait(500);

		selectText();
		cy.cGet('#document-container g.Page .TextParagraph .TextPosition[x="12491"], #document-container g.Page .TextParagraph .TextPosition[x="12492"]')
			.should('exist');

		// Set justified alignment
		cy.cGet('#justifypara').click();
		cy.wait(500);

		selectText();
		cy.cGet('#document-container g.Page .TextParagraph .TextPosition')
			.should('have.attr', 'x', '1400');
	});

	it('Apply default bulleting on selected text.', function() {
		selectText(this.win);

		// We have no bulleting by default
		cy.cGet('#document-container g.Page .BulletChars')
			.should('not.exist');

		desktopHelper.openOverflowGroup('overflow-button-other-toptoolbar');

		// Apply bulleting
		desktopHelper.getCompactIcon('DefaultBullet').click();

		selectText(this.win);
		cy.cGet('#document-container g.Page .BulletChars')
			.should('exist');
	});

	it('Apply default numbering on selected text.', function() {
		selectText(this.win);

		// We have no numbering by default
		cy.cGet('#document-container g.Page .SVGTextShape tspan[ooo\\:numbering-type]')
			.should('not.exist');

		desktopHelper.openOverflowGroup('overflow-button-other-toptoolbar');

		// Apply numbering
		desktopHelper.getCompactIcon('DefaultNumbering').click();

		selectText(this.win);
		cy.cGet('#document-container g.Page .SVGTextShape tspan[ooo\\:numbering-type="number-style"]')
			.should('exist');
	});

	// FIXME: fails on 6600 might be related to recent core margin updates
	it.skip('Increase/decrease spacing of selected text.', function() {
		selectText();
		cy.cGet('#document-container g.Page .TextParagraph:nth-of-type(2) tspan')
			.should('have.attr', 'y', '6600');

		// Increase spacing
		desktopHelper.getCompactIconArrow('LineSpacing').click();
		cy.cGet('#linespacing-dropdown .ui-combobox-entry').contains('Increase Paragraph Spacing').click();

		selectText();
		cy.cGet('#document-container g.Page .TextParagraph:nth-of-type(2) tspan')
			.should('have.attr', 'y', '6700');

		// Decrease spacing
		desktopHelper.getCompactIconArrow('LineSpacing').click();
		cy.cGet('#linespacing-dropdown .ui-combobox-entry').contains('Decrease Paragraph Spacing').click();

		selectText();
		cy.cGet('#document-container g.Page .TextParagraph:nth-of-type(2) tspan')
			.should('have.attr', 'y', '6600');
	});
});
