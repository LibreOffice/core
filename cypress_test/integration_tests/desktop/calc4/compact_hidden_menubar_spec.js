/* global describe it cy beforeEach require */

var helper = require('../../common/helper');
var desktopHelper = require('../../common/desktop_helper');

describe(['tagdesktop'], 'Compact view: the document button brings back a hidden menu bar in Calc', function() {

	beforeEach(function() {
		cy.viewport(1280, 800);
		helper.setupAndLoadDocument('calc/top_toolbar.ods');
		desktopHelper.switchUIToCompact();
		cy.cGet('#toolbar-up').should('be.visible');
		cy.getFrameWindow().then(function(win) {
			this.win = win;
		});
	});

	it('brings the menu bar back and moves focus to File', function() {
		cy.cGet('#menu-view').click();
		cy.cGet('#menu-togglemenubar').click();
		helper.processToIdle(this.win);
		cy.cGet('.main-nav').should('not.be.visible');
		cy.cGet('#toolbar-up .documentmenu').should('have.class', 'calc-icon-img');

		cy.cGet('#toolbar-up .documentmenu').click();
		helper.processToIdle(this.win);

		cy.cGet('.main-nav').should('be.visible');
		cy.cGet('#menu-file > a').should('have.focus');
	});
});
