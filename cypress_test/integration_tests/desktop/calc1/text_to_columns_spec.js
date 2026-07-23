/* global describe it cy require */

var helper = require('../../common/helper');

describe(['tagdesktop'], 'Text to Columns', function () {
	it('split a single cell into columns via the fixed-width ruler', { defaultCommandTimeout: 60000 }, function () {
		var ruler = '.ui-csv-grid-container .ui-csv-ruler-canvas';
		var headers = '.ui-csv-grid-container .ui-csv-grid-col-header';

		var press = function (selector, key) {
			cy.cGet(selector).trigger('keydown', { key: key, bubbles: true });
		};

		cy.viewport(1280, 960);

		helper.setupAndLoadDocument('calc/empty-selections.ods');

		// Put a single delimited-looking value into A1 and re-select it. Text
		// to Columns is only offered for a single-column selection.
		helper.typeIntoInputField(helper.addressInputSelector, 'A1');
		helper.typeIntoDocument('XXHHHHHYYY{enter}');
		helper.typeIntoInputField(helper.addressInputSelector, 'A1');

		// Text to Columns reuses ScImportAsciiDlg. The dialog must open
		// asynchronously under LOK; if it were launched with a blocking
		// Execute() this dispatch would hang and the grid below never appears.
		cy.getFrameWindow().then(function (win) {
			win.app.map.sendUnoCommand('.uno:TextToColumns');
		});

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

		cy.cGet('.ui-pushbutton.jsdialog.button-primary').click();
		cy.cGet('form.jsdialog-container.lokdialog_container').should('not.exist');

		cy.getFrameWindow().then(function (win) {
			helper.processToIdle(win);
		});

		// the cell is split across three columns: A1 = "XX", B1 = "HHHHH", C1 = "YYY"
		helper.setDummyClipboardForCopy();
		helper.typeIntoInputField(helper.addressInputSelector, 'A1:C1');
		helper.copy();
		cy.cGet('#copy-paste-container table td').should('have.length', 3);
		cy.cGet('#copy-paste-container table td').eq(0).should('have.text', 'XX');
		cy.cGet('#copy-paste-container table td').eq(1).should('have.text', 'HHHHH');
		cy.cGet('#copy-paste-container table td').eq(2).should('have.text', 'YYY');
	});
});
