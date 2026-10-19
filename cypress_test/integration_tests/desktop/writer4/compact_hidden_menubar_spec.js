/* global describe it cy beforeEach require expect */

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

	function setPermission(permission) {
		cy.getFrameWindow().then(function(win) {
			win.app.map.setPermission(permission);
		});
	}

	it('shows a menu bar hidden by the user in read-only and hides it again in edit', function() {
		hideMenubarFromViewMenu(this.win);

		setPermission('readonly');
		helper.processToIdle(this.win);
		cy.cGet('.main-nav').should('be.visible');

		setPermission('edit');
		helper.processToIdle(this.win);
		cy.cGet('.main-nav').should('not.be.visible');
		cy.cGet('#toolbar-up .documentmenu').should('be.visible');
	});

	it('moves focus from the Editing switch to the document button when the header hides', function() {
		hideMenubarFromViewMenu(this.win);
		setPermission('readonly');
		helper.processToIdle(this.win);

		cy.cGet('#viewModeDropdownButton-button').focus();
		cy.cGet('#viewModeDropdownButton-button').should('have.focus');
		setPermission('edit');
		helper.processToIdle(this.win);

		cy.cGet('#toolbar-up .documentmenu button').should('have.focus');
	});

	it('keeps a menu bar hidden by the integrator hidden in read-only', function() {
		cy.getFrameWindow().then(function(win) {
			win.app.map.uiManager.hideMenubar();
		});
		helper.processToIdle(this.win);
		cy.cGet('.main-nav').should('not.be.visible');

		setPermission('readonly');
		helper.processToIdle(this.win);
		cy.cGet('.main-nav').should('not.be.visible');
	});

	it('lists the same File entries as the menu bar', function() {
		cy.cGet('#menu-file').click();
		cy.cGet('#menu-file > ul > li > a:visible').should('have.length.greaterThan', 3);

		cy.getFrameWindow().should(function(win) {
			var expected = win.app.map.menubar.getFileMenuEntries()
				.filter(function(entry) { return entry.type !== 'separator'; })
				.map(function(entry) { return entry.text; });
			var shown = Array.from(win.document.querySelectorAll('#menu-file > ul > li > a:not(.separator)'))
				.filter(function(link) { return link.checkVisibility(); })
				.map(function(link) { return link.firstChild.textContent.trim(); });
			expect(shown).to.deep.equal(expected);
		});
	});
});
