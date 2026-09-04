/* global describe it cy before require expect */

const helper = require('../../common/helper');
const desktopHelper = require('../../common/desktop_helper');

describe(['tagdesktop'], 'Tab from a notebookbar tab accelerator', { testIsolation: false }, function () {
	let win;

	before(function () {
		cy.viewport(1920, 1024);
		helper.setupAndLoadDocument('writer/help_dialog.odt');
		desktopHelper.switchUIToNotebookbar();

		cy.getFrameWindow().then(function (frameWindow) {
			win = frameWindow;
		});
		cy.cGet('.notebookbar-tabs-container').should('be.visible');

		cy.wrap(null, { timeout: 20000 }).should(function () {
			if (!win.app.UI.notebookbarAccessibility.initialized)
				throw new Error('accessibility not initialized yet');
		});
	});

	function selectedTabPage() {
		return win.document.querySelector('div.ui-content.level-0.notebookbar:not(.hidden)');
	}

	it('lands inside the page of the tab the accelerator selected', function () {
		let tabs;

		cy.then(function () {
			const known = win.app.UI.notebookbarAccessibility.tabInfoList;

			tabs = Array.from(win.document.querySelectorAll('button.ui-tab.notebookbar'))
				.filter(function (el) {
					return !el.classList.contains('hidden') && el.offsetParent !== null;
				})
				.map(function (el) {
					const id = Object.keys(known).find(function (key) {
						return el.id.startsWith(key);
					});
					return { el: el, combination: id ? known[id].combination : null };
				})
				.filter(function (tab) { return tab.combination; });

			expect(tabs, 'visible tabs carrying an accelerator').to.not.be.empty;
		});

		cy.then(function () {
			tabs.forEach(function (tab) {
				cy.then(function () {
					// Clicking the tab that is already selected collapses the
					// notebookbar, so its own accelerator is not a case to drive here.
					if (tab.el.classList.contains('selected')) return;

					cy.realPress('Alt');
					tab.combination.split('').forEach(function (letter) {
						cy.realPress(letter);
					});

					cy.then(function () {
						expect(win.document.activeElement.id,
							tab.el.id + ': the accelerator state holds the focus')
							.to.equal('accessibilityInputElement');
					});

					cy.realPress('Tab');

					cy.then(function () {
						const page = selectedTabPage();
						expect(page, tab.el.id + ': a page is shown').to.not.be.null;
						expect(page.contains(win.document.activeElement),
							tab.el.id + ': Tab must move into ' + page.id + ', not past it')
							.to.be.true;
					});
				});
			});
		});
	});
});
