/* global describe expect it cy before require Cypress Array JSON */

const helper = require('../../common/helper');
const desktopHelper = require('../../common/desktop_helper');
const a11yHelper = require('../../common/a11y_helper');

// What every item of the notebookbar owes a screen reader, over every tab.
describe(['tagdesktop'], 'Notebookbar items', { testIsolation: false }, function () {
	let win;
	const swept = [];
	const dark = [];

	before(function () {
		cy.viewport(1920, 1080);
		const brand = Cypress.env('brandTheme');
		helper.setupAndLoadDocument('calc/help_dialog.ods', false, false, undefined,
			brand ? 'theme=' + brand : undefined);

		cy.getFrameWindow().then(function (frameWindow) {
			win = frameWindow;
		});

		desktopHelper.switchUIToNotebookbar();
		cy.cGet('.notebookbar-tabs-container').should('be.visible');
		cy.then(function () { return helper.processToIdle(win); });
	});

	function tabs() {
		return win.app.map.uiManager.notebookbar.getTabs().filter(function (tab) {
			const label = win.document.getElementById(tab.id);

			return label && !label.classList.contains('hidden');
		});
	}

	// Clicking while the strip is hidden collapses the notebookbar.
	function openTab(tab) {
		cy.cGet('.ui-tabs.notebookbar').should('be.visible');
		cy.cGet('#' + tab.id).should('be.visible').click();
		cy.cGet('#' + tab.id).should('have.class', 'selected');
		cy.cGet('#' + tab.name + '-container').should('be.visible');
		cy.then(function () { return helper.waitUntilLayoutingIsIdle(win); });
		cy.then(function () { return helper.waitForTimers(win, 'jsdialog-deferred'); });
	}

	function describe(item) {
		return item.tagName.toLowerCase() + '.'
			+ item.className.split(' ').slice(0, 2).join('.')
			+ '[cooltip=' + JSON.stringify(item.getAttribute('data-cooltip')) + ']';
	}

	function isGrouped(item, container) {
		for (let at = item.parentElement; at && at !== container; at = at.parentElement)
			if (at.hasAttribute('data-cooltip') || at.classList.contains('unotoolbutton'))
				return true;

		return false;
	}

	function readOpenTab(tab, into) {
		cy.then(function () {
			const container = win.document.getElementById(tab.name + '-container');
			// A preview carries a tooltip and no button, and is named by its owner.
			const items = Array.from(
				container.querySelectorAll('.unotoolbutton, [data-cooltip]'))
				.filter(function (item) {
					return item.classList.contains('unotoolbutton')
						|| item.tagName === 'BUTTON'
						|| item.querySelector('button') !== null;
				});

			expect(items.length, 'items read off ' + tab.name).to.be.greaterThan(0);

			items.forEach(function (item) {
				const button = item.querySelector('button') || item;
				const label = item.querySelector('.unolabel');

				into.push({
					where: tab.name + ' ' + (item.id || describe(item)),
					name: button.getAttribute('aria-label') || item.getAttribute('aria-label'),
					description: button.getAttribute('aria-description'),
					pressed: button.getAttribute('aria-pressed'),
					ariaDisabled: button.getAttribute('aria-disabled'),
					tooltip: item.getAttribute('data-cooltip'),
					selected: item.classList.contains('selected'),
					disabled: item.hasAttribute('disabled'),
					grouped: isGrouped(item, container),
					ratio: label
						? a11yHelper.contrastRatio(
							a11yHelper.renderedTextColor(win, label),
							a11yHelper.effectiveBackground(win, label))
						: null,
				});
			});
		});
	}

	// Joined: chai prints a long array as Array(n).
	function offenders(sweep, broken) {
		return sweep.filter(broken).map(function (item) {
			return item.where;
		}).join(', ');
	}

	function unreadable(sweep) {
		return offenders(sweep, function (item) {
			return item.ratio !== null && item.ratio < 4.5;
		});
	}

	it('walks every tab of the notebookbar', function () {
		cy.then(function () {
			tabs().forEach(function (tab) {
				openTab(tab);
				readOpenTab(tab, swept);
			});
		});

		cy.then(function () {
			const seen = swept.map(function (item) {
				return item.where.split(' ')[0];
			});

			tabs().forEach(function (tab) {
				expect(seen, 'items read off ' + tab.name).to.include(tab.name);
			});
		});
	});

	it('every item that stands on its own carries a name', function () {
		const standalone = swept.filter(function (item) { return !item.grouped; });

		expect(standalone, 'items that stand on their own').to.not.be.empty;
		expect(offenders(standalone, function (item) { return !item.name; }),
			'items with no accessible name').to.be.empty;
	});

	it('describes itself with the tooltip it carries', function () {
		expect(offenders(swept, function (item) {
			return item.description !== null && item.description !== item.tooltip;
		}), 'items whose description is not the tooltip they carry').to.be.empty;
	});

	it('says whether it is on', function () {
		const on = swept.filter(function (item) { return item.selected; });

		expect(on, 'selected items in the sweep').to.not.be.empty;
		expect(offenders(on, function (item) { return item.pressed !== 'true'; }),
			'items that look pressed and do not say so').to.be.empty;
	});

	// No item of a Calc notebookbar declares a purpose yet, so what is asserted
	// here is the mechanism that would carry one.
	it('writes a description only when there is one to write', function () {
		cy.then(function () {
			const item = win.document.querySelector('#Home-container .unotoolbutton');
			const button = item.querySelector('button') || item;
			const purpose = 'Lets a screen reader follow the cursor';

			expect(win.JSDialog.AddAriaDescription, 'the helper').to.be.a('function');

			win.JSDialog.AddAriaDescription(button, { aria: { description: purpose } });
			expect(button.getAttribute('aria-description'), item.id + ' describes itself')
				.to.equal(purpose);

			button.removeAttribute('aria-description');
			win.JSDialog.AddAriaDescription(button, { aria: { description: '   ' } });
			expect(button.getAttribute('aria-description'), 'a blank description')
				.to.be.null;

			win.JSDialog.AddAriaDescription(button, {});
			expect(button.getAttribute('aria-description'), 'no description at all')
				.to.be.null;
		});
	});

	it('says when it is greyed out', function () {
		const greyed = swept.filter(function (item) { return item.disabled; });

		expect(greyed, 'greyed-out items in the sweep').to.not.be.empty;
		expect(offenders(greyed, function (item) {
			return item.ariaDisabled !== 'true';
		}), 'greyed-out items that do not say so').to.be.empty;
	});

	it('says it is greyed out when the state arrives', function () {
		let item;

		cy.then(function () {
			openTab(tabs().filter(function (tab) { return tab.name === 'Home'; })[0]);
		});

		cy.then(function () {
			item = win.document.querySelector(
				'#Home-container .unotoolbutton:not([disabled])');

			expect(item, 'an item to grey out').to.not.be.null;
			// As the state arrives: an attribute change the widget mirrors.
			item.setAttribute('disabled', '');
		});

		cy.then(function () { return helper.waitUntilLayoutingIsIdle(win); });

		cy.then(function () {
			const button = item.querySelector('button') || item;

			expect(button.getAttribute('aria-disabled'), item.id + ' greyed out')
				.to.equal('true');
			item.removeAttribute('disabled');
		});
	});

	it('keeps every label readable', function () {
		expect(unreadable(swept), 'labels under 4.5:1').to.be.empty;
	});

	// What core sends for a gallery carries no name of its own.
	it('names the gallery a tab groups', function () {
		const withGallery = [];

		cy.then(function () {
			tabs().forEach(function (tab) {
				openTab(tab);
				cy.then(function () {
					const container = win.document.getElementById(tab.name + '-container');

					container.querySelectorAll('.ui-iconview').forEach(function (gallery) {
						withGallery.push({ tab: tab.name, id: gallery.id });
					});
				});
			});
		});

		cy.then(function () {
			expect(withGallery, 'galleries in the notebookbar').to.not.be.empty;

			withGallery.forEach(function (gallery) {
				openTab(tabs().filter(function (tab) {
					return tab.name === gallery.tab;
				})[0]);

				a11yHelper.getAXNodesWithin('#' + gallery.id).then(function (nodes) {
					const group = nodes.filter(function (node) {
						return node.role === 'radiogroup' || node.role === 'listbox';
					})[0];

					expect(group, 'an AX node for ' + gallery.id).to.not.be.undefined;
					expect(group.name, gallery.tab + ' ' + gallery.id + ' announces itself')
						.to.not.be.empty;
				});
			});
		});
	});

	it('and in dark mode too', function () {
		cy.then(function () { win.app.map.uiManager.toggleDarkMode(); });
		cy.cframe().find('html').should('have.attr', 'data-theme', 'dark');

		cy.then(function () {
			tabs().forEach(function (tab) {
				openTab(tab);
				readOpenTab(tab, dark);
			});
		});

		cy.then(function () {
			expect(dark, 'items read in dark mode').to.not.be.empty;
			expect(unreadable(dark), 'labels under 4.5:1 in dark mode').to.be.empty;
			win.app.map.uiManager.toggleDarkMode();
		});
	});
});
