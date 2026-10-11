/* global describe it cy beforeEach require */

var helper = require('../../common/helper');
var mobileHelper = require('../../common/mobile_helper');
var calcHelper = require('../../common/calc_helper');

describe(['tagmobile', 'tagnextcloud', 'tagproxy'], 'Delete Objects',function() {

	beforeEach(function() {
		helper.setupAndLoadDocument('calc/delete_objects.ods');

		// Click on edit button
		mobileHelper.enableEditingMobile();
	});

	it('Delete Text', function() {
		helper.setDummyClipboardForCopy();
		calcHelper.dblClickOnFirstCell();
		helper.typeIntoDocument('text');
		helper.selectAllText();
		helper.copy();
		helper.expectTextForClipboard('text');
		helper.typeIntoDocument('{del}');
		helper.typeIntoDocument('{ctrl}a');
		helper.textSelectionShouldNotExist();
	});

	it('Delete Shapes', function() {
		mobileHelper.openInsertionWizard();

		// Do insertion
		cy.cGet('body').contains('.menu-entry-with-icon', 'Shape').click();
		cy.cGet('.col.w2ui-icon.basicshapes_rectangle').click();

		// Check that the shape is there
		cy.cGet('#document-container svg g').should('exist');

		//deletion
		cy.cGet('#test-div-shape-handle-rotation').should('exist');
		cy.cGet('#document-canvas').then(function(element) {
			const clientRect = element[0].getBoundingClientRect();
			const x = (clientRect.left + clientRect.right) / 2;
			const y = (clientRect.top + clientRect.bottom) / 2;

			// Right click on canvas is safe for canvas sections while testing mobile view. Also, this helps cypress.
			cy.cGet('#document-canvas').rightclick(x, y)
		});

		cy.cGet('body').contains('.menu-entry-with-icon', 'Delete').should('be.visible');
		cy.cGet('body').contains('.menu-entry-with-icon', 'Delete').click();
		cy.cGet('#document-container svg g').should('not.exist');
	});

	it('Delete Chart' , function() {
		mobileHelper.openInsertionWizard();
		cy.cGet('body').contains('.menu-entry-with-icon', 'Chart').click();
		cy.cGet('.mobile-wizard #ChartWizard #ok.ui-pushbutton-wrapper').click();
		cy.cGet('#mobile-wizard').should('not.be.visible');
		cy.cGet('#document-container svg g').should('exist');

		//deletion
		cy.cGet('#document-canvas').then(function(element) {
			const clientRect = element[0].getBoundingClientRect();
			const x = (clientRect.left + clientRect.right) / 2;
			const y = (clientRect.top + clientRect.bottom) / 2;

			cy.cGet('#document-canvas').rightclick(x, y)
		});

		cy.cGet('body').contains('.menu-entry-with-icon', 'Delete')
			.should('be.visible').click();

		cy.cGet('#document-container svg g').should('not.exist');
	});
});
