/* global describe expect it cy before require Cypress */

const helper = require('../../common/helper');
const desktopHelper = require('../../common/desktop_helper');
const a11yHelper = require('../../common/a11y_helper');

// Where the deployment keeps screen reading on for everyone, the Help tab's
// button is greyed out. The state comes the way it does on a real host: a
// userid of its own routes the test WOPI server to a preset store, and the
// browsersetting.json checked in there is what DocumentBroker reads before it
// sends lockaccessibilityon.
describe(['tagdesktop'], 'Screen Reading button locked on', { testIsolation: false }, function () {
	let win;

	before(function () {
		cy.viewport(1920, 1080);
		const brand = Cypress.env('brandTheme');
		const query = 'userid=a11ylocked' + (brand ? '&theme=' + brand : '');

		helper.setupAndLoadDocument('writer/help_dialog.odt', false, false, undefined, query);

		cy.getFrameWindow().then(function (frameWindow) { win = frameWindow; });

		desktopHelper.switchUIToNotebookbar();
		cy.cGet('#Help-tab-label').click();
		cy.cGet('#Help-container').should('be.visible');
		cy.cGet('#togglea11ystate').should('exist');
	});

	function label() {
		return win.document.getElementById('togglea11ystate').querySelector('.ui-checkbox-label');
	}

	it('the deployment locked it on, without the spec saying so', function () {
		cy.wrap(null).should(function () {
			expect(win.app.map._lockAccessibilityOn,
				'the lock the server sent').to.be.true;
		});
	});

	it('and a screen reader is told the checkbox is off limits', function () {
		cy.then(function () { return a11yHelper.getAXNodesWithin('#togglea11ystate'); })
			.then(function (nodes) {
				const checkbox = nodes.filter(function (node) {
					return node.role === 'checkbox';
				})[0];

				expect(checkbox, 'an AX node for the checkbox').to.not.be.undefined;
				expect(checkbox.properties.disabled, 'the checkbox, greyed out').to.be.true;
			});
	});

	it('and its label stays readable while it is greyed out', function () {
		cy.then(function () {
			const seat = a11yHelper.effectiveBackground(win, label());
			const drawn = a11yHelper.renderedTextColor(win, label());

			expect(a11yHelper.contrastRatio(drawn, seat),
				'greyed out: ' + drawn + ' on ' + seat).to.be.at.least(4.5);
		});
	});
});
