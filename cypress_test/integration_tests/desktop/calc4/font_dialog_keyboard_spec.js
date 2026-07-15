/* global describe it cy beforeEach require */

var helper = require('../../common/helper');
var calcHelper = require('../../common/calc_helper');

describe(['tagdesktop', 'tagnextcloud', 'tagproxy'], 'Font dialog keyboard navigation', function() {
	beforeEach(function() {
		helper.setupAndLoadDocument('calc/top_toolbar.ods');
	});

	it('Up Arrow from the first font entry focuses the search field', function() {
		calcHelper.dblClickOnFirstCell();
		cy.getFrameWindow().then(function(win) {
			win.app.map.sendUnoCommand('.uno:FontDialog');
		});

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
