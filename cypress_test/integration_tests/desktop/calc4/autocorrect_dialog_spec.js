/* global describe it cy beforeEach require */

const helper = require('../../common/helper');
const desktopHelper = require('../../common/desktop_helper');

describe(['tagdesktop'], 'AutoCorrect Options in Calc.', function() {

	beforeEach(function() {
		helper.setupAndLoadDocument('calc/BottomBar.ods');
		// Wide enough for the Review tab not to fold the button into a menu.
		cy.viewport(1920, 1080);
	});

	it('The AutoCorrect Options dialog opens from the Review tab.', function() {
		cy.cGet('#Review-tab-label').click();
		desktopHelper.getNbIcon('AutoCorrectDlg', 'Review').click();

		// Calc has the Options page of its own, with one checkbox per row.
		cy.cGet('[id^="AutoCorrectDialog"]').should('be.visible');
		cy.cGet('[id^="AutoCorrectDialog"]').closest('.jsdialog-window')
			.find('button#options.ui-tab').should('be.visible');

		cy.cGet('[id^="AutoCorrectDialog"]').closest('.jsdialog-window')
			.find('#cancel button').click();
		cy.cGet('[id^="AutoCorrectDialog"]').should('not.exist');
	});
});
