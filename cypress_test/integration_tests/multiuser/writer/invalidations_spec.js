/* global describe it cy beforeEach expect require */

var helper = require('../../common/helper');
var desktopHelper = require('../../common/desktop_helper');
var ceHelper = require('../../common/contenteditable_helper');
var writerHelper = require('../../common/writer_helper');

function waitForInit(hasClass) {
	// TODO: skipDocumentCheck=FALSE in beforeEach, let's do it here once for now
	//       somehow it doesn't work for second iframe
	if (hasClass) {
		cy.cGet('#map')
			.should('have.class', 'initialized');
	}

	cy.cGet('.notebookbar-scroll-wrapper')
			.should('have.class', 'initialized');
	cy.cGet('#stylesview .ui-iconview-entry img').should('be.visible');

	cy.cGet('div.clipboard').as('clipboard');
}

describe(['tagmultiuser'], 'Joining a document should not trigger an invalidation', function() {

	beforeEach(function() {
		// Turn off SpellChecking by default because grammar checking,
		// when available, currently adds an extra empty update when
		// grammar checking kicks in at server-side idle after a change.
		localStorage.setItem('spellOnline', false);
		helper.setupAndLoadDocument('writer/invalidations.odt',
																/* skipDocumentCheck */ true,
																/* isMulti */ true);
		desktopHelper.switchUIToNotebookbar();
	});

	it.skip('Join document', function() {
		cy.cSetActiveFrame('#iframe1');
		waitForInit(true);
		cy.cGet('#toolbar-down #StateWordCount').should('have.text', '0 words, 0 characters');

		ceHelper.type('X');
		cy.wait(1000);

		cy.cSetActiveFrame('#iframe2');
		waitForInit(false);
		cy.cGet('#toolbar-down #StateWordCount').should('have.text', '1 word, 1 character');

		cy.cSetActiveFrame('#iframe1');
		writerHelper.selectAllTextOfDoc();
		cy.cGet('#toolbar-down #StateWordCount').should('have.text', '1 word, 1 character');

		cy.cGet('.empty-deltas').then(($before) => {
			const beforeCount = $before.text();

			// joining triggered a theme related invalidation
			cy.cSetActiveFrame('#iframe2');
			cy.get('#form2').submit();
			cy.wait(1000);

			cy.cSetActiveFrame('#iframe1');
			writerHelper.selectAllTextOfDoc();
			cy.cGet('#toolbar-down #StateWordCount').should('have.text', '1 word, 1 character');

			cy.cGet('.empty-deltas').should(($after) => {
				expect($after.text()).to.eq(beforeCount);
			});
		});
	});

	it.skip('Join after document save and modify', function() {
		cy.cSetActiveFrame('#iframe1');
		waitForInit(true);
		cy.cGet('#toolbar-down #StateWordCount').should('have.text', '0 words, 0 characters');

		ceHelper.type('X');
		cy.wait(1000);

		cy.cSetActiveFrame('#iframe2');
		waitForInit(false);
		cy.cGet('#toolbar-down #StateWordCount').should('have.text', '1 word, 1 character');

		cy.cSetActiveFrame('#iframe1');
		writerHelper.selectAllTextOfDoc();
		cy.cGet('#document-container').click({force:true});
		cy.cGet('#toolbar-down #StateWordCount').should('have.text', '1 word, 1 character');

		cy.cGet('.empty-deltas').then(($before) => {
			var beforeCount = parseInt($before.text());

			// joins after a save triggered excessive invalidations on changes
			cy.cGet('#File-tab-label').click();
			cy.cGet('.notebookbar-shortcuts-bar .unoSave > button').click();

			// Reload page
			cy.cSetActiveFrame('#iframe2');
			cy.get('#form2').submit();
			// Wait for page to unload
			cy.wait(1000);
			// Wait for page to finish loading
			helper.documentChecks(true);

			cy.cSetActiveFrame('#iframe1');
			writerHelper.selectAllTextOfDoc();
			cy.cGet('#document-container').click({force:true});
			cy.cGet('#toolbar-down #StateWordCount').should('have.text', '1 word, 1 character');

			ceHelper.type('X');
			cy.wait(1000);

			cy.cGet('#toolbar-down #StateWordCount').should('have.text', '1 word, 2 characters');

			cy.cGet('.empty-deltas').should(($after) => {
				// allow one row of empty deltas, the case this protects regression against
				// is a whole document invalidation
				expect(parseInt($after.text())).to.be.lessThan(beforeCount + 12);
			});
		});
	});

});
