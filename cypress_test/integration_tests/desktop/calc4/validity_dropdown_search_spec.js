/* global describe it cy beforeEach require */

var helper = require('../../common/helper');
var calcHelper = require('../../common/calc_helper');

// The cell drop-down of a "list" data validation lets the user jump to an
// entry by typing. The search must build up across keystrokes, match the
// typed text anywhere in the entry, and treat digits the same as letters.
// The list holds Apple, Sunny, Stormy, 100, 250 and Maybe.

describe(['tagdesktop', 'tagnextcloud', 'tagproxy'], 'Validity dropdown search', function() {
	var win;

	beforeEach(function() {
		helper.setupAndLoadDocument('calc/validity_list_dropdown.fods');
		cy.getFrameWindow().then(function(w) {
			win = w;
		});
		// The cursor lands on A1, the cell that carries the list validation.
		calcHelper.clickOnFirstCell();
		cy.then(function() {
			return helper.processToIdle(win);
		});
	});

	function openDropdownAndFocusList() {
		cy.getFrameWindow().then(function(w) {
			w.app.map.sendUnoCommand('.uno:DataSelect');
		});
		cy.cGet('.autofilter .ui-treeview-entry').should('have.length', 6);
		// Focus the list without clicking an entry, which would commit it.
		cy.cGet('.autofilter .ui-treeview-entry').first().focus();
	}

	function focusedEntry() {
		return cy.cGet('.autofilter .ui-treeview-entry:focus');
	}

	it('typing a digit jumps to a numeric entry', function() {
		openDropdownAndFocusList();

		// "250" begins with a digit, which the old search dropped outright.
		cy.realPress('2');
		focusedEntry().should('contain.text', '250');
	});

	it('successive keystrokes narrow to a substring match', function() {
		openDropdownAndFocusList();

		// "o", "r", "m" together match "Stormy", and only as a substring, not
		// as a prefix. The old search restarted on every key and matched only
		// the start, so it landed on "Maybe" (the last key) instead.
		cy.realPress('o');
		cy.realPress('r');
		cy.realPress('m');
		focusedEntry().should('contain.text', 'Stormy');
	});
});
