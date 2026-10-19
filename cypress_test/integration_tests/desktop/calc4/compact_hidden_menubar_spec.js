/* global describe it cy beforeEach require expect */

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
		cy.cGet('#documentmenu-entries [data-filter-text="show menu bar"]').click();
		helper.processToIdle(this.win);

		cy.cGet('.main-nav').should('be.visible');
		cy.cGet('#menu-file > a').should('have.focus');
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
