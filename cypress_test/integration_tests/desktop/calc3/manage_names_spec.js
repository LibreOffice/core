/* global describe it cy require beforeEach expect */

var helper = require('../../common/helper');

describe(['tagdesktop'], 'Manage Names dialog focus', function() {

	beforeEach(function() {
		helper.setupAndLoadDocument('calc/manage_names.fods');
		cy.viewport(1920, 1080);
	});

	it('Focus moves to another dialog control when delete button becomes disabled', function() {
		var win;

		cy.getFrameWindow().then(function(frameWindow) {
			win = frameWindow;
			win.app.map.sendUnoCommand('.uno:DefineName');
		});

		cy.cGet('.ui-dialog[role="dialog"]').should('have.length', 1);
		cy.then(function() {
			return helper.processToIdle(win);
		});

		cy.cGet('#delete-button').should('not.be.disabled');

		cy.cGet('#delete-button').focus();
		cy.cGet('#delete-button').should('have.focus');

		cy.cGet('#delete-button').click();

		cy.then(function() {
			return helper.processToIdle(win);
		});

		cy.cGet('#delete-button').should('be.disabled');

		cy.then(function() {
			var active = win.document.activeElement;
			var dialog = win.document.querySelector('.ui-dialog[role="dialog"]');
			var contains = dialog ? dialog.contains(active) : false;
			expect(contains).to.be.true;
		});
	});
});
