/* global describe it cy beforeEach require */

var helper = require('../../common/helper');
var mobileHelper = require('../../common/mobile_helper');

describe(['tagmobile', 'tagnextcloud', 'tagproxy'], 'Annotation Tests',function() {
	var newFilePath;

	beforeEach(function() {
		newFilePath = helper.setupAndLoadDocument('calc/annotation.ods');

		// Click on edit button
		mobileHelper.enableEditingMobile();
	});

	it('Saving comment.', function() {
		mobileHelper.insertComment(false, 'Note');
		cy.cGet('#comment-container-1').should('exist');
		mobileHelper.selectHamburgerMenuItem(['File', 'Save']);
		helper.waitUntilDocumentSaved();

		helper.reloadDocument(newFilePath);
		mobileHelper.enableEditingMobile();
		mobileHelper.openCommentWizard();
		cy.cGet('#mobile-wizard-content').should('exist');
		cy.cGet('#annotation-content-area-1').should('have.text', 'some text');
		cy.cGet('#comment-container-1').should('exist');
	});

	it('Modifying comment.', function() {
		mobileHelper.insertComment(false, 'Note');
		cy.cGet('#comment-container-1').should('exist');
		mobileHelper.selectAnnotationMenuItem('Modify');
		cy.cGet('#annotation-content-area-1').should('have.text', 'some text');
		cy.cGet('#input-modal-input').type('{end}');
		cy.cGet('#input-modal-input').type('modified');
		cy.cGet('#response-ok').click();
		cy.cGet('#toolbar-up #comment_wizard').click();
		cy.cGet('#comment-container-1').should('exist');
		cy.cGet('#annotation-content-area-1').should('have.text', 'some textmodified');
	});

	it('Remove comment.', function() {
		mobileHelper.insertComment(false, 'Note');
		cy.cGet('#comment-container-1').should('exist');
		cy.cGet('#annotation-content-area-1').should('have.text', 'some text');
		mobileHelper.selectAnnotationMenuItem('Remove');
		cy.cGet('#annotation-content-area-1').should('not.exist');
		cy.cGet('#comment-container-1').should('not.exist');
	});

	it('Try to insert empty comment.', function() {
		mobileHelper.openInsertionWizard();
		cy.cGet('body').contains('.menu-entry-with-icon', 'Note').click();
		cy.cGet('.cool-annotation-table').should('exist');
		cy.cGet('#input-modal-input').should('have.text', '');
		cy.cGet('#response-ok').click();
		cy.cGet('.cool-annotation-content-wrapper.wizard-comment-box').should('not.exist');
		cy.cGet('.wizard-comment-box .cool-annotation-content').should('not.exist');
	});
});
