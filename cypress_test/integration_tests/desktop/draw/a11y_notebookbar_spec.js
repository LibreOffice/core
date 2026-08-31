/* global describe expect it cy before after afterEach require Cypress */

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

	// Roles a user operates. Their node has to carry a name, or the reader
	// announces the kind of widget and nothing about it.
	const OPERABLE = [
		'button', 'tab', 'combobox', 'option', 'checkbox', 'radio', 'textbox',
		'link', 'menuitem', 'menuitemcheckbox', 'menuitemradio', 'treeitem',
		'spinbutton', 'slider', 'switch',
	];

	// Containers that carry no name today. An ARIA container of a kind not
	// listed here appearing without a name fails.
	const UNNAMED_CONTAINERS = [
		'toolbar', 'list', 'group', 'generic', 'none', 'presentation',
		'paragraph', 'image',
	];

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
		// Selecting an object already raises its contextual tab, and clicking a
		// tab that is selected collapses the notebookbar.
		cy.cGet(selector).should('be.visible').then(function ($label) {
			if (!$label.hasClass('selected'))
				cy.cGet(selector).click();
		});
		cy.cGet(selector).should('have.class', 'selected');

		helper.processToIdle(win);
		a11yHelper.runA11yValidation(win, 'validatenotebookbara11y');
		if (a11yHelper.axTreeAvailable())
			sweepTree(tab);

		visitedTabNames.push(tab.name);
	}

	// runA11yValidation reads the markup; this reads the name and role the
	// browser computed from it, so a label that resolves to nothing is caught.
	function sweepTree(tab) {
		const container = '#' + tab.name + '-container';

		a11yHelper.getAXNodesWithin(container).then(function (nodes) {
			expect(nodes.length, 'accessibility nodes under ' + container)
				.to.be.greaterThan(0);

			const nameless = [];
			const unexpected = [];

			nodes.forEach(function (node) {
				// An ignored node is not handed to a screen reader, so it has
				// no computed name to judge. queryAXTree returns the inner
				// element of every toolitem, ignored in favour of the wrapper
				// that carries the name.
				if (node.ignored || node.roleType !== 'role') return;
				if (node.name.trim() !== '') return;

				if (OPERABLE.indexOf(node.role) !== -1)
					nameless.push(node);
				else if (UNNAMED_CONTAINERS.indexOf(node.role) === -1)
					unexpected.push(node);
			});

			return Cypress.Promise.all([
				Cypress.Promise.all(nameless.map(a11yHelper.describeAXNode)),
				Cypress.Promise.all(unexpected.map(a11yHelper.describeAXNode)),
			]).then(function (found) {
				expect(found[0], 'operable widgets with no accessible name in ' + container)
					.to.be.empty;
				expect(found[1], 'containers of an unlisted kind with no name in ' + container)
					.to.be.empty;
			});
		});
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
