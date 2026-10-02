/* global describe it cy beforeEach require */

var helper = require('../../common/helper');
var mobileHelper = require('../../common/mobile_helper');

describe(['tagmobile', 'tagnextcloud', 'tagproxy'], 'Toolbar tests', function() {

	beforeEach(function() {
		helper.setupAndLoadDocument('writer/toolbar.odt');
	});

	it('State of mobile wizard toolbar item.', function() {
		// Mobile wizard toolbar button is disabled by default
		cy.cGet('#toolbar-up #mobile_wizard').should('not.be.visible');
		// Click on edit button
		mobileHelper.enableEditingMobile();
		// Button should be enabled now
		cy.cGet('#toolbar-up #mobile_wizard').should('not.have.class', 'disabled');
	});

	it('State of insertion mobile wizard toolbar item.', function() {
		// Insertion mobile wizard toolbar button is disabled by default
		cy.cGet('#toolbar-up #insertion_mobile_wizard').should('not.be.visible');
		// Click on edit button
		mobileHelper.enableEditingMobile();
		// Button should be enabled now
		cy.cGet('#toolbar-up #insertion_mobile_wizard').should('not.have.attr', 'disabled');
	});

	it('State of comment wizard toolbar item.', function() {
		// Insertion mobile wizard toolbar button is disabled by default
		cy.cGet('#toolbar-up #comment_wizard').should('not.have.attr', 'disabled');
		// Click on edit button
		mobileHelper.enableEditingMobile();
		// Button should be enabled now
		cy.cGet('#toolbar-up #comment_wizard').should('not.have.attr', 'disabled');
	});

	it('State of undo toolbar item.', function() {
		// Insertion mobile wizard toolbar button is disabled by default
		cy.cGet('#toolbar-up .unoUndo').should('have.attr', 'disabled');
		// Click on edit button
		mobileHelper.enableEditingMobile();
		// Button should be still disabled
		cy.cGet('#toolbar-up .unoUndo').should('have.attr', 'disabled');
		// Type something in the document
		helper.typeIntoDocument('x');
		// Button should become enabled
		cy.cGet('#toolbar-up .unoUndo').should('not.have.attr', 'disabled');
	});

	it('State of redo toolbar item.', function() {
		// Insertion mobile wizard toolbar button is disabled by default
		cy.cGet('#toolbar-up .unoRedo').should('have.attr', 'disabled');
		// Click on edit button
		mobileHelper.enableEditingMobile();
		// Button should be still disabled
		cy.cGet('#toolbar-up .unoRedo').should('have.attr', 'disabled');
		// Type something in the document
		helper.typeIntoDocument('x');
		// Button should be still disabled
		cy.cGet('#toolbar-up .unoRedo').should('have.attr', 'disabled');
		// Do an undo
		cy.cGet('#toolbar-up .unoUndo').should('not.have.attr', 'disabled');
		cy.cGet('#toolbar-up .unoUndo').click();
		// Button should become enabled
		cy.cGet('#toolbar-up .unoRedo').should('not.have.attr', 'disabled');
	});

	it('Open and close mobile wizard by toolbar item.', function() {
		// Click on edit button
		mobileHelper.enableEditingMobile();
		// Click on mobile wizard toolbar item
		cy.cGet('#toolbar-up #mobile_wizard').should('not.have.attr', 'disabled');
		cy.cGet('#toolbar-up #mobile_wizard button').click();
		// Mobile wizard is opened and it has any content
		cy.cGet('#mobile-wizard-content').should('not.be.empty');
		// Toolbar button is checked
		cy.cGet('#toolbar-up #mobile_wizard').should('have.class', 'selected');
		cy.cGet('#toolbar-up #mobile_wizard button').click();
		// Mobile wizard is closed
		cy.cGet('#mobile-wizard').should('not.be.visible');
		cy.cGet('#toolbar-up #mobile_wizard').should('not.have.class', 'selected');
	});

	it('Open and close insertion mobile wizard by toolbar item.', function() {
		// Click on edit button
		mobileHelper.enableEditingMobile();
		mobileHelper.openInsertionWizard();
		mobileHelper.closeInsertionWizard();
		mobileHelper.openInsertionWizard();
	});

	it('Open comment wizard by toolbar item.', function() {
		// Click on edit button
		mobileHelper.enableEditingMobile();
		// Click on mobile wizard toolbar item
		cy.cGet('#toolbar-up #comment_wizard').should('not.have.attr', 'disabled');
		cy.cGet('#toolbar-up #comment_wizard button').click();
		// Mobile wizard is opened and it has any content
		cy.cGet('#mobile-wizard-content').should('not.be.empty');
		// Toolbar button is checked
		cy.cGet('#toolbar-up #comment_wizard').should('have.class', 'selected');
		cy.cGet('#toolbar-up #comment_wizard button').click();
		// Mobile wizard is closed
		cy.cGet('#mobile-wizard').should('not.be.visible');
		cy.cGet('#toolbar-up #comment_wizard').should('not.have.class', 'selected');
	});

	it('Hides the top toolbar when the screen is too small', function() {
		// We use #toolbar-wrapper here since as #toolbar-up only has the editing actions, whereas toolbar-wrapper has the back button and hamburger menu as well

		// By default, the top toolbar is shown...
		cy.cGet('#toolbar-wrapper:has(#toolbar-up)').should('be.visible');

		// ...including in edit mode...
		mobileHelper.enableEditingMobile();
		cy.cGet('#toolbar-wrapper:has(#toolbar-up)').should('be.visible');

		// ...even when we are in landscape mode...
		cy.viewport('iphone-6', 'landscape');
		cy.cGet('#toolbar-wrapper:has(#toolbar-up)').should('be.visible');

		// ...and even when the keyboard is shown while on a different part of the UI, here simulated by setting our height just under the limit
		cy.viewport(667, 149);
		cy.cGet('#toolbar-wrapper:has(#toolbar-up)').should('be.visible');

		// ...but when we start editing the document - moving focus into it - the toolbar is hidden to leave space for the keyboard...
		// ...there's no way to know exactly how big the keyboard will be on different devices - particularly when taking into account browser UI - but 150px is still enough to edit the document so it's kind of OK
		helper.typeIntoDocument('x');
		cy.cGet('#toolbar-wrapper:has(#toolbar-up)').should('not.be.visible');

		// ...and closing the keyboard should make it appear again
		cy.viewport('iphone-6', 'landscape');
		cy.cGet('#toolbar-wrapper:has(#toolbar-up)').should('be.visible');
	});
});
