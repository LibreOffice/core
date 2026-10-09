/* global describe expect it cy before require */

const helper = require('../../common/helper');

describe(['tagdesktop'], 'Writer image from the keyboard', { testIsolation: false }, function () {
	let win;

	before(function () {
		helper.setupAndLoadDocument('writer/a11y_image.fodt');

		cy.getFrameWindow().then(function (frameWindow) {
			win = frameWindow;
		});

		helper.typeIntoDocument('{ctrl}{home}');
		cy.then(function () {
			return helper.processToIdle(win);
		});
	});

	it('Shift+F4 selects an image and F4 opens its properties', function () {
		cy.realPress(['Shift', 'F4']);
		cy.then(function () {
			return helper.processToIdle(win);
		});
		cy.realPress('F4');
		cy.cGet('.ui-dialog-title').filter(':contains("Image")').should('be.visible');
		cy.cGet('.ui-dialog').should(function ($dialog) {
			expect($dialog[0].contains(win.document.activeElement), 'the focus in the dialog')
				.to.equal(true);
		});
		// already open: not a second one
		cy.realPress('F4');
		cy.then(function () {
			return helper.processToIdle(win);
		});
		cy.cGet('.ui-dialog-title').filter(':contains("Image")').should('have.length', 1);

		cy.realPress('Escape');
		cy.cGet('.ui-dialog-title:contains("Image")').should('not.exist');
	});
});
