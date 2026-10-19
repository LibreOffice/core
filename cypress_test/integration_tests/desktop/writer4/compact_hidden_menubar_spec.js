/* global describe it cy beforeEach require */

var helper = require('../../common/helper');
var desktopHelper = require('../../common/desktop_helper');

describe(['tagdesktop'], 'Compact view: the document button brings back a hidden menu bar', function() {

	beforeEach(function() {
		cy.viewport(1280, 800);
		helper.setupAndLoadDocument('writer/top_toolbar.odt');
		desktopHelper.switchUIToCompact();
		cy.cGet('#toolbar-up').should('be.visible');
		cy.getFrameWindow().then(function(win) {
			this.win = win;
		});
	});

	function hideMenubarFromViewMenu(win) {
		cy.cGet('#menu-view').click();
		cy.cGet('#menu-togglemenubar').click();
		helper.processToIdle(win);
		cy.cGet('.main-nav').should('not.be.visible');
	}

	it('shows no document button while the menu bar is shown', function() {
		cy.cGet('.main-nav').should('be.visible');
		cy.cGet('#toolbar-up .documentmenu').should('not.be.visible');
		cy.cGet('#documentmenubreak').should('not.be.visible');
	});

	it('shows the document button first in the toolbar once the menu bar is hidden', function() {
		hideMenubarFromViewMenu(this.win);

		cy.cGet('#toolbar-up .documentmenu').should('be.visible');
		cy.cGet('#toolbar-up .documentmenu').should('have.class', 'writer-icon-img');
		cy.cGet('#documentmenubreak').should('be.visible');
		cy.cGet('#overflow-manager-toptoolbar > :visible').first()
			.should('have.class', 'documentmenu');
	});

	it('keeps the document button out of the overflow at a narrow width', function() {
		hideMenubarFromViewMenu(this.win);
		cy.viewport(640, 800);
		helper.processToIdle(this.win);

		cy.cGet('#toolbar-up .documentmenu').should('be.visible');
		cy.cGet('.hidden-overflow-container .documentmenu').should('not.exist');
	});

	it('brings the menu bar back on click and moves focus to File', function() {
		hideMenubarFromViewMenu(this.win);

		cy.cGet('#toolbar-up .documentmenu').click();
		helper.processToIdle(this.win);

		cy.cGet('.main-nav').should('be.visible');
		cy.cGet('#toolbar-up .documentmenu').should('not.be.visible');
		cy.cGet('#documentmenubreak').should('not.be.visible');
		cy.cGet('#menu-file > a').should('have.focus');
	});

	it('brings the menu bar back with Enter and moves focus to File', function() {
		hideMenubarFromViewMenu(this.win);

		cy.cGet('#toolbar-up .documentmenu button').focus();
		cy.cGet('#toolbar-up .documentmenu button').should('have.focus');
		cy.realPress('Enter');
		helper.processToIdle(this.win);

		cy.cGet('.main-nav').should('be.visible');
		cy.cGet('#menu-file > a').should('have.focus');
	});

	it('shows no document button when the integrator hid it', function() {
		cy.getFrameWindow().then(function(win) {
			win.app.map.uiManager.showButton('documentmenu', false);
		});
		hideMenubarFromViewMenu(this.win);

		cy.cGet('#toolbar-up .documentmenu').should('not.be.visible');
	});
});
