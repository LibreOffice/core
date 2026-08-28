/* global describe expect it cy before require Array */

const helper = require('../../common/helper');
const desktopHelper = require('../../common/desktop_helper');
const a11yHelper = require('../../common/a11y_helper');

// The twin of the writer spec. The toolbuttons are built by the same code, but
// draw reaches its formatting state another way: through the text of a shape,
// since it has no body text to select.
describe(['tagdesktop'], 'Draw toggle state', { testIsolation: false }, function () {
	let win;

	before(function () {
		cy.viewport(1920, 1080);
		helper.setupAndLoadDocument('draw/to_curve.fodg');

		cy.getFrameWindow().then(function (frameWindow) {
			win = frameWindow;
		});

		desktopHelper.switchUIToNotebookbar();
		cy.cGet('.notebookbar-tabs-container').should('be.visible');

		// Tab walks the objects of the page in document order, and Enter opens
		// the text of the one it reaches.
		helper.typeIntoDocument('{esc}');
		cy.realPress('Tab');
		cy.cGet('#test-div-shapeHandlesSection').should('exist');
		helper.typeIntoDocument('{enter}');
		cy.then(function () {
			return helper.processToIdle(win);
		});

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
			const pressed = Array.prototype.filter.call(
				win.document.querySelectorAll('#Home-container button'),
				function (b) { return b.classList.contains('selected'); });
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
