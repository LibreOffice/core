/* global describe it cy require */

var helper = require('../../common/helper');

describe(['tagdesktop'], 'Import Options dialog for HTML content', function () {
	it('an .xls file holding HTML opens after the import options are confirmed', { defaultCommandTimeout: 60000 }, function () {
		var filePath = helper.setupDocument('calc/text_import_html.xls');
		helper.loadDocument(filePath, true);

		cy.cGet('form.jsdialog-container.lokdialog_container').should('exist');
		cy.cGet('#automatic-input').should('be.checked');
		cy.cGet('.ui-pushbutton.jsdialog.button-primary').click();
		cy.cGet('form.jsdialog-container.lokdialog_container').should('not.exist');

		// The spreadsheet only starts loading once the dialog is confirmed.
		cy.cGet('.leaflet-canvas-container canvas').should('exist');
		cy.getFrameWindow().then(function (win) {
			helper.processToIdle(win);
		});

		cy.cGet('#addressInput input').should('be.visible');

		helper.setDummyClipboardForCopy();
		helper.typeIntoInputField(helper.addressInputSelector, 'A1:B1');
		helper.copy();
		cy.cGet('#copy-paste-container table td').should('have.length', 2);
		cy.cGet('#copy-paste-container table td').eq(0).should('have.text', 'First');
		cy.cGet('#copy-paste-container table td').eq(1).should('have.text', 'Second');
	});
});
