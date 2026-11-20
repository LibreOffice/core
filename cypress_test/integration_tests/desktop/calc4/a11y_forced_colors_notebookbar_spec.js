/* global describe it cy before expect require Cypress */

const helper = require('../../common/helper');
const a11yHelper = require('../../common/a11y_helper');

// A toolitem's button carried a light plate so the dark icon on it stayed
// legible. The icon takes the palette itself now, so the button has no reason
// to keep a colour of its own and can be left to the system's button colours.
describe(['tagdesktop'], 'Notebookbar under forced colours', { testIsolation: false }, function () {
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

	function home() {
		return win.document.querySelector('#Home-container');
	}

	it('no toolitem button keeps a colour of its own', function () {
		cy.then(function () {
			const buttons = home().querySelectorAll('.unotoolbutton.notebookbar .unobutton');
			expect(buttons.length, 'the Home tab has toolitem buttons').to.be.greaterThan(0);

			const pinned = [];
			buttons.forEach(function (button) {
				if (win.getComputedStyle(button).forcedColorAdjust === 'none')
					pinned.push(button.id || '?');
			});
			expect(pinned.join(', '), 'these are still pinned to their own colours').to.be.empty;
		});
	});

	it('their labels stay readable', function () {
		cy.then(function () {
			const labels = home().querySelectorAll('span.ui-content.unolabel');
			expect(labels.length, 'the Home tab has labels').to.be.greaterThan(0);

			labels.forEach(function (label) {
				const text = (label.innerText || '').trim();
				if (!text) return;
				const ratio = a11yHelper.contrastRatio(
					win.getComputedStyle(label).color,
					a11yHelper.effectiveBackground(win, label));
				expect(ratio, '"' + text.slice(0, 20) + '" against what it sits on')
					.to.be.at.least(4.5);
			});
		});
	});
});
