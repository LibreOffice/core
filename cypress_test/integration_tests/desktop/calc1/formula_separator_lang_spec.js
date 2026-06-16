/* global describe it cy beforeEach require */

var helper = require('../../common/helper');
var calcHelper = require('../../common/calc_helper');

// The formula separators depend on the document language: a comma-decimal
// locale (German) uses a semicolon as the function argument separator, while a
// dot-decimal locale (English) uses a comma. In a multi-locale process the
// separators were being taken from a global locale that lagged the view
// language, so each locale ended up with the other one's separators.

function cellValue(win, address, expected) {
	calcHelper.enterCellAddressAndConfirm(win, address);
	helper.copy();
	cy.cGet('#copy-paste-container table td')
		.should('not.contain.text', 'Err')
		.invoke('text')
		.should('match', expected);
}

describe(['tagdesktop', 'tagnextcloud', 'tagproxy'], 'Formula separators in a comma-decimal language', function() {
	beforeEach(function() {
		helper.setupAndLoadDocument('calc/cell_edit.fods', false, false, 'de-DE');
		cy.getFrameWindow().then((win) => { this.win = win; });
	});

	it('comma is read as the decimal separator', function() {
		// CA980 is an empty cell in this document.
		calcHelper.enterCellAddressAndConfirm(this.win, 'CA980');
		// Before the fix the comma was read as an argument separator, so this
		// became "=2;3" and the cell showed Err:509.
		helper.typeIntoDocument('=2,3{enter}');
		cellValue(this.win, 'CA980', /^2[.,]3$/);
	});
});

describe(['tagdesktop', 'tagnextcloud', 'tagproxy'], 'Formula separators in a dot-decimal language', function() {
	beforeEach(function() {
		helper.setupAndLoadDocument('calc/cell_edit.fods', false, false, 'en-US');
		cy.getFrameWindow().then((win) => { this.win = win; });
	});

	it('comma is read as the argument separator', function() {
		calcHelper.enterCellAddressAndConfirm(this.win, 'CA980');
		// Before the fix the English session received the semicolon separator,
		// so this comma-separated call failed.
		helper.typeIntoDocument('=SUM(2,3){enter}');
		cellValue(this.win, 'CA980', /^5$/);
	});
});
