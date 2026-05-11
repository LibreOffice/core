/* -*- js-indent-level: 8 -*- */
/* global describe it cy require expect beforeEach*/

var helper = require('../../common/helper');
var impressHelper = require('../../common/impress_helper');
var desktopHelper = require('../../common/desktop_helper');

describe(['tagdesktop', 'tagnextcloud', 'tagproxy'], 'Slide operations', function() {

	beforeEach(function() {
		helper.setupAndLoadDocument('impress/slide_operations.odp');
		desktopHelper.switchUIToNotebookbar();
		cy.getFrameWindow().then((win) => {
			this.win = win;
		});
	});

	it('Add slides', function() {
		cy.cGet('#presentation-toolbar #insertpage').click();

		impressHelper.assertSlidePreviewCountAfterIdle(this.win, 2);
	});

	it('Remove slides', function() {
		// Add slides
		cy.cGet('#presentation-toolbar #insertpage').click();

		impressHelper.assertSlidePreviewCountAfterIdle(this.win, 2);

		// Remove Slides
		cy.cGet('#presentation-toolbar #deletepage')
			.should('not.have.attr', 'disabled');

		cy.cGet('#presentation-toolbar #deletepage')
			.click();

		cy.cGet('#modal-dialog-deleteslide-modal .button-primary').click();

		cy.cGet('#presentation-toolbar #deletepage')
			.should('have.attr', 'disabled')

		impressHelper.assertSlidePreviewCountAfterIdle(this.win, 1);

	});

	it('Check slide sorter focus', function() {
		cy.cGet('#insertpage-button').click();
		helper.processToIdle(this.win);

		// Set the focus to slide sorter.
		cy.cGet('#preview-frame-part-0').click();
		cy.cGet('#preview-frame-part-1').click();

		// Slide sorter should keep focus while user clicks on different slides.
		cy.then(() => {
			expect(this.win.app.map._docLayer._preview.partsFocused).to.equal(true);
		});

		cy.cGet('#toolbar-up').click();
		// Slide sorter should have lost the focus after user clicked somewhere.
		cy.then(() => {
			expect(this.win.app.map._docLayer._preview.partsFocused).to.equal(false);
		});
	});

	it('Slide sorter keeps focus when Alt or Meta is pressed', function() {
		cy.cGet('#insertpage-button').click();
		helper.processToIdle(this.win);

		cy.cGet('#preview-frame-part-0').click();

		cy.then(() => {
			expect(this.win.app.map._docLayer._preview.partsFocused).to.equal(true);
		});

		cy.cGet('#preview-frame-part-0').trigger('keydown', { key: 'Alt', code: 'AltLeft', which: 18 });

		cy.then(() => {
			expect(this.win.app.map._docLayer._preview.partsFocused).to.equal(true);
		});

		cy.cGet('#preview-frame-part-0').trigger('keydown', { key: 'Meta', code: 'MetaLeft', which: 91 });

		cy.then(() => {
			expect(this.win.app.map._docLayer._preview.partsFocused).to.equal(true);
		});
	});

	it('Duplicate slide', function() {
		// Also check if comments are getting duplicated
		desktopHelper.closeNavigatorSidebar();
		desktopHelper.getNbIcon('ModifyPage').click();
		desktopHelper.insertComment();
		cy.cGet('[id^=annotation-content-area-]').should('include.text', 'some text0');
		cy.cGet('#Insert-tab-label').click();
		desktopHelper.getNbIcon('DuplicatePage', 'Insert').click();

		impressHelper.assertSlidePreviewCountAfterIdle(this.win, 2);
		cy.cGet('#SlideStatus').should('have.text', 'Slide 2 of 2');
		cy.cGet('[id^=annotation-content-area-]').should('include.text', 'some text0');

	});

	it('Navigator height test', function() {
		var navigationContainer, navOptionContainer, presentationWrapper, navHeading;

		cy.cGet('.navigation-header')
			.then(function(items) {
				expect(items).to.have.lengthOf(1);
				navHeading = items[0].getBoundingClientRect();
			});

		cy.cGet('.navigation-options-container')
			.then(function(items) {
				expect(items).to.have.lengthOf(1);
				navOptionContainer = items[0].getBoundingClientRect();
			});


		cy.cGet('#presentation-controls-wrapper')
			.then(function(items) {
				expect(items).to.have.lengthOf(1);
				presentationWrapper = items[0].getBoundingClientRect();
			});

		cy.cGet('#navigation-sidebar')
			.then(function(items) {
				expect(items).to.have.lengthOf(1);
				navigationContainer = items[0].getBoundingClientRect();
				expect(navigationContainer.height).equal(navHeading.height + navOptionContainer.height + presentationWrapper.height);
			});
	});
});
