/* global describe expect it cy before require Array */

const helper = require('../../common/helper');
const desktopHelper = require('../../common/desktop_helper');
const a11yHelper = require('../../common/a11y_helper');

// A toolbutton shows it is on with the selected class and announces it with
// aria-pressed. IsToggleButton decides whether the attribute is written at all,
// once, when the button is built, so a button can end up looking pressed and
// saying nothing.
describe(['tagdesktop'], 'Writer toggle state', { testIsolation: false }, function () {
	let win;

	function buttonsOf(container) {
		return Array.from(
			win.document.querySelector(container).querySelectorAll('button'));
	}

	before(function () {
		cy.viewport(1920, 1080);
		helper.setupAndLoadDocument('writer/copy_paste.odt');

		cy.getFrameWindow().then(function (frameWindow) {
			win = frameWindow;
		});

		desktopHelper.switchUIToNotebookbar();
		cy.cGet('.notebookbar-tabs-container').should('be.visible');

		helper.typeIntoDocument('{ctrl}{home}');
		helper.typeIntoDocument('toggle state probe');
		helper.typeIntoDocument('{shift}{home}');
		cy.then(function () {
			return helper.processToIdle(win);
		});
	});

	it('a pressed toolbutton announces that it is pressed', function () {
		cy.then(function () {
			win.app.map.sendUnoCommand('.uno:Bold');
			return helper.processToIdle(win);
		});

		cy.cGet('#Home-container').then(function () {
			const pressed = buttonsOf('#Home-container').filter(function (b) {
				return b.classList.contains('selected');
			});
			expect(pressed, 'something is pressed to check').to.not.be.empty;
			a11yHelper.assertToggleStatesAgree(win, '#Home-container', 'with bold on');
		});

	});

	it('the announced state follows the document', function () {
		cy.then(function () {
			win.app.map.sendUnoCommand('.uno:Bold');
			return helper.processToIdle(win);
		});

		cy.cGet('#Home-container').then(function () {
			a11yHelper.assertToggleStatesAgree(win, '#Home-container', 'with bold off');
		});

	});
});
