/* global describe it cy before expect require Cypress */

const helper = require('../../common/helper');

// With forced colours the user picks the palette, and anything that keeps a
// colour of its own is what they cannot read. The Navigator kept two: the
// expander chevron of a collapsed row, darkened by a filter, and the close
// button, whose icon is a background image. Both are read off the page under
// the emulated media query rather than trusted from the stylesheet.
describe(['tagdesktop'], 'Navigator under forced colours', { testIsolation: false }, function () {
	let win;

	before(function () {
		cy.viewport(1920, 1080);
		helper.setupAndLoadDocument('calc/navigator.ods', false, false, undefined, 'vector=false');
		cy.getFrameWindow().then(function (frameWindow) { win = frameWindow; });
		cy.then(function () { win.app.map.sendUnoCommand('.uno:Navigator'); });
		cy.cGet('#contentbox').should('be.visible');
		cy.then(function () { return helper.processToIdle(win); });

		cy.then(function () {
			return Cypress.automation('remote:debugger:protocol', {
				command: 'Emulation.setEmulatedMedia',
				params: { features: [{ name: 'forced-colors', value: 'active' }] },
			});
		});
		cy.then(function () { return helper.processToIdle(win); });
		cy.then(function () {
			expect(win.matchMedia('(forced-colors: active)').matches,
				'the forced colours media query is on').to.be.true;
		});
	});

	function collapsedExpanders() {
		const rows = win.document.querySelectorAll('#contentbox [aria-expanded="false"]');
		return Array.prototype.map.call(rows, function (row) {
			return row.querySelector('.ui-treeview-expander');
		}).filter(Boolean);
	}

	function assertChevrons(theme) {
		const found = collapsedExpanders();
		expect(found.length, theme + ': the Navigator has collapsed rows').to.be.greaterThan(0);

		// What matters is that nothing dims it. Light drops the filter, dark
		// keeps an identity brightness so the theme's own handling survives.
		found.forEach(function (expander, i) {
			const filter = win.getComputedStyle(expander, '::before').filter;
			const dimmed = /brightness\(0(\.\d+)?\)/.test(filter);
			expect(dimmed, theme + ': row ' + i + ', the chevron is dimmed by ' + filter)
				.to.be.false;
		});
	}

	function assertCloseButton(theme) {
		const close = win.document.querySelector('.close-navigation-button');
		expect(close, theme + ': the Navigator has a close button').to.not.be.null;

		const style = win.getComputedStyle(close);
		const mask = style.maskImage || style.webkitMaskImage;

		expect(style.backgroundImage, theme + ': the icon is no longer a background image')
			.to.equal('none');
		expect(mask, theme + ': it is a mask instead').to.not.equal('none');
		expect(style.backgroundColor, theme + ': and it takes a colour of the palette')
			.to.not.equal('rgba(0, 0, 0, 0)');
	}

	it('every collapsed row keeps its chevron out of the filter', function () {
		cy.then(function () { assertChevrons('light'); });
	});

	it('the close button is painted in the user colour, not its own', function () {
		cy.then(function () { assertCloseButton('light'); });
	});

	// Both rules carry a [data-theme='dark'] variant, and the dark one for the
	// chevron sets a filter of its own, so the dark pass is not a formality.
	it('and in dark mode too', function () {
		cy.then(function () { win.app.map.uiManager.toggleDarkMode(); });
		cy.cframe().find('html').should('have.attr', 'data-theme', 'dark');
		cy.then(function () { return helper.processToIdle(win); });

		cy.then(function () {
			assertChevrons('dark');
			assertCloseButton('dark');
		});
	});
});
