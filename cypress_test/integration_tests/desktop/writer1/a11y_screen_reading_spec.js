/* global describe expect it cy before require Cypress */

const helper = require('../../common/helper');
const desktopHelper = require('../../common/desktop_helper');
const a11yHelper = require('../../common/a11y_helper');

// The Help tab's Screen Reading button is greyed out where the deployment keeps
// screen reading on for everyone. What the button shows has to reach a screen
// reader as well: that it is off limits, what it is there for, whether it is on,
// and a label its reader can make out.
describe(['tagdesktop'], 'Screen Reading button', { testIsolation: false }, function () {
	let win;

	before(function () {
		cy.viewport(1920, 1080);
		const brand = Cypress.env('brandTheme');
		helper.setupAndLoadDocument('writer/help_dialog.odt', false, false, undefined,
			brand ? 'theme=' + brand : undefined);

		cy.getFrameWindow().then(function (frameWindow) {
			win = frameWindow;
		});

		desktopHelper.switchUIToNotebookbar();
		cy.cGet('#Help-tab-label').click();
		cy.cGet('#Help-container').should('be.visible');
		cy.cGet('#togglea11ystate').should('exist');
	});

	function container() {
		return win.document.getElementById('togglea11ystate');
	}

	function label() {
		return container().querySelector('.unolabel');
	}

	function buttonNode() {
		return a11yHelper.getAXNodesWithin('#togglea11ystate-button')
			.then(function (nodes) {
				const button = nodes.filter(function (node) {
					return node.role === 'button';
				})[0];

				expect(button, 'an AX node for the button').to.not.be.undefined;
				return button;
			});
	}

	function lockOn() {
		cy.then(function () {
			win.app.map._lockAccessibilityOn = true;
			win.app.map.fire('a11ystatechanged');
		});

		cy.cGet('#togglea11ystate').should('have.attr', 'disabled');
	}

	function assertLabelReadable(state) {
		cy.then(function () {
			const seat = a11yHelper.effectiveBackground(win, label());
			const drawn = a11yHelper.renderedTextColor(win, label());
			const ratio = a11yHelper.contrastRatio(drawn, seat);

			expect(ratio, state + ': ' + drawn + ' on ' + seat)
				.to.be.at.least(4.5);
		});
	}

	it('says what it is there for', function () {
		cy.then(function () { return buttonNode(); }).then(function (button) {
			expect(button.description, 'a description of ' + button.name)
				.to.not.be.empty;
			expect(button.description, 'the description of ' + button.name)
				.to.equal(container().getAttribute('data-cooltip'));
		});
	});

	it('says whether screen reading is on', function () {
		cy.then(function () { return buttonNode(); }).then(function (button) {
			expect(String(button.properties.pressed),
				'the state of ' + button.name + ', which looks '
				+ (container().classList.contains('selected') ? 'on' : 'off'))
				.to.equal(String(container().classList.contains('selected')));
		});
	});

	it('keeps its label readable', function () {
		assertLabelReadable('enabled');
	});

	it('is off limits to a screen reader once it is locked on', function () {
		lockOn();

		cy.then(function () { return buttonNode(); }).then(function (button) {
			expect(button.properties.disabled, 'the button, greyed out').to.be.true;
		});
	});

	it('keeps it readable while it is greyed out', function () {
		lockOn();
		assertLabelReadable('greyed out');
	});

	it('and in dark mode too', function () {
		lockOn();
		cy.then(function () { win.app.map.uiManager.toggleDarkMode(); });
		cy.cframe().find('html').should('have.attr', 'data-theme', 'dark');
		cy.then(function () { return helper.processToIdle(win); });

		assertLabelReadable('greyed out, dark');
	});
});
