/* global describe it cy beforeEach require expect */

var helper = require('../../common/helper');

describe(['tagmultiuser'], 'Multiuser Writer Save State', function() {

	beforeEach(function() {
		helper.setupAndLoadDocument('writer/invalidations.odt', true);

		cy.cSetActiveFrame('#iframe1');
		cy.getFrameWindow().then((win) => { this.win1 = win; });
		cy.cSetActiveFrame('#iframe2');
		cy.getFrameWindow().then((win) => { this.win2 = win; });
	});

	function ensureNotebookbar(win) {
		cy.then(function() {
			if (win.app.map.uiManager.getCurrentMode() !== 'notebookbar')
				win.app.map.uiManager.onChangeUIMode({ mode: 'notebookbar' });
		});
		cy.cGet('[id^="save"].unotoolbutton').should('be.visible');
	}

	it('Modified state survives shortcuts bar rebuild on user join', function() {
		// User 1 works in the notebookbar; the document starts unmodified.
		cy.cSetActiveFrame('#iframe1');
		ensureNotebookbar(this.win1);
		cy.cGet('[id^="save"].unotoolbutton').should('not.have.class', 'savemodified');

		// User 1 modifies the document.
		helper.clickIntoDocument();
		helper.processToIdle(this.win1);
		helper.typeIntoDocument('collaborative edit');

		// The save icon shows the unsaved change
		cy.cGet('[id^="save"].unotoolbutton').should('have.class', 'savemodified');
		cy.then(() => {
			expect(this.win1.app.file.modified, 'user 1 app.file.modified').to.be.true;
		});

		// User 2's session receives the broadcast modified state
		cy.cSetActiveFrame('#iframe2');
		ensureNotebookbar(this.win2);
		cy.cGet('[id^="save"].unotoolbutton').should('have.class', 'savemodified');
		cy.then(() => {
			expect(this.win2.app.file.modified, 'user 2 app.file.modified').to.be.true;
		});

		// Back on user 1: reproduce the shortcuts bar rebuild like a user join
		cy.cSetActiveFrame('#iframe1');
		cy.then(() => {
			this.win1.app.map.uiManager.notebookbar.reloadShortcutsBar();
			helper.processToIdle(this.win1);
		});

		// The rebuilt save icon must still carry the modified state
		cy.cGet('[id^="save"].unotoolbutton').should('have.class', 'savemodified');
	});
});
