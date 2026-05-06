/* global describe expect it cy before after afterEach require */

var helper = require('../../common/helper');
var desktopHelper = require('../../common/desktop_helper');
var a11yHelper = require('../../common/a11y_helper');

describe(['tagdesktop'], 'Accessibility Writer Notebookbar Tests', { testIsolation: false }, function () {
	var tabs;
	var allTabNames;
	var visitedTabNames;
	var win;

	before(function () {
		helper.setupAndLoadDocument('writer/help_dialog.odt');

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
	var skipContextTabs = ['Chart'];

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

	it('Notebookbar tab: Table (context)', function () {
		// Workaround: we start in a table, so table context
		// should be active, but at the moment is it not, workaround
		// that bug by leaving the table and reentering it.
		helper.typeIntoDocument('{ctrl}{end}');
		helper.typeIntoDocument('{ctrl}{end}');
		helper.typeIntoDocument('{ctrl}{home}');

		selectAndValidateTab(findTab('Table'));
	});

	it('Notebookbar tab: Shape (context)', function () {

		cy.then(function () {
			win.app.map.sendUnoCommand('.uno:BasicShapes.octagon');
		});

		cy.cGet('#test-div-shapeHandlesSection').should('exist');

		selectAndValidateTab(findTab('Shape'));

		// exit shape mode
		helper.typeIntoDocument('{esc}');
	});

	it('Notebookbar tab: Picture (context)', function () {

		desktopHelper.insertImage();

		selectAndValidateTab(findTab('Picture'));

		// exit picture mode
		helper.typeIntoDocument('{esc}');
	});

	it('Notebookbar tab: Formula (context)', function () {

		cy.then(function () {
			win.app.map.sendUnoCommand('.uno:InsertObjectStarMath');
		});

		cy.cGet('#test-div-shapeHandlesSection').should('exist');

		selectAndValidateTab(findTab('Formula'));

		// exit formula mode
		helper.typeIntoDocument('{esc}');
	});

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
