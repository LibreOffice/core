/* global describe expect it cy before require Cypress */

const helper = require('../../common/helper');
const desktopHelper = require('../../common/desktop_helper');
const a11yHelper = require('../../common/a11y_helper');

// An entry that opens a submenu says so with an arrow, and an informative
// graphic has to stand out 3:1 from what it sits on - in every state, since a
// focused entry paints its own background behind it.
describe(['tagdesktop'], 'Submenu arrow', { testIsolation: false }, function () {
	let win;

	before(function () {
		cy.viewport(1920, 1080);
		helper.setupAndLoadDocument('calc/switch.ods');

		cy.getFrameWindow().then(function (frameWindow) {
			win = frameWindow;
		});

		desktopHelper.switchUIToNotebookbar();
		cy.cGet('#Home-container').should('be.visible');
		cy.cGet('#Home-container .unoConditionalFormatMenu').first().should('be.visible').click();
		cy.cGet('.ui-combobox-entry.ui-has-menu').should('exist');
	});

	function withSubmenu() {
		return Array.from(win.document.querySelectorAll('.ui-combobox-entry.ui-has-menu'))
			.filter(function (entry) { return entry.offsetParent !== null; });
	}

	function opaque(colour) {
		const parts = String(colour).match(/[\d.]+/g);

		return !!parts && (parts.length < 4 || parseFloat(parts[3]) === 1);
	}

	/// The arrow has to carry its own colour for the contrast to be measurable
	/// at all: baked into the asset it is neither readable here nor bound to
	/// the palette. What it sits on is the entry's own text background.
	function arrowAgainstItsSeat(entry) {
		const arrow = win.getComputedStyle(entry, '::after');
		const span = entry.querySelector('span');
		const seat = a11yHelper.effectiveBackground(win, span || entry);

		expect(opaque(arrow.backgroundColor),
			'the arrow of ' + entry.textContent.trim() + ' carries a colour of its own')
			.to.be.true;

		return {
			colour: arrow.backgroundColor,
			seat: seat,
			ratio: a11yHelper.contrastRatio(arrow.backgroundColor, seat),
			drawn: arrow.visibility === 'visible' && parseFloat(arrow.width) > 0,
		};
	}

	it('every entry that opens a submenu draws one', function () {
		cy.then(function () {
			const entries = withSubmenu();

			expect(entries, 'entries with a submenu').to.not.be.empty;

			entries.forEach(function (entry) {
				const arrow = arrowAgainstItsSeat(entry);

				expect(arrow.drawn, 'an arrow on ' + entry.textContent.trim()).to.be.true;
			});
		});
	});

	it('the arrow stands out 3:1 from the entry it sits on', function () {
		cy.then(function () {
			withSubmenu().forEach(function (entry) {
				const arrow = arrowAgainstItsSeat(entry);

				expect(arrow.ratio, entry.textContent.trim() + ': ' + arrow.colour +
					' on ' + arrow.seat).to.be.at.least(3);
			});
		});
	});

	it('and still does once the entry has the focus', function () {
		let focused;

		cy.then(function () {
			focused = withSubmenu()[0];
			focused.focus();
			focused.classList.add('selected');
		});

		cy.then(function () {
			const arrow = arrowAgainstItsSeat(focused);

			expect(arrow.drawn, 'the arrow of the focused entry').to.be.true;
			expect(arrow.ratio, 'focused ' + focused.textContent.trim() + ': ' +
				arrow.colour + ' on ' + arrow.seat).to.be.at.least(3);
		});
	});

	// The arrow is a mask painted in background-color, and forced colours
	// override that property with the surface behind it, so the shape is drawn
	// in the colour of its own seat and goes.
	describe('under forced colours', function () {
		before(function () {
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

		it('the arrow is still there and still stands out', function () {
			cy.then(function () {
				const entries = withSubmenu();

				expect(entries, 'entries with a submenu').to.not.be.empty;

				entries.forEach(function (entry) {
					const arrow = arrowAgainstItsSeat(entry);

					expect(arrow.drawn, 'an arrow on ' + entry.textContent.trim()).to.be.true;
					expect(arrow.ratio, entry.textContent.trim() + ': ' + arrow.colour +
						' on ' + arrow.seat).to.be.at.least(3);
				});
			});
		});
	});
});
