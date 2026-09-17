/* global describe expect it cy before require Cypress */

const helper = require('../../common/helper');
const calcHelper = require('../../common/calc_helper');
const a11yHelper = require('../../common/a11y_helper');

// A widget core draws as a picture is a focusable box in the browser and nothing
// marks the focus inside the image, so the ring on the box is the only indicator
// it can show. The tabs are walked rather than named, so a widget added to any
// page of the dialog is held to the same rule.
describe(['tagdesktop'], 'Drawn widget focus', { testIsolation: false }, function () {
	let win;

	before(function () {
		cy.viewport(1920, 1080);
		const brand = Cypress.env('brandTheme');
		helper.setupAndLoadDocument('calc/switch.ods', false, false, undefined,
			brand ? 'theme=' + brand : undefined);

		cy.getFrameWindow().then(function (frameWindow) { win = frameWindow; });

		calcHelper.dblClickOnFirstCell();
		cy.then(function () { win.app.map.sendUnoCommand('.uno:FormatCellDialog'); });
		cy.cGet('.ui-dialog[role="dialog"]').should('have.length', 1);
		cy.then(function () { return helper.processToIdle(win); });
		cy.cGet('.ui-dialog [role="tablist"]').should('exist');
	});

	function tabs() {
		return Array.from(win.document.querySelectorAll('.ui-dialog [role="tab"]'));
	}

	function drawnWidgetsIn(panel) {
		if (!panel) return [];

		return Array.from(
			panel.querySelectorAll('.ui-drawing-area-container [tabindex="0"]'))
			.filter(function (el) {
				return el.checkVisibility({
					visibilityProperty: true,
					contentVisibilityAuto: true,
				});
			});
	}

	function ring(el) {
		const style = win.getComputedStyle(el);

		if (style.outlineStyle !== 'none' && parseFloat(style.outlineWidth) > 0)
			return {
				width: parseFloat(style.outlineWidth),
				colour: style.outlineColor,
				fromBrowser: style.outlineStyle === 'auto',
			};

		const shadow = style.boxShadow;
		if (!shadow || shadow === 'none')
			return { width: 0, colour: null, fromBrowser: false };

		const lengths = (shadow.match(/(-?[\d.]+)px/g) || [])
			.map(function (px) { return Math.abs(parseFloat(px)); });

		return {
			width: Math.max.apply(null, lengths.concat([0])),
			colour: (shadow.match(/rgba?\([^)]*\)/) || [])[0],
			fromBrowser: false,
		};
	}

	function assertRing(el, theme) {
		const where = theme + ': ' + el.id;
		const drawn = ring(el);

		expect(el.matches(':focus-visible'), where + ' took a visible focus').to.be.true;

		// getComputedStyle reports 1px for outline-style auto, which is the
		// browser's own ring and is painted thicker than that, so measuring it
		// says nothing; only a ring a stylesheet drew has to answer for itself.
		if (drawn.fromBrowser) return;

		const seat = a11yHelper.effectiveBackground(win, el);

		expect(drawn.width, where + ' draws a ring of its own').to.be.at.least(2);
		expect(a11yHelper.contrastRatio(drawn.colour, seat),
			where + ': ' + drawn.colour + ' on ' + seat).to.be.at.least(3);
	}

	function walkPages(theme) {
		let seen = 0;

		tabs().forEach(function (tab) {
			cy.then(function () { tab.click(); });
			cy.then(function () { return helper.processToIdle(win); });

			cy.then(function () {
				const panel = win.document.getElementById(tab.getAttribute('aria-controls'));

				drawnWidgetsIn(panel).forEach(function (el) {
					seen += 1;
					el.focus();
					assertRing(el, theme);
				});
			});
		});

		cy.then(function () {
			expect(seen, theme + ': focusable drawn widgets found in the dialog')
				.to.be.greaterThan(0);
		});
	}

	it('every drawn widget the dialog offers draws a ring that stands out 3:1', function () {
		walkPages('light');
	});

	it('and it still stands out in dark mode', function () {
		cy.then(function () { win.app.map.uiManager.toggleDarkMode(); });
		cy.cframe().find('html').should('have.attr', 'data-theme', 'dark');
		cy.then(function () { return helper.processToIdle(win); });

		walkPages('dark');
	});
});
