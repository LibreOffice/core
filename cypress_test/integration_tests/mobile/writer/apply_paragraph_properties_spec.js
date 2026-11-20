/* global describe it cy beforeEach require */

var helper = require('../../common/helper');
var mobileHelper = require('../../common/mobile_helper');
var writerHelper = require('../../common/writer_helper');

describe.skip('Apply paragraph properties.', function() {

	beforeEach(function() {
		helper.setupAndLoadDocument('writer/apply_paragraph_properties.odt', false, false, undefined, 'vector=false');
		mobileHelper.enableEditingMobile();
		writerHelper.selectAllTextOfDoc();
		mobileHelper.openMobileWizard();
		// Open paragraph properties
		cy.cGet('#Paragraph').click();
		cy.cGet('#Paragraph').should('have.class', 'selected');
		cy.cGet('#LeftPara').should('be.visible');
	});

	/*it('Apply left/right alignment.', function() {
		cy.cGet('#RightPara').click();

		mobileHelper.closeMobileWizard();

		writerHelper.selectAllTextOfDoc();

		cy.get('#copy-paste-container p')
			.should('have.attr', 'align', 'right');

		// Then apply left alignment
		mobileHelper.openMobileWizard();

		cy.cGet('#Paragraph').click();

		cy.get('#Paragraph')
			.should('have.class', 'selected');

		cy.cGet('#LeftPara').click();

		cy.get('#LeftParaimg')
			.should('have.class', 'selected');

		mobileHelper.closeMobileWizard();

		writerHelper.selectAllTextOfDoc();

		cy.get('#copy-paste-container p')
			.should('have.attr', 'align', 'left');
	});

	it('Apply center alignment.', function() {
		cy.cGet('#CenterPara').click();

		writerHelper.selectAllTextOfDoc();

		cy.get('#copy-paste-container p')
			.should('have.attr', 'align', 'center');
	});

	it('Apply justify alignment.', function() {
		cy.cGet('#JustifyPara').click();

		writerHelper.selectAllTextOfDoc();

		cy.get('#copy-paste-container p')
			.should('have.attr', 'align', 'justify');
	});

	it('Change writing direction.', function() {
		cy.cGet('#ParaRightToLeft').click();

		mobileHelper.closeMobileWizard();

		writerHelper.selectAllTextOfDoc();

		cy.get('#copy-paste-container p')
			.should('have.attr', 'dir', 'rtl');

		writerHelper.selectAllTextOfDoc();

		mobileHelper.openMobileWizard();

		// Open paragraph properties
		cy.cGet('#Paragraph').click();

		// Change writing mode
		cy.cGet('#ParaLeftToRight').click();

		mobileHelper.closeMobileWizard();

		writerHelper.selectAllTextOfDoc();

		cy.get('#copy-paste-container p')
			.should('not.have.attr', 'dir');
	});

	it('Apply default bulleting.', function() {
		cy.cGet('#DefaultBullet').click();

		writerHelper.selectAllTextOfDoc();

		cy.get('#copy-paste-container ul li p')
			.should('exist');
	});

	it('Apply default numbering.', function() {
		cy.cGet('#DefaultNumbering').click();

		writerHelper.selectAllTextOfDoc();

		cy.get('#copy-paste-container ol li p')
			.should('exist');
	});*/

	it('Apply background color.', function() {
		cy.cGet('#BackgroundColor .ui-header').click();
		mobileHelper.selectFromColorPalette(4, 5, 5, 2);
		writerHelper.selectAllTextOfDoc();
		cy.cGet('#copy-paste-container p').should('have.attr', 'style').should('contain', 'background: #6aa84f');
	});

	it('Increase / decrease para spacing.', function() {
		// Increase para spacing
		cy.cGet('#ParaspaceIncrease').click();
		cy.cGet('#ParaspaceIncrease').click();
		mobileHelper.closeMobileWizard();
		writerHelper.selectAllTextOfDoc();
		cy.cGet('#copy-paste-container p').should('have.attr', 'style').should('contain', 'margin-top: 0.08in');
		cy.cGet('#copy-paste-container p').should('have.attr', 'style').should('contain', 'margin-bottom: 0.08in');
		writerHelper.selectAllTextOfDoc();
		mobileHelper.openMobileWizard();
		// Open paragraph properties
		cy.cGet('#Paragraph').click();
		// Decrease para spacing
		cy.cGet('#ParaspaceDecrease').click();
		writerHelper.selectAllTextOfDoc();
		cy.cGet('#copy-paste-container p').should('have.attr', 'style').should('contain', 'margin-top: 0.04in');
		cy.cGet('#copy-paste-container p').should('have.attr', 'style').should('contain', 'margin-bottom: 0.04in');
	});

	it('Change para spacing via combobox.', function() {
		// Check para spacing current value
		cy.cGet('#aboveparaspacing .spinfield').should('have.value', '0');
		cy.cGet('#belowparaspacing .spinfield').should('have.value', '0');

		// Change spacing
		cy.cGet('#aboveparaspacing .plus').click();
		cy.cGet('#aboveparaspacing .spinfield').should('have.value', '0.01');
		cy.cGet('#aboveparaspacing .plus').click();
		cy.cGet('#aboveparaspacing .spinfield').should('have.value', '0.02');

		cy.cGet('#aboveparaspacing .plus').click();
		cy.cGet('#aboveparaspacing .spinfield').should('have.value', '0.03');

		cy.cGet('#aboveparaspacing .plus').click();
		cy.cGet('#aboveparaspacing .spinfield').should('have.value', '0.04');

		cy.cGet('#aboveparaspacing .minus').click();
		cy.cGet('#aboveparaspacing .spinfield').should('have.value', '0.03');

		cy.cGet('#belowparaspacing .plus').click();
		cy.cGet('#belowparaspacing .spinfield').should('have.value', '0.01');

		writerHelper.selectAllTextOfDoc();

		cy.cGet('#copy-paste-container p').should('have.attr', 'style').should('contain', 'margin-top: 0.03in');
		cy.cGet('#copy-paste-container p').should('have.attr', 'style').should('contain', 'margin-bottom: 0.01in');
	});

	it('Increase / decrease indent.', function() {
		// Increase indent
		cy.cGet('#IncrementIndent').click();
		cy.cGet('#IncrementIndent').click();
		mobileHelper.closeMobileWizard();
		writerHelper.selectAllTextOfDoc();
		cy.cGet('#copy-paste-container p').should('have.attr', 'style').should('contain', 'margin-left: 0.98in');
		writerHelper.selectAllTextOfDoc();
		mobileHelper.openMobileWizard();
		// Open paragraph properties
		cy.cGet('#Paragraph').click();
		// Decrease indent
		cy.cGet('#DecrementIndent').click();
		writerHelper.selectAllTextOfDoc();
		cy.cGet('#copy-paste-container p').should('have.attr', 'style').should('contain', 'margin-left: 0.49in');
	});

	it('Apply before text indent.', function() {
		// Change indent
		cy.cGet('#beforetextindent .plus').click();
		cy.cGet('#beforetextindent .spinfield').should('have.value', '0.01');

		cy.cGet('#beforetextindent .plus').click();
		cy.cGet('#beforetextindent .spinfield').should('have.value', '0.02');

		writerHelper.selectAllTextOfDoc();

		cy.cGet('#copy-paste-container p').should('have.attr', 'style').should('contain', 'margin-left: 0.02in');
	});

	it('Apply after text indent.', function() {
		// Change indent
		cy.cGet('#aftertextindent .plus').click();
		cy.cGet('#aftertextindent .spinfield').should('have.value', '0.01');

		cy.cGet('#aftertextindent .plus').click();
		cy.cGet('#aftertextindent .spinfield').should('have.value', '0.02');

		writerHelper.selectAllTextOfDoc();

		cy.cGet('#copy-paste-container p').should('have.attr', 'style').should('contain', 'margin-right: 0.02in');
	});

	it('Apply first line indent.', function() {
		// Increase firstline indent
		cy.cGet('#firstlineindent .plus').click();
		cy.cGet('#firstlineindent .spinfield').should('have.value', '0.01');

		cy.cGet('#firstlineindent .plus').click();
		cy.cGet('#firstlineindent .spinfield').should('have.value', '0.02');

		writerHelper.selectAllTextOfDoc();

		cy.cGet('#copy-paste-container p').should('have.attr', 'style').should('contain', 'text-indent: 0.02in');
	});

	it('Linespacing item is hidden.', function() {
		// Linespacing item triggers a drop down menu in core
		// which is not implemented in online yet.
		cy.cGet('#LineSpacing').should('not.exist');
	});
});
