/* global describe it cy require beforeEach expect */

var helper = require('../../common/helper');
var calcHelper = require('../../common/calc_helper');

describe(['tagmultiuser'], 'Multiuser Calc Cross-Document Copy Paste', function() {

	beforeEach(function() {
		helper.setupAndLoadTwoDocuments(
			'calc/cell_cursor_jump.ods',
			'calc/cell_cursor_jump.ods');
	});

	it('Cross-doc paste special shows the paste-special dialog', function() {
		// Given a source document, a cell is copied to the clipboard:
		cy.cSetActiveFrame('#iframe2');
		cy.cGet(helper.addressInputSelector).type('{selectAll}A1{enter}');
		cy.cSetActiveFrame('#iframe1');
		cy.cGet(helper.addressInputSelector).type('{selectAll}A1{enter}');
		helper.setDummyClipboardForCopy('text/html');
		calcHelper.clickOnFirstCell();
		helper.copy();
		cy.cGet('#copy-paste-container').should(function($el) {
			expect($el.html()).to.contain('meta-origin');
		});

		// When pasting that copied HTML in an other Calc document as paste special:
		cy.cGet('#copy-paste-container').invoke('html').then(function(html) {
			cy.cSetActiveFrame('#iframe2');
			calcHelper.clickOnFirstCell();
			cy.getFrameWindow().then(function(win) {
				win.app.map._clip._navigatorClipboardPasteSpecial = true;
				win.app.map._clip.paste({
					clipboardData: {
						getData: function(t) {
							return t === 'text/html' ? html : '';
						},
						types: ['text/html'],
					},
					preventDefault: function() {},
				});
			});
		});

		// Then make sure the paste special dialog is visible:
		cy.cSetActiveFrame('#iframe2');
		cy.cGet('#PasteSpecialDialog').should('be.visible');
		// And it is the only dialog:
		// Without the accompanying fix in place, this test would have failed with:
		// - assert expected [ <form.jsdialog-container.ui-dialog.ui-widget-content.lokdialog_container>, 2 more... ] to have a length of 1 but got 3
		// i.e. this paste special dialog wasn't async, so all of the progress indicator,
		// the paste special dialog and the non-async warning dialog was visible.
		cy.cGet('.ui-dialog[role="dialog"]:not(.snackbar)').should('have.length', 1);
	});
});
