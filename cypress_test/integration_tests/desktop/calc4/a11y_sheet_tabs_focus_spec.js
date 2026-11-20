/* global describe expect it cy before require Cypress */

const helper = require('../../common/helper');
const a11yHelper = require('../../common/a11y_helper');

describe(['tagdesktop'], 'Sheet tab focus', { testIsolation: false }, function () {
	let win;

	before(function () {
		cy.viewport(1920, 1080);
		// switch.ods carries two sheets, so one tab is inactive.
		const brand = Cypress.env('brandTheme');
		helper.setupAndLoadDocument('calc/switch.ods', false, false, undefined,
			(brand ? 'theme=' + brand + '&' : '') + 'vector=false');

		cy.getFrameWindow().then(function (frameWindow) {
			win = frameWindow;
		});
		cy.cGet('#spreadsheet-tab1').should('be.visible');
	});

	function inactiveTab() {
		return Array.from(win.document.querySelectorAll('button[id^="spreadsheet-tab"]'))
			.filter(function (tab) {
				return !tab.classList.contains('spreadsheet-tab-selected');
			})[0];
	}

	function assertRingStandsOut(theme) {
		let unfocused;

		// The tab may still hold the focus from an earlier check, and the
		// unfocused colour is what this measures against.
		cy.then(function () {
			expect(inactiveTab(), 'an inactive sheet tab in ' + theme).to.not.be.undefined;
			inactiveTab().blur();
			win.app.map.focus();
		});
		cy.then(function () {
			return new Cypress.Promise(function (resolve) {
				win.requestAnimationFrame(function () {
					win.requestAnimationFrame(resolve);
				});
			});
		});

		cy.then(function () {
			const tab = inactiveTab();

			expect(tab.matches(':focus-visible'), theme + ': the tab starts unfocused')
				.to.be.false;
			unfocused = a11yHelper.effectiveBackground(win, tab);
		});

		cy.then(function () { inactiveTab().focus(); });

		cy.then(function () {
			const tab = inactiveTab();
			const style = win.getComputedStyle(tab);
			const ratio = a11yHelper.contrastRatio(style.outlineColor, unfocused);

			expect(tab.matches(':focus-visible'), 'the tab took a visible focus').to.be.true;
			expect(style.outlineStyle, 'the focused tab draws an outline').to.not.equal('none');
			expect(parseFloat(style.outlineWidth), 'width of the ring in ' + theme)
				.to.be.at.least(2);
			expect(ratio, theme + ': ' + style.outlineColor + ' on ' + unfocused)
				.to.be.at.least(3);
		});
	}

	it('an inactive tab draws a ring that stands out 3:1', function () {
		assertRingStandsOut('light');
	});

	it('and it still stands out in dark mode', function () {
		cy.then(function () { win.app.map.uiManager.toggleDarkMode(); });
		cy.cframe().find('html').should('have.attr', 'data-theme', 'dark');
		cy.then(function () { return helper.processToIdle(win); });

		assertRingStandsOut('dark');
	});
});
