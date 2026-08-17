/* global describe it cy before require */

const helper = require('../../common/helper');
const desktopHelper = require('../../common/desktop_helper');

describe(['tagdesktop'], 'Sidebar accelerator info boxes', function () {
	let win;

	before(function () {
		cy.viewport(1920, 1024);
		helper.setupAndLoadDocument('impress/sidebar.odp');
		desktopHelper.switchUIToNotebookbar();

		cy.getFrameWindow().then(function (frameWindow) {
			win = frameWindow;
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

	it('Slide panel widgets show a clash free accelerator combination', function () {
		const expected = {
			'paperformat-input': 'KF',
			'fillstyle-input': 'KB',
			'orientation-input': 'KO',
			'marginLB-input': 'KM',
			'masterslide-input': 'KS'
		};

		Object.keys(expected).forEach(function (id) {
			cy.cGet('#sidebar-dock-wrapper #' + id)
				.should('have.attr', 'accesskey', expected[id]);
		});

		cy.then(function () {
			const shown = Array.from(
				win.document.querySelectorAll('.accessibility-info-box')
			).map(function (box) { return box.textContent; });

			cy.wrap(shown).should('include.members', Object.values(expected));

			const defs = win.app.UI.notebookbarAccessibility.definitions.getDefinitions();
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
