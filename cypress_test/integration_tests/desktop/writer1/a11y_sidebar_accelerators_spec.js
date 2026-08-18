/* global describe it cy before require */

const helper = require('../../common/helper');
const desktopHelper = require('../../common/desktop_helper');

describe(['tagdesktop'], 'Sidebar accelerator info boxes', function () {
	let win;
	const sidebarJSON = [];

	before(function () {
		cy.viewport(1920, 1024);
		helper.setupAndLoadDocument('writer/help_dialog.odt');
		desktopHelper.switchUIToNotebookbar();

		cy.getFrameWindow().then(function (frameWindow) {
			win = frameWindow;
			win.app.map.on('sidebar', function (e) {
				sidebarJSON.push(e.data);
			});
		});

		// The page widgets live in the Page deck, not in the default Properties one.
		cy.then(function () {
			win.app.map.sendUnoCommand('.uno:SidebarDeck.WriterPageDeck');
			return helper.processToIdle(win);
		});


		cy.wrap(null, { timeout: 20000 }).should(function () {
			const combinations = win.app.UI.notebookbarAccessibility.definitions
				.sidebarCombinations[win.app.map.getDocType()] || {};
			const sidebar = win.document.getElementById('sidebar-dock-wrapper');

			Object.keys(combinations).forEach(function (id) {
				if (!sidebar || !sidebar.querySelector('[id="' + id + '"]'))
					throw new Error(id + ' is not in the sidebar yet');
			});
		});

		cy.wrap(null, { timeout: 20000 }).should(function () {
			if (!win.app.UI.notebookbarAccessibility.initialized)
				throw new Error('accessibility not initialized yet');
		});

		cy.then(function () {
			return helper.processToIdle(win).then(function () {
				const a11y = win.app.UI.notebookbarAccessibility;
				a11y.mayShowAcceleratorInfoBoxes = true;
				a11y.onDocumentKeyUp({ keyCode: 18 });
			});
		});
	});

	it('Page panel widgets show a clash free accelerator combination', function () {
		const definitions = win.app.UI.notebookbarAccessibility.definitions;
		const combinations = definitions.sidebarCombinations[win.app.map.getDocType()] || {};

		cy.wrap(Object.keys(combinations)).should('not.be.empty');

		cy.then(function () {
			const owners = [];
			function findOwners(node, panel) {
				if (!node) return;
				const owner = node.type === 'panel' ? node : panel;
				if (owner && node.type === 'listbox' && combinations[node.id + '-input'] !== undefined)
					owners.push(owner);
				(node.children || []).forEach(function (child) { findOwners(child, owner); });
			}
			sidebarJSON.forEach(function (payload) { findOwners(payload, null); });

			const listboxes = [];
			function collect(node) {
				if (!node) return;
				if (node.type === 'listbox') listboxes.push(node.id + '-input');
				(node.children || []).forEach(collect);
			}
			owners.forEach(collect);

			return listboxes.filter(function (id, at) { return listboxes.indexOf(id) === at; });
		}).should('not.be.empty').each(function (id) {
			cy.cGet('#sidebar-dock-wrapper #' + id).then(function ($widget) {
				if (!$widget.is(':visible')) {
					cy.wrap($widget).should('not.have.attr', 'accesskey');
					return;
				}

				cy.wrap(combinations).should('have.property', id);
				cy.wrap($widget).should('have.attr', 'accesskey', combinations[id]);
			});
		});

		cy.then(function () {
			const shown = Array.from(
				win.document.querySelectorAll('.accessibility-info-box')
			).map(function (box) { return box.textContent; });

			cy.wrap(shown).should('include.members', Object.values(combinations));

			const defs = definitions.getDefinitions();
			const seen = {};
			const clashes = [];

			Object.keys(defs).forEach(function (id) {
				const combination = defs[id].combination;
				if (!combination) return;
				if (seen[combination]) clashes.push(combination + ': ' + seen[combination] + ' vs ' + id);
				else seen[combination] = id;
			});

			cy.wrap(clashes).should('be.empty');
		});
	});
});
