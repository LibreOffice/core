/* global describe it cy beforeEach require expect*/

var helper = require('../../common/helper');
var calcHelper = require('../../common/calc_helper');
var mobileHelper = require('../../common/mobile_helper');

describe(['tagmobile', 'tagnextcloud'], 'Calc insertion wizard.', function() {

	beforeEach(function() {
		helper.setupAndLoadDocument('calc/insertion_wizard.ods');

		// Click on edit button
		mobileHelper.enableEditingMobile();

		calcHelper.clickOnFirstCell();

		mobileHelper.openInsertionWizard();
	});

	it('Inset local image.', function() {
		// We can't use the menu item directly, because it would open file picker.
		cy.cGet('body').contains('.menu-entry-with-icon', 'Local Image...')
			.should('be.visible');

		cy.cGet('#insertgraphic[type=file]')
			.attachFile('/mobile/calc/image_to_insert.png');

		// The image is not selected after insertion, so wait for it to
		// reach the document instead.
		cy.cGet('#document-container svg g.Graphic').should('exist');

		// Select image
		cy.cGet('#test-div-cell_selection_handle_end')
			.then(function(items) {
				expect(items).to.have.lengthOf(1);
				var XPos = items[0].getBoundingClientRect().right + 10;
				var YPos = items[0].getBoundingClientRect().top;
				cy.cGet('body').click(XPos, YPos);
			});

		cy.cGet('#document-container svg g.Graphic').should('exist');
	});

	it('Insert chart.', function() {
		cy.cGet('body').contains('.menu-entry-with-icon', 'Chart...')
			.click();

		// TODO: why we have 32 markers here instead of 8?
		cy.cGet('#test-div-shape-handle-rotation').should('exist');
	});

	it('Insert hyperlink.', function() {
		helper.setDummyClipboardForCopy();
		cy.cGet('body').contains('.menu-entry-with-icon', 'Hyperlink...')
			.click();

		// Dialog is opened
		cy.cGet('#target-input')
			.should('exist');

		// Type text and link
		cy.cGet('#indication-input')
			.clear()
			.type('some text');
		cy.cGet('#target-input')
			.type('www.something.com');

		// Insert
		cy.cGet('#ok')
			.click();

		cy.cGet('.blinking-cursor')
			.should('be.visible');

		calcHelper.selectEntireSheet();
		helper.copy();

		cy.cGet('#copy-paste-container table td a')
			.should('have.text', 'some text');

		cy.cGet('#copy-paste-container table td a')
			.should('have.attr', 'href', 'http://www.something.com/');
	});

	it('Insert shape.', function() {
		// Do insertion
		cy.cGet('body').contains('.menu-entry-with-icon', 'Shape').click();

		cy.cGet('.basicshapes_ellipse').click();

		// Check that the shape is there
		cy.cGet('#document-container svg g').should('exist');

		// This should pass but Cypress can't get the correct width and height.
		//cy.cGet('#document-container svg svg').should(function(svg) {
		//		expect(svg[0].getBoundingClientRect().width).to.be.greaterThan(0);
		//		expect(svg[0].getBoundingClientRect().height).to.be.greaterThan(0);
		//	});
	});

	it('Insert date.', function() {
		helper.setDummyClipboardForCopy();
		// Do insertion
		cy.cGet('body').contains('.menu-entry-with-icon', 'Date')
			.click();

		calcHelper.selectEntireSheet();
		helper.copy();

		var regex = new RegExp(';MM/DD/YY$');
		cy.cGet('#copy-paste-container table td')
			.should('have.attr', 'sdnum')
			.should('match', regex);
	});

	it('Insert time.', function() {
		helper.setDummyClipboardForCopy();
		// Do insertion
		cy.cGet('body').contains('.menu-entry-with-icon', 'Time')
			.click();

		calcHelper.selectEntireSheet();
		helper.copy();

		var regex = new RegExp(';HH:MM:SS AM/PM$');
		cy.cGet('#copy-paste-container table td')
			.should('have.attr', 'sdnum')
			.should('match', regex);
	});
});
