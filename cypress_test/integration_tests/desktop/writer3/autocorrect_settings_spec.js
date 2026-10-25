/* global describe it cy beforeEach require Cypress */

const helper = require('../../common/helper');
const desktopHelper = require('../../common/desktop_helper');
const ceHelper = require('../../common/contenteditable_helper');

describe(['tagdesktop'], 'AutoCorrect settings.', function() {

	let documentFilePath;

	// The dev WOPI server keeps a per-user preset store only for a test that
	// asks for one with an explicit &userid (see skipPersist in
	// test/TestWopiFileServer.hpp).
	const userId = 'autocorrect-settings';
	const userQuery = 'userid=' + userId;

	// Where the dev WOPI server files a userconfig upload away. Mirrors
	// userPresetDir() in test/TestWopiFileServer.hpp.
	const userStore = Cypress.env('PRESETS_ROOT') + '/user/u-' + userId;

	// A word no AutoCorrect list knows, and what it is to become.
	const shortcut = 'zzqx';
	const replacement = 'quartz';

	// The row of "Capitalize first letter of every sentence" on the Options page.
	const capitalizeSentenceRow = 2;
	// The [T] column, While Typing; [M] is column 0.
	const whileTypingColumn = 1;

	function openAutoCorrectDialog() {
		// Wide enough for the AutoCorrect group not to fold into a menu.
		cy.viewport(1920, 1080);
		cy.cGet('#Review-tab-label').click();
		desktopHelper.getNbIcon('AutoCorrectDlg', 'Review').click();
		cy.cGet('[id^="AutoCorrectDialog"]').should('be.visible');
	}

	function autoCorrectWindow() {
		return cy.cGet('[id^="AutoCorrectDialog"]').closest('.jsdialog-window');
	}

	function capitalizeSentenceWhileTyping() {
		return autoCorrectWindow().find('#list-checkbox-' + capitalizeSentenceRow + '-' + whileTypingColumn + ':visible');
	}

	beforeEach(function() {
		// Start from an empty store, so nothing an earlier run uploaded is
		// served back.
		cy.task('removeUserPresets', { userId: userId });

		documentFilePath = helper.setupAndLoadDocument('writer/presets.odt', false, false,
			undefined, userQuery);
		cy.cGet('div.clipboard').as('clipboard');
	});

	it('The AutoCorrect Options dialog changes outlive the session.', function() {
		openAutoCorrectDialog();

		// A replacement of our own on the Replace page.
		autoCorrectWindow().find('button#replace.ui-tab').click();
		autoCorrectWindow().find('#origtext input').type(shortcut);
		autoCorrectWindow().find('#newtext input').type(replacement);
		autoCorrectWindow().find('#new button').click();

		// The While Typing half of an option on the Options page, which has a
		// second column of checkboxes for applying AutoCorrect on demand.
		autoCorrectWindow().find('button#applypage.ui-tab').click();
		capitalizeSentenceWhileTyping().should('be.checked');
		capitalizeSentenceWhileTyping().uncheck();
		capitalizeSentenceWhileTyping().should('not.be.checked');

		autoCorrectWindow().find('#ok button').click();
		cy.cGet('[id^="AutoCorrectDialog"]').should('not.exist');

		// The settings reach the host while the document is still open:
		// readFile retries until the upload has arrived.
		cy.readFile(userStore + '/registrymodifications.xcu')
			.should('contain', '/org.openoffice.Office.Writer/AutoFunction');
		cy.readFile(userStore + '/acor_en-US.dat', 'binary').should('not.be.empty');

		helper.reloadDocument(documentFilePath, userQuery);
		cy.cGet('div.clipboard').as('clipboard');

		// The replacement is made and the sentence is not capitalized.
		ceHelper.type(shortcut + ' hello. world ');
		ceHelper.checkPlainContent(replacement + ' hello. world ');

		// And the dialog shows the option the way it was left.
		openAutoCorrectDialog();
		autoCorrectWindow().find('button#applypage.ui-tab').click();
		capitalizeSentenceWhileTyping().should('not.be.checked');
		autoCorrectWindow().find('#cancel button').click();
	});
});
