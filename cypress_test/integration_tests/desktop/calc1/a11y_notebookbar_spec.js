/* global describe expect it cy before after afterEach require */

var helper = require('../../common/helper');
var desktopHelper = require('../../common/desktop_helper');
var a11yHelper = require('../../common/a11y_helper');

describe(['tagdesktop'], 'Accessibility Calc Notebookbar Tests', { testIsolation: false }, function () {
	var tabs;
	var allTabNames;
	var visitedTabNames;
	var win;

	before(function () {
		helper.setupAndLoadDocument('calc/help_dialog.ods');

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

	// Context tabs that need complex setup not yet implemented
	var skipContextTabs = ['Chart', 'Sparkline', 'Table'];

	after(function () {
		var unvisited = allTabNames.filter(function (name) {
			return !visitedTabNames.includes(name) && !skipContextTabs.includes(name);
		});
		expect(unvisited, 'unvisited notebookbar tabs').to.be.empty;
	});

	afterEach(function () {
		a11yHelper.resetState();
	});

	function selectAndValidateTab(tab) {
		var selector = '#' + tab.id;
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

		cy.then(function () {
			selectAndValidateTab(findTab('Shape'));
		});

		// exit shape mode
		helper.typeIntoDocument('{esc}');
	});

	it('Notebookbar tab: Picture (context)', function () {
		cy.viewport(1920, 1080);

		desktopHelper.insertImage();

		cy.then(function () {
			selectAndValidateTab(findTab('Picture'));
		});

		// exit picture mode
		helper.typeIntoDocument('{esc}');
	});

	// TODO: Add a Sparkline to test document after merge of outstanding pr
	// TODO: Table context is for "Table in Table" feature, not available in .ods format

	it('All non-context notebookbar tabs', function () {
		cy.then(function () {
			var nonContextTabs = tabs.filter(function (tab) {
				return !tab.context || tab.context.includes('default');
			});

			var chain = cy.wrap(null);
			nonContextTabs.forEach(function (tab) {
				chain = chain.then(function () {
					selectAndValidateTab(tab);
				});
			});
		});
	});
});
