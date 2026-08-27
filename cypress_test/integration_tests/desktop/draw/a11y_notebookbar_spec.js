/* global describe expect it cy before after afterEach require */

const helper = require('../../common/helper');
const desktopHelper = require('../../common/desktop_helper');
const a11yHelper = require('../../common/a11y_helper');

describe(['tagdesktop'], 'Accessibility Draw Notebookbar Tests', { testIsolation: false }, function () {
	let tabs;
	let allTabNames;
	let visitedTabNames;
	let win;

	before(function () {
		helper.setupAndLoadDocument('draw/insert_position.fodg');

		desktopHelper.switchUIToNotebookbar();

		cy.getFrameWindow().then(function (frameWindow) {
			win = frameWindow;
		});

		cy.then(function () {
			tabs = win.app.map.uiManager.notebookbar.getTabs();
			allTabNames = tabs.map(function (tab) { return tab.name; });
			visitedTabNames = [];
		});
	});

	// Chart needs its edit mode, not just the object; impress skips it too.
	const skipContextTabs = ['Chart'];

	after(function () {
		const unvisited = allTabNames.filter(function (name) {
			return !visitedTabNames.includes(name) && !skipContextTabs.includes(name);
		});
		expect(unvisited, 'unvisited notebookbar tabs').to.be.empty;
	});

	afterEach(function () {
		a11yHelper.resetState();
	});

	function selectAndValidateTab(tab) {
		const selector = '#' + tab.id;
		cy.cGet(selector).should('be.visible').click();
		cy.cGet(selector).should('have.class', 'selected');

		helper.processToIdle(win);
		a11yHelper.runA11yValidation(win, 'validatenotebookbara11y');

		visitedTabNames.push(tab.name);
	}

	function findTab(name) {
		return tabs.find(function (t) { return t.name === name; });
	}

	it('Notebookbar tab: Shape (context)', function () {
		cy.then(function () {
			win.app.map.sendUnoCommand('.uno:BasicShapes.octagon');
		});

		cy.cGet('#test-div-shapeHandlesSection').should('exist');

		selectAndValidateTab(findTab('Shape'));

		helper.typeIntoDocument('{esc}');
	});

	it('Notebookbar tab: Picture (context)', function () {
		desktopHelper.insertImage();

		selectAndValidateTab(findTab('Picture'));

		helper.typeIntoDocument('{esc}');
	});

	it('Notebookbar tab: Table (context)', function () {
		cy.then(function () {
			win.app.map.sendUnoCommand('.uno:InsertTable');
		});

		cy.cGet('.ui-dialog[role="dialog"]').should('exist');
		cy.cGet('#ok-button').click();
		helper.processToIdle(win);

		selectAndValidateTab(findTab('Table'));
		// Table Design shares the Table context.
		selectAndValidateTab(findTab('TableDesign'));

		helper.typeIntoDocument('{esc}');
		helper.typeIntoDocument('{esc}');
	});

	it('All non-context notebookbar tabs', function () {
		cy.then(function () {
			const nonContextTabs = tabs.filter(function (tab) {
				return !tab.context || tab.context.includes('default');
			});

			let chain = cy.wrap(null);
			nonContextTabs.forEach(function (tab) {
				chain = chain.then(function () {
					selectAndValidateTab(tab);
				});
			});
		});
	});
});
