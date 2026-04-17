/* global describe it cy require beforeEach */

var helper = require('../../common/helper');
var impressHelper = require('../../common/impress_helper');
var mobileHelper = require('../../common/mobile_helper');
var repairHelper = require('../../common/repair_document_helper');

describe.skip(['tagmobile'], 'Trigger hamburger menu options.', function() {
	var newFilePath;

	beforeEach(function() {
		newFilePath = helper.setupAndLoadDocument('impress/hamburger_menu.odp');

		// Click on edit button
		mobileHelper.enableEditingMobile();
		cy.getFrameWindow().then((win) => {
			this.win = win;
		});
	});

	it('Save', { defaultCommandTimeout: 60000 }, function() {
		// Change the document content and save it
		impressHelper.selectTextShapeInTheCenter();

		cy.cGet('#document-container g.Page .TextPosition tspan')
			.should('have.text', 'X');

		// Type a new text
		impressHelper.dblclickOnSelectedShape();

		helper.typeIntoDocument('new');

		impressHelper.triggerNewSVGForShapeInTheCenter();

		cy.cGet('#document-container g.Page .TextPosition tspan')
			.should('have.text', 'Xnew');

		mobileHelper.selectHamburgerMenuItem(['File', 'Save']);

		// Reopen the document and check content.
		helper.reloadDocument(newFilePath);

		mobileHelper.enableEditingMobile();

		impressHelper.selectTextShapeInTheCenter();

		cy.cGet('#document-container g.Page .TextPosition tspan')
			.should('have.text', 'Xnew');
	});

	it('Print', function() {
		// A new window should be opened with the PDF.
		cy.getFrameWindow()
			.then(function(win) {
				cy.stub(win, 'open');
			});

		mobileHelper.selectHamburgerMenuItem(['File', 'Print']);

		cy.getFrameWindow().its('open').should('be.called');
	});

	it('Download as PDF', function() {
		mobileHelper.selectHamburgerMenuItem(['Download as', 'PDF Document (.pdf)']);
		mobileHelper.pressPushButtonOfDialog('Export');

		cy.cGet('iframe')
			.should('have.attr', 'data-src')
			.should('contain', 'download');
	});

	it('Download as ODP', function() {
		mobileHelper.selectHamburgerMenuItem(['Download as', 'ODF presentation (.odp)']);

		cy.cGet('iframe')
			.should('have.attr', 'data-src')
			.should('contain', 'download');
	});

	it('Download as PPT', function() {
		mobileHelper.selectHamburgerMenuItem(['Download as', 'PowerPoint 2003 Presentation (.ppt)']);

		cy.cGet('iframe')
			.should('have.attr', 'data-src')
			.should('contain', 'download');
	});

	it('Download as PPTX', function() {
		mobileHelper.selectHamburgerMenuItem(['Download as', 'PowerPoint Presentation (.pptx)']);

		cy.cGet('iframe')
			.should('have.attr', 'data-src')
			.should('contain', 'download');
	});

	it('Undo/redo.', function() {
		impressHelper.selectTextShapeInTheCenter();

		cy.cGet('#document-container g.Page .TextPosition tspan')
			.should('have.text', 'X');

		// Type a new character
		impressHelper.dblclickOnSelectedShape();

		helper.typeIntoDocument('q');

		impressHelper.triggerNewSVGForShapeInTheCenter();

		cy.cGet('#document-container g.Page .TextPosition tspan')
			.should('have.text', 'Xq');

		// Undo
		mobileHelper.selectHamburgerMenuItem(['Edit', 'Undo']);

		impressHelper.triggerNewSVGForShapeInTheCenter();

		cy.cGet('#document-container g.Page .TextPosition tspan')
			.should('have.text', 'X');

		// Redo
		mobileHelper.selectHamburgerMenuItem(['Edit', 'Redo']);

		impressHelper.triggerNewSVGForShapeInTheCenter();

		cy.cGet('#document-container g.Page .TextPosition tspan')
			.should('have.text', 'Xq');
	});

	it('Repair.', function() {
		impressHelper.selectTextShapeInTheCenter();

		cy.cGet('#document-container g.Page .TextPosition tspan')
			.should('have.text', 'X');

		// Type a new character
		impressHelper.dblclickOnSelectedShape();

		helper.typeIntoDocument('q');

		impressHelper.triggerNewSVGForShapeInTheCenter();

		cy.cGet('#document-container g.Page .TextPosition tspan')
			.should('have.text', 'Xq');

		repairHelper.rollbackPastChange('Undo', undefined, true);

		impressHelper.triggerNewSVGForShapeInTheCenter();

		cy.cGet('#document-container g.Page .TextPosition tspan')
			.should('have.text', 'X');
	});

	it('Cut.', function() {
		impressHelper.selectTextShapeInTheCenter();
		impressHelper.selectTextOfShape();

		mobileHelper.selectHamburgerMenuItem(['Edit', 'Cut']);

		cy.cGet('#mobile-wizard-content-modal-dialog-copy_paste_warning-box').should('exist');
	});

	it('Copy.', function() {
		impressHelper.selectTextShapeInTheCenter();
		impressHelper.selectTextOfShape();

		mobileHelper.selectHamburgerMenuItem(['Edit', 'Copy']);

		cy.cGet('#mobile-wizard-content-modal-dialog-copy_paste_warning-box').should('exist');
	});

	it('Paste.', function() {
		impressHelper.selectTextShapeInTheCenter();
		impressHelper.selectTextOfShape();

		mobileHelper.selectHamburgerMenuItem(['Edit', 'Paste']);

		cy.cGet('#mobile-wizard-content-modal-dialog-copy_paste_warning-box').should('exist');
	});

	it('Select all.', function() {
		impressHelper.selectTextShapeInTheCenter();

		impressHelper.dblclickOnSelectedShape();

		cy.cGet('#copy-paste-container pre')
			.should('not.exist');

		mobileHelper.selectHamburgerMenuItem(['Edit', 'Select All']);

		helper.textSelectionShouldExist();

		helper.expectTextForClipboard('X');
	});

	it.skip('Search some word.', function() {
		mobileHelper.selectHamburgerMenuItem(['Search']);

		// Search bar become visible
		cy.cGet('#mobile-wizard-content')
			.should('not.be.empty');

		// Search for some word
		helper.typeIntoInputField('#searchterm', 'X');

		cy.cGet('#search')
			.should('not.have.attr', 'disabled');

		cy.cGet('#search').click();

		// A shape and some text should be selected
		//cy.get('.transform-handler--rotate')
		//	.should('be.not.visible');
		cy.cGet('.text-selection-handle-start')
			.should('be.visible');
	});

	it('Slide: New Slide.', function() {
		impressHelper.assertSlidePreviewCountAfterIdle(this.win, 1);

		mobileHelper.selectHamburgerMenuItem(['Slide', 'New Slide']);

		impressHelper.assertSlidePreviewCountAfterIdle(this.win, 2);
	});

	it('Slide: Duplicate Slide.', function() {
		impressHelper.assertSlidePreviewCountAfterIdle(this.win, 1);

		mobileHelper.selectHamburgerMenuItem(['Slide', 'Duplicate Slide']);

		impressHelper.assertSlidePreviewCountAfterIdle(this.win, 2);
	});

	it('Slide: Delete Slide.', function() {
		impressHelper.assertSlidePreviewCountAfterIdle(this.win, 1);

		mobileHelper.selectHamburgerMenuItem(['Slide', 'New Slide']);

		impressHelper.assertSlidePreviewCountAfterIdle(this.win, 2);

		mobileHelper.selectHamburgerMenuItem(['Slide', 'Delete Slide']);

		cy.cGet('#mobile-wizard-content-modal-dialog-deleteslide-modal').should('exist');
		cy.cGet('#deleteslide-modal-response').click();
		cy.cGet('#mobile-wizard-content-modal-dialog-deleteslide-modal').should('not.exist');

		impressHelper.assertSlidePreviewCountAfterIdle(this.win, 1);
	});

	it('Full Screen.', function() {
		mobileHelper.selectHamburgerMenuItem(['Full Screen']);

		// TODO: We can't hit the actual full screen from cypress
		cy.wait(500);
	});

	it('Fullscreen presentation.', function() {
		cy.cGet('iframe.leaflet-slideshow')
			.should('not.exist');

		mobileHelper.selectHamburgerMenuItem(['Fullscreen presentation']);

		cy.cGet('iframe.leaflet-slideshow')
			.should('exist');
	});

	it('Check version information.', function() {
		mobileHelper.selectHamburgerMenuItem(['About']);

		cy.cGet('#mobile-wizard-content')
			.should('exist');

		// Check the version
		cy.cGet('body').find('#coolwsd-version').should('exist');

		// Close about dialog
		cy.cGet('div.mobile-wizard.jsdialog-overlay.cancellable').click({force : true});
	});
});
