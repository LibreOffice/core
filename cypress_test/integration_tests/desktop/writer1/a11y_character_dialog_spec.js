/* global describe it cy before after expect require */

const helper = require('../../common/helper');
const desktopHelper = require('../../common/desktop_helper');
const a11yHelper = require('../../common/a11y_helper');

describe(['tagdesktop'], 'Accessibility of the character dialog', { testIsolation: false }, function () {
	let win;
	let comboboxes;

	before(function () {
		helper.setupAndLoadDocument('writer/help_dialog.odt');
		desktopHelper.switchUIToNotebookbar();

		cy.getFrameWindow().then(function (frameWindow) {
			win = frameWindow;
		});
		cy.cGet('.notebookbar-tabs-container').should('be.visible');

		cy.then(function () {
			win.app.map.sendUnoCommand('.uno:FontDialog');
			return helper.processToIdle(win);
		});

		a11yHelper.getActiveDialog(1).then(function ($dialog) {
			comboboxes = Array.from($dialog[0].querySelectorAll('.ui-combobox[id]'))
				.filter(function (box) {
					return win.getComputedStyle(box).visibility !== 'hidden';
				})
				.map(function (box) { return box.id; });

			expect(comboboxes, 'comboboxes on the Font page').to.not.be.empty;
		});
	});

	after(function () {
		a11yHelper.closeActiveDialog(1);
	});

	it('The button that opens a font dropdown says which list it opens', function () {
		if (!a11yHelper.axTreeAvailable()) {
			this._runnable.title += ' (skipped: needs a chromium browser)';
			this.skip();
		}

		cy.then(function () {
			comboboxes.forEach(function (id) {
				a11yHelper.assertDropdownButtonNamesItsList(id);
			});
		});
	});
});
