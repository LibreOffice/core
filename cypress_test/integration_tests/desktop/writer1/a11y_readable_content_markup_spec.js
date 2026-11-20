/* global describe it cy before require */

const helper = require('../../common/helper');

describe(['tagdesktop'], 'Writer readable content with markup characters', { testIsolation: false }, function () {
	let win;

	before(function () {
		helper.setupAndLoadDocument('writer/readable_content_markup.fodt', false, false, undefined, 'vector=false');

		cy.getFrameWindow().then(function (frameWindow) {
			win = frameWindow;
		});

		cy.then(function () {
			win.app.map.setAccessibilityState(true);
			return helper.processToIdle(win);
		});
	});

	it('a paragraph that contains markup characters is read as the same text', function () {
		helper.typeIntoDocument('{ctrl}{home}');
		cy.then(function () {
			return helper.processToIdle(win);
		});

		helper.typeIntoDocument('{downarrow}');
		cy.then(function () {
			return helper.processToIdle(win);
		});

		cy.cGet('#readable-content').should('have.text', 'Text with <i>tags</i> & a < b');
		cy.cGet('#readable-content').children().should('have.length', 0);
	});
});
