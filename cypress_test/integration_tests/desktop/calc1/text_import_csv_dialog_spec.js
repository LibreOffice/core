/* global describe it cy require */

var helper = require('../../common/helper');

describe(['tagdesktop'], 'Text Import (CSV) dialog', function () {
	it('open a .csv, split with the ruler and hide a column', { defaultCommandTimeout: 60000 }, function () {
		var ruler = '.ui-csv-grid-container .ui-csv-ruler-canvas';
		var headers = '.ui-csv-grid-container .ui-csv-grid-col-header';

		var press = function (selector, key) {
			cy.cGet(selector).trigger('keydown', { key: key, bubbles: true });
		};

		cy.viewport(1280, 960);

		var filePath = helper.setupDocument('calc/text_import.csv');
		helper.loadDocument(filePath, true);

		cy.cGet('form.jsdialog-container.lokdialog_container').should('exist');
		cy.cGet('.ui-csv-grid-container').should('exist');

		// switch to fixed width
		cy.cGet('#tofixedwidth-input').check();
		cy.cGet(ruler).should('exist');
		// one column spanning the whole line: XXHHHHHYYY
		cy.cGet(headers).should('have.length', 1);

		// drop a split after position 2  (XX | HHHHHYYY)
		press(ruler, 'ArrowRight');
		press(ruler, ' ');
		cy.cGet(headers).should('have.length', 2);

		// drop a split after position 7  (XX | HHHHH | YYY)
		for (var i = 0; i < 5; i++) press(ruler, 'ArrowRight');
		press(ruler, ' ');
		cy.cGet(headers).should('have.length', 3);

		// select the middle column (HHHHH)
		cy.cGet(headers).eq(1).trigger('keydown', { key: ' ', bubbles: true });
		cy.cGet(headers).eq(1).should('have.class', 'selected');

		// hide it
		cy.cGet('#columntype-input').should('not.be.disabled');
		cy.cGet('#columntype-input').select('Hide');
		cy.cGet(headers).eq(1).should('have.text', 'Hide');

		cy.cGet('.ui-pushbutton.jsdialog.button-primary').click();
		cy.cGet('form.jsdialog-container.lokdialog_container').should('not.exist');

		// The spreadsheet only starts loading once the import dialog is
		// confirmed. Wait for it to load before touching the view-mode UI,
		// otherwise entering edit mode runs against a document layer that
		// does not exist yet and throws.
		cy.cGet('.leaflet-canvas-container canvas').should('exist');
		cy.getFrameWindow().then(function (win) {
			helper.processToIdle(win);
		});

		// Enter edit mode
		cy.cGet('#viewModeDropdownButton-button').click();
		cy.cGet('#viewModeDropdownButton-entry-1').click();
		cy.cGet('#modal-dialog-switch-to-edit-mode-modal-yesbutton').click();
		cy.cGet('#addressInput input').should('be.visible');

		// the hidden column is dropped: A1 = "XX", B1 = "YYY"
		helper.setDummyClipboardForCopy();
		helper.typeIntoInputField(helper.addressInputSelector, 'A1:B1');
		helper.copy();
		cy.cGet('#copy-paste-container table td').should('have.length', 2);
		cy.cGet('#copy-paste-container table td').eq(0).should('have.text', 'XX');
		cy.cGet('#copy-paste-container table td').eq(1).should('have.text', 'YYY');
	});
});
