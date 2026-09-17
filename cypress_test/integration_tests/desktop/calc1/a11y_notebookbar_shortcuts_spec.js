/* global describe it cy before expect require */

const helper = require('../../common/helper');
const desktopHelper = require('../../common/desktop_helper');
const a11yHelper = require('../../common/a11y_helper');

describe(['tagdesktop'], 'Accessibility Calc notebookbar shortcut tests', { testIsolation: false }, function () {
	let tabs;
	let targets;

	before(function () {
		helper.setupAndLoadDocument('calc/help_dialog.ods');
		desktopHelper.switchUIToNotebookbar();

		cy.cGet('.notebookbar-tabs-container').should('be.visible');

		cy.getFrameWindow().then(function (win) {
			return helper.processToIdle(win).then(function () {
				tabs = a11yHelper.notebookbarShortcuts(win);
				targets = a11yHelper.notebookbarShortcutTargets(win);
				expect(tabs, 'notebookbar tabs carrying shortcuts').to.not.be.empty;
			});
		});
	});

	it('No two shortcuts of a tab can be typed for one another', function () {
		a11yHelper.assertShortcutsAreDistinct(tabs);
	});

	it('Every control of a tab has a shortcut', function () {
		a11yHelper.assertEveryControlHasAShortcut(tabs);
	});

	it('Every shortcut of a tab finds its own widget', function () {
		a11yHelper.assertShortcutsFindTheirWidgets(targets);
	});
});
