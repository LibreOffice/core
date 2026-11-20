/* global describe cy it beforeEach require */

var helper = require('../../common/helper');
var mobileHelper = require('../../common/mobile_helper');

describe(['tagmobile', 'tagnextcloud', 'tagproxy'], 'Delete Objects', function() {

	beforeEach(function() {
		helper.setupAndLoadDocument('impress/delete_objects.odp', false, false, undefined, 'vector=false');

		mobileHelper.enableEditingMobile();

		cy.getFrameWindow().then((win) => {
			this.win = win;
		});
	});

	it('Delete Text', function() {
		helper.setDummyClipboardForCopy();

		cy.cGet('#document-container').dblclick('center');

		// Wait for text edit mode (cursor should appear)
		cy.cGet('.cursor-overlay .blinking-cursor').should('be.visible');

		helper.typeIntoDocument('text');

		helper.selectAllText();
		helper.processToIdle(this.win);

		helper.copy();

		helper.expectTextForClipboard('text');

		helper.typeIntoDocument('{del}');

		helper.typeIntoDocument('{ctrl}a');
		helper.processToIdle(this.win);

		helper.textSelectionShouldNotExist();
	});


	it('Delete Shapes', function() {
		mobileHelper.openInsertionWizard();

		// Do insertion
		cy.cGet('body').contains('.menu-entry-with-icon', 'Shape').click();

		cy.cGet('.col.w2ui-icon.basicshapes_rectangle').click();

		// Check that the shape is there
		cy.cGet('#document-container svg g').should('exist');

		//cy.cGet('#document-container svg')
		//	.then(function(svg) {
		//		expect(svg[0].getBoundingClientRect().width).to.be.greaterThan(0);
		//		expect(svg[0].getBoundingClientRect().height).to.be.greaterThan(0);
		//	});

		//deletion

		cy.cGet('#document-container').trigger('contextmenu', 'center');

		cy.cGet('body').contains('.menu-entry-with-icon', 'Delete')
			.should('be.visible').click();

		cy.cGet('#document-container svg g').should('not.exist');
	});

	it('Delete Table', function() {
		mobileHelper.openInsertionWizard();

		// Open Table submenu
		cy.cGet('body').contains('.ui-header.level-0.mobile-wizard.ui-widget', 'Table')
			.click();

		cy.cGet('.mobile-wizard.ui-text')
			.should('be.visible');

		// Push insert table button
		cy.cGet('.inserttablecontrols button')
			.should('be.visible')
			.click();

		// Table is inserted with the markers shown
		cy.cGet('.table-column-resize-marker')
			.should('exist');

		//delete
		cy.cGet('#document-container').trigger('contextmenu', 'center');

		cy.cGet('.menu-entry-icon.delete').parent()
			.click();

		cy.cGet('.table-column-resize-marker')
			.should('not.exist');
	});

	it('Delete Fontwork', function() {
		mobileHelper.openInsertionWizard();

		// Do insertion
		cy.cGet('body').contains('.menu-entry-with-icon', 'Fontwork...')
			.click();

		cy.cGet('#FontworkGalleryDialog').should('exist');
		cy.cGet('#FontworkGalleryDialog #ok.ui-pushbutton-wrapper button').click();
		cy.cGet('#FontworkGalleryDialog').should('not.exist');

		helper.processToIdle(this.win);

		cy.cGet('#test-div-shapeHandlesSection')
			.should('exist');

		cy.cGet('#document-container').trigger('contextmenu', 'center');

		cy.cGet('body').contains('.menu-entry-with-icon', 'Delete')
			.should('be.visible').click();

		cy.cGet('#test-div-shapeHandlesSection')
			.should('not.exist');
	});

	it('Delete Chart' , function() {
		mobileHelper.openInsertionWizard();

		cy.cGet('body').contains('.menu-entry-with-icon', 'Chart...')
			.click();

		cy.cGet('#test-div-shapeHandlesSection')
			.should('exist');

		cy.cGet('#test-div-shape-handle-1').then(function(items) {
			const rect = items[0].getBoundingClientRect();
			const x = rect.left + rect.width / 2;
			const y = rect.bottom + 30;
			cy.cGet('#document-canvas').trigger('contextmenu', x, y);
		});

		cy.cGet('body').contains('.menu-entry-with-icon', 'Delete')
			.should('be.visible').click();

		cy.cGet('#test-div-shapeHandlesSection')
			.should('not.exist');
	});
});
