/* global describe it cy before expect require */

const helper = require('../../common/helper');
const desktopHelper = require('../../common/desktop_helper');

describe(['tagdesktop'], 'Accessibility notebookbar shortcut tests', { testIsolation: false }, function () {
	let win;
	let tabs;

	before(function () {
		helper.setupAndLoadDocument('writer/help_dialog.odt');
		desktopHelper.switchUIToNotebookbar();

		cy.getFrameWindow().then(function (frameWindow) {
			win = frameWindow;
		});
		cy.cGet('.notebookbar-tabs-container').should('be.visible');

		cy.then(function () {
			return helper.processToIdle(win);
		});

		cy.then(function () {
			tabs = readTabs();
			expect(tabs, 'notebookbar tabs carrying shortcuts').to.not.be.empty;
		});
	});

	function named(item) {
		return item.command || item.id;
	}

	function collect(node, into) {
		if (Array.isArray(node)) {
			node.forEach(function (child) { collect(child, into); });
			return;
		}
		if (!node || typeof node !== 'object') return;

		const foldsAway = node.type === 'overflowgroup';

		if (node.accessibility && node.accessibility.combination && !foldsAway)
			into.push({ name: named(node), combination: node.accessibility.combination });

		if (node.more && node.more.accessibility && node.more.accessibility.combination) {
			into.push({
				name: named(node) + ' (more)',
				combination: node.more.accessibility.combination,
			});
		}

		if (node.children) collect(node.children, into);
	}

	function readTabs() {
		const notebookbar = win.app.map.uiManager.notebookbar;
		const json = notebookbar.getFullJSON();

		let container = json;
		while (container && container.id !== 'ContextContainer' && container.children)
			container = container.children[0];
		expect(container && container.id, 'the tab page container').to.equal('ContextContainer');

		const found = [];
		container.children.forEach(function (page) {
			const content = page.children && page.children[0];
			if (!content) return;
			const items = [];
			collect(content.children, items);
			if (items.length) found.push({ where: content.id, items: items });
		});

		const strip = notebookbar.getTabs().filter(function (tab) {
			return tab.accessibility && tab.accessibility.combination;
		}).map(function (tab) {
			return { name: tab.id, combination: tab.accessibility.combination };
		});
		found.push({ where: 'the tab strip', items: strip });

		return found;
	}

	it('No two shortcuts of a tab name different commands', function () {
		tabs.forEach(function (tab) {
			const byCombination = {};
			tab.items.forEach(function (item) {
				const names = byCombination[item.combination] || [];
				if (names.indexOf(item.name) === -1) names.push(item.name);
				byCombination[item.combination] = names;
			});

			Object.keys(byCombination).forEach(function (combination) {
				expect(byCombination[combination],
					combination + ' in ' + tab.where + ' names one command')
					.to.have.lengthOf(1);
			});
		});
	});

	it('No shortcut of a tab is the start of another', function () {
		tabs.forEach(function (tab) {
			const combinations = tab.items.map(function (item) {
				return item.combination;
			}).filter(function (combination, at, all) {
				return all.indexOf(combination) === at;
			});

			combinations.forEach(function (shorter) {
				combinations.forEach(function (longer) {
					if (shorter === longer) return;
					expect(longer.indexOf(shorter) === 0,
						shorter + ' in ' + tab.where + ' would swallow ' + longer)
						.to.equal(false);
				});
			});
		});
	});
});
