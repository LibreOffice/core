/* global describe it cy before require */

var helper = require('../../common/helper');

// The WOPI data next to this spec's document describes a host that allows
// neither Save As (UserCanNotWriteRelative) nor rename (SupportsRename), so
// there is no way for the user to give the document another name.
describe(['tagdesktop'], 'Document name editability', { testIsolation: false }, function() {

	before(function() {
		helper.setupAndLoadDocument('writer/document_name_permission.fodt',
			/* isMultiUser */ false, /* copy the .wopi.json next to it */ true);
	});

	function documentNameInput() {
		return cy.cGet('#document-name-input');
	}

	// An automatic save runs in the background save process, which reports its
	// start and its end with these two frames.
	function backgroundSave() {
		cy.getFrameWindow().then(function(win) {
			win.app.socket._onMessage({ textMsg: 'progress: { "id":"start", "type":"bg" }' });
		});

		cy.cGet('#document-name-input-progress-bar').should('be.visible');

		cy.getFrameWindow().then(function(win) {
			win.app.socket._onMessage({ textMsg: 'progress: { "id":"finish", "type":"bg" }' });
		});

		cy.cGet('#document-name-input-progress-bar').should('not.be.visible');
	}

	it('the name is read-only when the host allows neither Save As nor rename', function() {
		documentNameInput().should('be.disabled').should('not.have.class', 'editable');
	});

	it('the name is still read-only once a background save has finished', function() {
		helper.typeIntoDocument('a few words');

		backgroundSave();

		documentNameInput().should('be.disabled').should('not.have.class', 'editable');
	});

	it('the name is still read-only once the loading animation has ended', function() {
		cy.getFrameWindow().then(function(win) {
			win.app.map.uiManager.documentNameInput.showLoadingAnimation();
		});

		cy.cGet('#document-name-input-loading-bar').should('be.visible');

		cy.getFrameWindow().then(function(win) {
			win.app.map.uiManager.documentNameInput.hideLoadingAnimation();
		});

		cy.cGet('#document-name-input-loading-bar').should('not.be.visible');

		documentNameInput().should('be.disabled').should('not.have.class', 'editable');
	});

	it('a host that allows Save As gets an editable name back after a background save', function() {
		cy.getFrameWindow().then(function(win) {
			win.app.map['wopi'].UserCanNotWriteRelative = false;
			win.app.map.uiManager.documentNameInput.restoreDocumentNameInput();
		});

		documentNameInput().should('not.be.disabled').should('have.class', 'editable');

		backgroundSave();

		documentNameInput().should('not.be.disabled').should('have.class', 'editable');

		cy.getFrameWindow().then(function(win) {
			win.app.map['wopi'].UserCanNotWriteRelative = true;
			win.app.map.uiManager.documentNameInput.restoreDocumentNameInput();
		});
	});
});
