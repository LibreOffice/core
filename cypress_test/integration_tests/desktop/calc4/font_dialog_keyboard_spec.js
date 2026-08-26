/* global describe expect it cy beforeEach require */

var helper = require('../../common/helper');
var calcHelper = require('../../common/calc_helper');

describe(['tagdesktop', 'tagnextcloud', 'tagproxy'], 'Font dialog keyboard navigation', function() {
	beforeEach(function() {
		helper.setupAndLoadDocument('calc/top_toolbar.ods');
		cy.getFrameWindow().then((win) => {
			this.win = win;
		});
	});

	it('Tab past the last widget wraps around to the first', function() {
		calcHelper.dblClickOnFirstCell();
		cy.then(() => {
			this.win.app.map.sendUnoCommand('.uno:FontDialog');
		});

		cy.cGet('.ui-dialog[role="dialog"]').should('have.length', 1);
		helper.processToIdle(this.win);

		// The cycle wraps on the first and the last of the dialog's focusables,
		// so read them from the dialog rather than naming widgets.
		const describeElement = (element) =>
			element ? element.tagName + '#' + (element.id || '<no id>') : 'nothing';

		cy.cGet('.ui-dialog[role="dialog"]').then(($dialog) => {
			const order = this.win.JSDialog.GetFocusableElements($dialog[0]);
			expect(order.length, 'focusable widgets of the dialog').to.be.greaterThan(1);
			order[order.length - 1].focus();
		});

		cy.realPress('Tab');
		cy.cGet('.ui-dialog[role="dialog"]').should(($dialog) => {
			const order = this.win.JSDialog.GetFocusableElements($dialog[0]);
			expect(describeElement(this.win.document.activeElement),
				'Tab from the last widget').to.equal(describeElement(order[0]));
		});

		cy.realPress(['Shift', 'Tab']);
		cy.cGet('.ui-dialog[role="dialog"]').should(($dialog) => {
			const order = this.win.JSDialog.GetFocusableElements($dialog[0]);
			expect(describeElement(this.win.document.activeElement),
				'Shift+Tab from the first widget').to.equal(
				describeElement(order[order.length - 1]));
		});

		helper.typeIntoDocument('{esc}');
	});

	it('Up Arrow from the first font entry focuses the search field', function() {
		calcHelper.dblClickOnFirstCell();
		cy.then(() => {
			this.win.app.map.sendUnoCommand('.uno:FontDialog');
		});

		cy.cGet('.ui-dialog[role="dialog"]').should('have.length', 1);
		helper.processToIdle(this.win);

		// The font-name list is a treeview with its own search field.
		cy.cGet('#trWestFontName .ui-treeview-search-input').should('exist');

		// Focus the first font entry (focus, not click, so nothing is committed).
		cy.cGet('#trWestFontName .ui-treeview-entry').first().focus();
		cy.cGet('#trWestFontName .ui-treeview-entry').first().should('have.focus');

		// Up Arrow on the first entry returns focus to the search field.
		cy.realPress('ArrowUp');
		cy.cGet('#trWestFontName .ui-treeview-search-input').should('have.focus');

		helper.typeIntoDocument('{esc}');
	});
});
