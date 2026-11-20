/* global describe it cy before expect require Cypress */

const helper = require('../../common/helper');

// A toolitem's icon is an <img>, and forced colours cannot reach the colours
// inside one. Each icon that is a plain shape publishes its URL, and the
// stylesheet paints it as a mask in the user's colour, so the icon follows the
// palette instead of sitting on a plate the user did not choose.
describe(['tagdesktop'], 'Toolitem icons under forced colours', { testIsolation: false }, function () {
	let win;

	before(function () {
		cy.viewport(1920, 1080);
		helper.setupAndLoadDocument('calc/navigator.ods', false, false, undefined, 'vector=false');
		cy.getFrameWindow().then(function (frameWindow) { win = frameWindow; });
		cy.then(function () { return helper.processToIdle(win); });
	});

	function maskable() {
		return win.document.querySelectorAll('.unotoolbutton img.icon-maskable');
	}

	// Ask the page what ButtonText is, so the check follows the run's palette.
	function buttonText() {
		const probe = win.document.createElement('span');
		probe.style.color = 'ButtonText';
		win.document.body.appendChild(probe);
		const value = win.getComputedStyle(probe).color;
		probe.remove();
		return value;
	}

	it('an SVG icon publishes its URL and keeps its alt', function () {
		cy.then(function () {
			const icons = maskable();
			expect(icons.length, 'the bars carry maskable icons').to.be.greaterThan(0);

			icons.forEach(function (icon, i) {
				expect(icon.style.getPropertyValue('--icon-url'),
					'icon ' + i + ' published its URL').to.contain('.svg');
				expect(icon.hasAttribute('alt'),
					'icon ' + i + ' still carries an alt').to.be.true;
			});
		});
	});

	it('nothing is masked while the palette is ours', function () {
		cy.then(function () {
			const icon = maskable()[0];
			const mask = win.getComputedStyle(icon).maskImage || win.getComputedStyle(icon).webkitMaskImage;
			expect(mask, 'the icon draws itself normally').to.equal('none');
		});
	});

	it('and every one takes the user colour once forced', function () {
		cy.then(function () {
			return Cypress.automation('remote:debugger:protocol', {
				command: 'Emulation.setEmulatedMedia',
				params: { features: [{ name: 'forced-colors', value: 'active' }] },
			});
		});
		cy.then(function () {
			expect(win.matchMedia('(forced-colors: active)').matches).to.be.true;
		});
		cy.then(function () {
			const expected = buttonText();
			const icons = maskable();

			icons.forEach(function (icon, i) {
				const style = win.getComputedStyle(icon);
				const mask = style.maskImage || style.webkitMaskImage;
				expect(mask, 'icon ' + i + ' is drawn as a mask').to.not.equal('none');
				expect(style.backgroundColor,
					'icon ' + i + ' takes the palette colour').to.equal(expected);
			});

			// The box has to survive, or the bars collapse.
			const box = icons[0].getBoundingClientRect();
			expect(box.width, 'the icon keeps its width').to.be.greaterThan(0);
			expect(box.height, 'the icon keeps its height').to.be.greaterThan(0);
		});
	});
});
