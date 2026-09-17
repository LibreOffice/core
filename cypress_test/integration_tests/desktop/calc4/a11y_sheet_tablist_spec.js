/* global describe expect it cy before require Cypress */

const helper = require('../../common/helper');
const a11yHelper = require('../../common/a11y_helper');

describe(['tagdesktop'], 'Sheet tablist', { testIsolation: false }, function () {
	let win;

	before(function () {
		cy.viewport(1920, 1080);
		// switch.ods carries two sheets, so one tab is selected and one is not.
		const brand = Cypress.env('brandTheme');
		helper.setupAndLoadDocument('calc/switch.ods', false, false, undefined,
			brand ? 'theme=' + brand : undefined);

		cy.getFrameWindow().then(function (frameWindow) {
			win = frameWindow;
		});
		cy.cGet('#spreadsheet-tab1').should('be.visible');
	});

	function tablistNodes() {
		return a11yHelper.getAXNodesWithin('#spreadsheet-tab-scroll');
	}

	function panelNode() {
		return a11yHelper.getAXNodesWithin('#document-container')
			.then(function (nodes) {
				return nodes.filter(function (node) {
					return node.role === 'tabpanel';
				})[0];
			});
	}

	function tabsOf(nodes) {
		return nodes.filter(function (node) {
			return node.role === 'tab';
		});
	}

	function selectedTabName(nodes) {
		const selected = tabsOf(nodes).filter(function (tab) {
			return tab.properties.selected === true;
		});

		expect(selected.length, 'tabs reported as selected').to.equal(1);
		return selected[0].name;
	}

	function selectAnotherSheet() {
		cy.then(function () {
			const other = Array.from(
				win.document.querySelectorAll('button[id^="spreadsheet-tab"]'))
				.filter(function (tab) {
					return !tab.classList.contains('spreadsheet-tab-selected');
				})[0];

			expect(other, 'a sheet tab that is not the selected one').to.not.be.undefined;
			other.click();
		});
		cy.then(function () { return helper.processToIdle(win); });
	}

	it('the sheet bar reaches the tree as a named tablist', function () {
		cy.then(function () { return tablistNodes(); }).then(function (nodes) {
			const tablist = nodes.filter(function (node) {
				return node.role === 'tablist';
			})[0];

			expect(tablist, 'a tablist node for the sheet bar').to.not.be.undefined;
			expect(tablist.name, 'the name the tablist carries').to.not.be.empty;
			expect(tablist.ignored, 'the tablist reaches the tree').to.not.be.true;
		});
	});

	it('every sheet is a tab of its own, and exactly one is selected', function () {
		let expected;

		cy.then(function () { expected = win.app.calc.getVisiblePartCount(); });

		cy.then(function () { return tablistNodes(); }).then(function (nodes) {
			const tabs = tabsOf(nodes);

			expect(tabs.length, 'tabs against the sheets the document shows')
				.to.equal(expected);

			tabs.forEach(function (tab) {
				expect(tab.name, 'the name of a tab').to.not.be.empty;
				expect(tab.properties.controls, tab.name + ' says what it controls')
					.to.not.be.undefined;
			});

			selectedTabName(nodes);
		});
	});

	it('the panel is named after the sheet whose tab is selected', function () {
		let name;

		cy.then(function () { return tablistNodes(); }).then(function (nodes) {
			name = selectedTabName(nodes);
		});

		cy.then(function () { return panelNode(); }).then(function (panel) {
			expect(panel, 'a tabpanel node for the document container').to.not.be.undefined;
			expect(panel.name, 'the name the panel carries').to.equal(name);
		});
	});

	it('and the panel follows when another sheet is selected', function () {
		let before;

		cy.then(function () { return tablistNodes(); }).then(function (nodes) {
			before = selectedTabName(nodes);
		});

		selectAnotherSheet();

		cy.then(function () { return tablistNodes(); }).then(function (nodes) {
			const after = selectedTabName(nodes);

			expect(after, 'the sheet selected after the switch').to.not.equal(before);

			cy.then(function () { return panelNode(); }).then(function (panel) {
				expect(panel.name, 'the name the panel carries after the switch')
					.to.equal(after);
			});
		});
	});
});
