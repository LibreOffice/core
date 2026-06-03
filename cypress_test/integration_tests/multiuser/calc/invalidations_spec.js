/* global describe it cy beforeEach expect require */

var helper = require('../../common/helper');
var desktopHelper = require('../../common/desktop_helper');

describe(['tagmultiuser'], 'Joining a document should not trigger an invalidation', function() {

	beforeEach(function() {
		// Turn off SpellChecking by default because grammar checking,
		// when available, currently adds an extra empty update when
		// grammar checking kicks in at server-side idle after a change.
		localStorage.setItem('spreadsheet.spellOnline', false);
		helper.setupAndLoadDocument('calc/invalidations.ods',true);
		desktopHelper.switchUIToNotebookbar();
	});

	it('Join document', function() {
		cy.cSetActiveFrame('#iframe1');

		cy.getFrameWindow().then((win) => {
			this.win1 = win;
		}).then(() => {
			helper.processToIdle(this.win1);
		}).then(() => {
			helper.typeIntoDocument('X');
			helper.typeIntoDocument('{enter}');
		}).then(() => {
			helper.processToIdle(this.win1);
		}).then(() => {
			helper.typeIntoDocument('{uparrow}');
		}).then(() => {
			// Wait until after tile updates have arrived.
			helper.processToIdle(this.win1);
		}).then(() => {
			cy.cGet('.empty-deltas');
		}).then(($before) => {
			const beforeCount = $before.text();

			// joining triggered some theme related invalidations

			// Reload page
			cy.cSetActiveFrame('#iframe2');
			cy.get('#form2').submit();
			// Wait for page to unload
			cy.wait(1000);
			// Wait for page to finish loading
			helper.documentChecks(true);

			cy.cSetActiveFrame('#iframe1');
			helper.typeIntoDocument('{rightarrow}');

			// Wait until after tile updates have arrived.
			helper.processToIdle(this.win1);

			cy.cGet('.empty-deltas').should(($after) => {
				expect($after.text()).to.eq(beforeCount);
			});
		});
	});
});
