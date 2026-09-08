/* global describe expect it cy before require Cypress */

const helper = require('../../common/helper');
const a11yHelper = require('../../common/a11y_helper');

describe(['tagdesktop'], 'Formula bar focus', { testIsolation: false }, function () {
	let win;
	let unfocused;

	before(function () {
		cy.viewport(1920, 1080);
		const brand = Cypress.env('brandTheme');
		helper.setupAndLoadDocument('calc/top_toolbar.ods', false, false, undefined,
			brand ? 'theme=' + brand : undefined);

		cy.getFrameWindow().then(function (frameWindow) {
			win = frameWindow;
		});
		cy.cGet('#sc_input_window').should('be.visible');

		cy.then(function () {
			const style = win.getComputedStyle(win.document.getElementById('sc_input_window'));

			expect(win.document.getElementById('sc_input_window').classList.contains('focused'),
				'the bar starts unfocused').to.be.false;

			unfocused = {
				outlineWidth: parseFloat(style.outlineWidth),
				outlineStyle: style.outlineStyle,
				background: a11yHelper.effectiveBackground(win, bar()),
			};
		});
	});

	function bar() {
		return win.document.getElementById('sc_input_window');
	}

	// F6 walks a ring, so the bar is some presses away.
	function pressF6Until(hasArrived) {
		const ring = 12;

		for (let i = 0; i < ring; i++) {
			cy.then(function () {
				if (hasArrived()) return;
				cy.realPress('F6');
			});
		}
	}

	it('F6 reaches the formula input, not the box beside it', function () {
		pressF6Until(function () { return win.app.map.calcInputBarHasFocus(); });

		cy.then(function () {
			expect(win.app.map.calcInputBarHasFocus(),
				'the ring reached the formula input').to.be.true;
			expect(bar().classList.contains('focused'),
				'the bar shows itself focused').to.be.true;
		});
	});

	it('the focus draws an indicator that stands out 3:1 from the field', function () {
		pressF6Until(function () { return win.app.map.calcInputBarHasFocus(); });

		cy.then(function () {
			const style = win.getComputedStyle(bar());
			const width = parseFloat(style.outlineWidth);

			expect(style.outlineStyle, 'the focused bar draws an outline')
				.to.not.equal('none');
			expect(width, 'the width of the focus ring').to.be.at.least(2);
			expect(unfocused.outlineStyle === 'none' || unfocused.outlineWidth < width,
				'the ring is not there when the bar is not focused').to.be.true;

			// An informative graphic needs 3:1 against what it sits on.
			const ratio = a11yHelper.contrastRatio(style.outlineColor, unfocused.background);

			expect(ratio, 'contrast of ' + style.outlineColor + ' against ' +
				unfocused.background).to.be.at.least(3);
		});
	});
	it('and the ring still stands out in dark mode', function () {
		cy.then(function () { win.app.map.uiManager.toggleDarkMode(); });
		cy.cframe().find('html').should('have.attr', 'data-theme', 'dark');

		cy.then(function () {
			const style = win.getComputedStyle(bar());
			const ratio = a11yHelper.contrastRatio(style.outlineColor,
				a11yHelper.effectiveBackground(win, bar()));

			expect(ratio, 'dark: ' + style.outlineColor + ' on ' + style.backgroundColor)
				.to.be.at.least(3);
		});
	});
});
