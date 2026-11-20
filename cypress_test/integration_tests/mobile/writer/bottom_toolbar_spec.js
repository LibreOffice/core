/* global describe it cy beforeEach require */

var helper = require('../../common/helper');
var mobileHelper = require('../../common/mobile_helper');
var writerHelper = require('../../common/writer_helper');

describe.skip('Pushing bottom toolbar items.', function() {

	beforeEach(function() {
		helper.setupAndLoadDocument('writer/bottom_toolbar.odt', false, false, undefined, 'vector=false');

		mobileHelper.enableEditingMobile();

		writerHelper.selectAllTextOfDoc();
	});

	it('Apply bold.', function() {
		cy.cGet('#bold').should('not.have.class', 'checked');
		cy.cGet('#bold').click();
		cy.cGet('#bold').should('have.class', 'checked');
		writerHelper.selectAllTextOfDoc();
		cy.cGet('#copy-paste-container p b').should('exist');
	});

	it('Apply italic.', function() {
		cy.cGet('#italic').should('not.have.class', 'checked');
		cy.cGet('#italic').click();
		cy.cGet('#italic').should('have.class', 'checked');
		writerHelper.selectAllTextOfDoc();
		cy.cGet('#copy-paste-container p i').should('exist');
	});
	it('Apply underline.', function() {
		cy.cGet('#underline').should('not.have.class', 'checked');
		cy.cGet('#underline').click();
		cy.cGet('#underline').should('have.class', 'checked');

		writerHelper.selectAllTextOfDoc();

		cy.cGet('#copy-paste-container p u').should('exist');
	});

	it.skip('Apply strikeout.', function() {
		cy.cGet('#strikeout').should('not.have.class', 'checked');
		cy.cGet('#strikeout').click();
		cy.cGet('#strikeout').should('have.class', 'checked');

		writerHelper.selectAllTextOfDoc();

		cy.cGet('#copy-paste-container p strike').should('exist');
	});

	it('Apply text color.', function() {
		cy.cGet('#fontcolor').click();
		mobileHelper.selectFromColorPalette(0, 7);
		cy.cGet('.w2ui-tb-image.w2ui-icon.textcolor').should('have.attr', 'style', 'box-shadow: rgb(255, 255, 255) 0px -2px inset, rgb(0, 0, 255) 0px -6px inset;');
		writerHelper.selectAllTextOfDoc();
		cy.cGet('#copy-paste-container p font').should('have.attr', 'color', '#0000ff');
	});

	it('Apply highlight color.', function() {
		cy.cGet('#backcolor').click();
		mobileHelper.selectFromColorPalette(0, 9);
		cy.cGet('.w2ui-tb-image.w2ui-icon.backcolor').should('have.attr', 'style', 'box-shadow: rgb(255, 255, 255) 0px -2px inset, rgb(255, 0, 255) 0px -6px inset;');
		writerHelper.selectAllTextOfDoc();
		cy.cGet('#copy-paste-container p font span').should('have.attr', 'style', 'background: #ff00ff');
	});

	it.skip('Apply left / right paragraph alignment.', function() {
		cy.cGet('#rightpara').should('not.have.class', 'checked');
		cy.cGet('#rightpara').click();
		cy.cGet('#rightpara').should('have.class', 'checked');
		writerHelper.selectAllTextOfDoc();
		cy.cGet('#copy-paste-container p').should('have.attr', 'align', 'right');
		cy.cGet('#leftpara').should('not.have.class', 'checked');
		cy.cGet('#leftpara').click();
		cy.cGet('#leftpara').should('have.class', 'checked');
		writerHelper.selectAllTextOfDoc();
		cy.cGet('#copy-paste-container p').should('have.attr', 'align', 'left');
	});

	it.skip('Apply center paragraph alignment.', function() {
		cy.cGet('#centerpara').should('not.have.class', 'checked');
		cy.cGet('#centerpara').click();
		cy.cGet('#centerpara').should('have.class', 'checked');
		writerHelper.selectAllTextOfDoc();
		cy.cGet('#copy-paste-container p').should('have.attr', 'align', 'center');
	});

	it.skip('Apply justify paragraph alignment.', function() {
		cy.cGet('#justifypara').should('not.have.class', 'checked');
		cy.cGet('#justifypara').click();
		cy.cGet('#justifypara').should('have.class', 'checked');
		writerHelper.selectAllTextOfDoc();
		cy.cGet('#copy-paste-container p').should('have.attr', 'align', 'justify');
	});

	it('Apply default numbering.', function() {
		cy.cGet('#defaultnumbering').should('not.have.class', 'checked');
		cy.cGet('#defaultnumbering').click();
		cy.cGet('#defaultnumbering').should('have.class', 'checked');
		writerHelper.selectAllTextOfDoc();
		cy.cGet('#copy-paste-container ol li p').should('exist');
	});

	it('Apply default bulleting.', function() {
		cy.cGet('#defaultbullet').should('not.have.class', 'checked');
		cy.cGet('#defaultbullet').click();
		cy.cGet('#defaultbullet').should('have.class', 'checked');
		writerHelper.selectAllTextOfDoc();
		cy.cGet('#copy-paste-container ul li p').should('exist');
	});

	it('Increase / decrease indent.', function() {
		cy.cGet('#incrementindent').click().click();
		writerHelper.selectAllTextOfDoc();
		cy.cGet('#copy-paste-container p').should('have.attr', 'style').should('contain', 'margin-left: 0.98in');
		cy.cGet('#decrementindent').click();
		writerHelper.selectAllTextOfDoc();
		cy.cGet('#copy-paste-container p').should('have.attr', 'style').should('contain', 'margin-left: 0.49in');
	});
});
