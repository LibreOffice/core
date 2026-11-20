/* global describe it cy before expect require Cypress */

const helper = require('../../common/helper');

// The status bar's dropdown arrow is a CSS triangle, and under forced colours it
// was pinned to a fixed grey. What it owes the user is their own colour, so the
// expectation is resolved from the page rather than written down here.
describe(['tagdesktop'], 'Status bar under forced colours', { testIsolation: false }, function () {
	let win;

	before(function () {
		cy.viewport(1920, 1080);
		helper.setupAndLoadDocument('calc/navigator.ods', false, false, undefined, 'vector=false');
		cy.getFrameWindow().then(function (frameWindow) { win = frameWindow; });
		cy.then(function () { return helper.processToIdle(win); });

		cy.then(function () {
			return Cypress.automation('remote:debugger:protocol', {
				command: 'Emulation.setEmulatedMedia',
				params: { features: [{ name: 'forced-colors', value: 'active' }] },
			});
		});
		cy.then(function () {
			expect(win.matchMedia('(forced-colors: active)').matches,
				'the forced colours media query is on').to.be.true;
		});
	});

	// Ask the page what ButtonText resolves to, so the check follows the palette
	// the run is under instead of a value that is only true of one of them.
	function buttonText() {
		const probe = win.document.createElement('span');
		probe.style.color = 'ButtonText';
		win.document.body.appendChild(probe);
		const value = win.getComputedStyle(probe).color;
		probe.remove();
		return value;
	}

	function assertArrows(theme) {
		const arrows = win.document.querySelectorAll('#toolbar-down .unoarrow');
		expect(arrows.length, theme + ': the status bar has dropdown arrows').to.be.greaterThan(0);

		const expected = buttonText();
		arrows.forEach(function (arrow, i) {
			expect(win.getComputedStyle(arrow).borderTopColor,
				theme + ': arrow ' + i + ' takes the palette colour').to.equal(expected);
		});
	}

	it('the dropdown arrows take the user colour', function () {
		cy.then(function () { assertArrows('light'); });
	});

	it('and in dark mode too', function () {
		cy.then(function () { win.app.map.uiManager.toggleDarkMode(); });
		cy.cframe().find('html').should('have.attr', 'data-theme', 'dark');
		cy.then(function () { return helper.processToIdle(win); });
		cy.then(function () { assertArrows('dark'); });
	});
});
