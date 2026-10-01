/* global describe it cy beforeEach require expect */

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

	it('Leave the empty editor with the back button.', function() {
		// Long press on a cell and pick Insert Note from the context menu.
		cy.cGet('#document-canvas').then(function(items) {
			expect(items).to.have.lengthOf(1);
			var XPos = items[0].getBoundingClientRect().left + 60;
			var YPos = items[0].getBoundingClientRect().top + 30;
			cy.cGet('body').rightclick(XPos, YPos);
		});
		cy.cGet('#mobile-wizard-content').should('be.visible');
		cy.cGet('body').contains('.context-menu-link', 'Insert Note').click();
		cy.cGet('.cool-annotation-table').should('exist');
		cy.cGet('#input-modal-input').should('have.text', '');

		// The device back button goes one level up in the wizard, which closes the editor.
		cy.getFrameWindow().then(function(win) {
			win.app.map.fire('mobilewizardback');
		});
		cy.cGet('#input-modal-input').should('not.exist');
		cy.getFrameWindow().then(function(win) {
			helper.processToIdle(win);
		});

		// No note is left behind.
		mobileHelper.openCommentWizard();
		cy.cGet('#mobile-wizard-content').should('exist');
		cy.cGet('[id^=comment-container-]').should('not.exist');
		cy.cGet('.wizard-comment-box .cool-annotation-content').should('not.exist');
	});

	it('Comment stays listed in the wizard after scrolling to it.', function() {
		// Put the note on a cell far below the visible rows.
		helper.typeIntoInputField(helper.addressInputSelector, 'A100');
		mobileHelper.insertComment(false, 'Note');

		// Close the wizard that opened on the new note and move the view back to the top.
		cy.cGet('#toolbar-up #comment_wizard button').click();
		cy.cGet('#toolbar-up #comment_wizard').should('not.have.class', 'selected');
		helper.typeIntoInputField(helper.addressInputSelector, 'A1');
		cy.getFrameWindow().then(function(win) {
			return helper.processToIdle(win);
		});
		cy.getFrameWindow().should(function(win) {
			expect(win.app.activeDocument.activeLayout.viewedRectangle.pY1).to.equal(0);
		});

		mobileHelper.openCommentWizard();
		cy.cGet('#comment-container-1').should('be.visible');
		cy.cGet('#comment-container-1').click();

		cy.getFrameWindow().then(function(win) {
			return helper.processToIdle(win);
		});
		cy.getFrameWindow().should(function(win) {
			expect(win.app.activeDocument.activeLayout.viewedRectangle.pY1).to.be.greaterThan(0);
		});

		// The entry is still on the list.
		cy.cGet('#comment-container-1').should('be.visible');
		cy.cGet('#annotation-content-area-1').should('have.text', 'some text');
	});
});
