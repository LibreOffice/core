/* global describe it cy beforeEach require Cypress */

var helper = require('../../common/helper');
var desktopHelper = require('../../common/desktop_helper');
var writerHelper = require('../../common/writer_helper');

describe(['tagdesktop'], 'Top toolbar apply tests.', function() {

	beforeEach(function() {
		cy.viewport(1920,1080);
		helper.setupAndLoadDocument('writer/top_toolbar.odt');
		desktopHelper.switchUIToNotebookbar();

		if (Cypress.env('INTEGRATION') === 'nextcloud') {
			desktopHelper.showSidebar();
		}
		cy.getFrameWindow().then((win) => {
			this.win = win;
		})

		writerHelper.selectAllTextOfDoc();
	});

	function refreshCopyPasteContainer() {
		helper.typeIntoDocument('{rightArrow}');
		writerHelper.selectAllTextOfDoc();
	}

	it('Apply highlight color.', function() {
		helper.setDummyClipboardForCopy();
		desktopHelper.getNbIconArrow('CharBackColor', 'Home').click();
		desktopHelper.selectColorFromPalette('FFB66C');
		writerHelper.selectAllTextOfDoc();
		helper.copy();
		cy.cGet('#copy-paste-container p font span')
			.should('have.attr', 'style', 'background: #ffb66c');
	});

	it('Apply transparent highlight color.', function() {
		helper.setDummyClipboardForCopy();
		desktopHelper.getNbIcon('CharBackColor', 'Home').first().click();
		writerHelper.selectAllTextOfDoc();
		helper.copy();
		cy.cGet('#copy-paste-container p font span')
			.should('have.attr', 'style', 'background: transparent');
		desktopHelper.getNbIcon('CharBackColor').find('.selected-color')
				.should('have.attr', 'style', 'background-color: transparent; border-color: var(--color-border);');
	});

	it('Apply font color.', function() {
		helper.setDummyClipboardForCopy();
		desktopHelper.getNbIconArrow('FontColor', 'Home').click();
		desktopHelper.selectColorFromPalette('3FAF46');
		writerHelper.selectAllTextOfDoc();
		helper.copy();
		cy.cGet('#copy-paste-container p font').should('have.attr', 'color', '#3faf46');
	});

	it('Apply style.', function() {
		helper.setDummyClipboardForCopy();
		cy.cGet('#stylesview').scrollTo('bottom');
		cy.cGet('#stylesview .notebookbar.ui-iconview-entry img[title=Title]').first().scrollIntoView().should('be.visible').click();
		refreshCopyPasteContainer();
		helper.copy();
		cy.cGet('#copy-paste-container p font font').should('have.attr', 'style', 'font-size: 28pt');
	});

	it('Apply font name.', function() {
		helper.setDummyClipboardForCopy();
		desktopHelper.actionOnSelector('fontName', (selector) => { cy.cGet(selector).click(); });
		cy.cGet('[id$="-dropdown"].modalpopup span').contains('Alef').scrollIntoView();
		desktopHelper.selectFromJSDialogListbox('Alef', true);
		refreshCopyPasteContainer();
		helper.copy();
		cy.cGet('#copy-paste-container p font').should('have.attr', 'face', 'Alef');
	});

	it('Apply bold font.', function() {
		helper.setDummyClipboardForCopy();
		desktopHelper.getNbIcon('Bold').click();
		writerHelper.selectAllTextOfDoc();
		helper.copy();
		cy.cGet('#copy-paste-container p b').should('exist');
	});

	it('Apply italic font.', function() {
		helper.setDummyClipboardForCopy();
		desktopHelper.getNbIcon('Italic').click();
		writerHelper.selectAllTextOfDoc();
		helper.copy();
		cy.cGet('#copy-paste-container p i').should('exist');
	});

	it('Apply underline.', function() {
		helper.setDummyClipboardForCopy();
		desktopHelper.getNbIcon('Underline').click();
		writerHelper.selectAllTextOfDoc();
		helper.copy();
		cy.cGet('#copy-paste-container p u').should('exist');
	});

	it('Apply strikethrough.', function() {
		helper.setDummyClipboardForCopy();
		desktopHelper.getNbIcon('Strikeout').click();
		writerHelper.selectAllTextOfDoc();
		helper.copy();
		cy.cGet('#copy-paste-container p strike').should('exist');
	});

	it('Apply font size', function() {
		helper.setDummyClipboardForCopy();
		desktopHelper.actionOnSelector('fontSize', (selector) => { cy.cGet(selector).click(); });
		desktopHelper.selectFromJSDialogListbox('72', false);
		refreshCopyPasteContainer();
		helper.copy();
		cy.cGet('#copy-paste-container p font').should('have.attr', 'style', 'font-size: 72pt');
	});

	it('Apply left/right alignment.', function() {
		desktopHelper.getNbIcon('Bold').click();
		writerHelper.selectAllTextOfDoc();
		//cy.cGet('#copy-paste-container p').should('have.attr', 'align', 'right');
		desktopHelper.getNbIcon('RightPara', 'Home').click();
		writerHelper.selectAllTextOfDoc();
		//cy.cGet('#copy-paste-container p').should('have.attr', 'align', 'left');
	});

	it('Apply center alignment.', function() {
		desktopHelper.getNbIcon('CenterPara', 'Home').click();
		writerHelper.selectAllTextOfDoc();
		//cy.cGet('#copy-paste-container p').should('have.attr', 'align', 'center');
	});

	it('Apply justified.', function() {
		helper.setDummyClipboardForCopy();
		desktopHelper.getNbIcon('JustifyPara', 'Home').click();
		writerHelper.selectAllTextOfDoc();
		helper.copy();
		cy.cGet('#copy-paste-container p').should('have.attr', 'align', 'justify');
	});

	function selectLineSpacing(entry) {
		desktopHelper.getNbIcon('LineSpacing').click();
		desktopHelper.getDropdown('home-line-spacing').contains('.ui-combobox-entry', entry).click();
	}

	it('Apply Line spacing: 1 and 1.5', function() {
		helper.setDummyClipboardForCopy();

		selectLineSpacing('Line Spacing: 1.5');

		writerHelper.selectAllTextOfDoc();
		helper.copy();
		cy.cGet('#copy-paste-container p').should('have.attr', 'style').should('contain', 'line-height: 150%');

		selectLineSpacing('Line Spacing: 1');

		writerHelper.selectAllTextOfDoc();
		helper.copy();
		cy.cGet('#copy-paste-container p').should('have.attr', 'style').should('contain', 'line-height: 100%');
	});

	it('Apply Line spacing: 2', function() {
		helper.setDummyClipboardForCopy();
		selectLineSpacing('Line Spacing: 2');
		writerHelper.selectAllTextOfDoc();
		helper.copy();
		cy.cGet('#copy-paste-container p').should('have.attr', 'style').should('contain', 'line-height: 200%');
	});

	it('Apply superscript.', function() {
		helper.setDummyClipboardForCopy();
		writerHelper.selectAllTextOfDoc();
		desktopHelper.getNbIcon('SuperScript').click();
		cy.cGet('#document-container').click('center');
		writerHelper.selectAllTextOfDoc();
		helper.copy();
		cy.cGet('#copy-paste-container p sup').should('exist');
	});

	it('Apply subscript.', function() {
		helper.setDummyClipboardForCopy();
		writerHelper.selectAllTextOfDoc();
		desktopHelper.getNbIcon('SubScript').click();
		cy.cGet('#document-container').click('center');
		writerHelper.selectAllTextOfDoc();
		helper.copy();
		cy.cGet('#copy-paste-container p sub').should('exist');
	});
});
