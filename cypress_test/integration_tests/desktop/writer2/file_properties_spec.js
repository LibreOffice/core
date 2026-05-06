/* -*- js-indent-level: 8 -*- */
/* global describe it cy beforeEach require */

var helper = require('../../common/helper');
var writerHelper = require('../../common/writer_helper');
const desktopHelper = require('../../common/desktop_helper');

describe(['tagdesktop', 'tagnextcloud', 'tagproxy'], 'File Property Tests', function() {

	beforeEach(function() {
		cy.viewport(1400, 1000);
		helper.setupAndLoadDocument('writer/file_properties.odt');
		desktopHelper.switchUIToNotebookbar();
		cy.getFrameWindow().then((win) => {
			this.win = win;
		});
	});

	it.skip('Add File Description.', function() {
		writerHelper.openFileProperties(this.win);

		cy.cGet('#tabcontrol-2').click();

		cy.cGet('#title-input.ui-edit').should('be.visible');;
		cy.cGet('#title-input.ui-edit').type('New Title');
		cy.cGet('#comments.ui-textarea').type('New');

		cy.cGet('#ok.ui-pushbutton-wrapper').click();

		writerHelper.openFileProperties(this.win);

		cy.cGet('#tabcontrol-2').click();

		cy.cGet('#title-input.ui-edit').should('have.value', 'New Title');
		cy.cGet('#comments.ui-textarea').should('have.value', 'New');

		cy.cGet('#cancel.ui-pushbutton-wrapper button').click();
	});

	it.skip('Add Custom Property.', function() {
		writerHelper.openFileProperties(this.win);

		cy.cGet('#tabcontrol-3').click();

		// Add property
		cy.cGet('#add.ui-pushbutton-wrapper').click();
		cy.cGet('#namebox-input-dialog').type('Mailstop');

		cy.cGet('#valueedit-input').type('123 Address');
		cy.cGet('#ok.ui-pushbutton-wrapper button').click();

		// Check property saved
		writerHelper.openFileProperties(this.win);

		cy.cGet('#tabcontrol-3').click();

		cy.cGet('#valueedit-input').should('have.value', '123 Address');
		cy.cGet('#namebox-input-dialog').should('have.value', 'Mailstop');

		cy.cGet('#cancel.ui-pushbutton-wrapper').click();
	});

	it.skip('Add Custom Duration Property.', function() {
		writerHelper.openFileProperties(this.win);

		cy.cGet('#tabcontrol-3').click();

		// Add property
		cy.cGet('#add.ui-pushbutton-wrapper').click();
		cy.cGet('#namebox-input-dialog').type('Received from');
		cy.cGet('#typebox-input').select('Duration');
		cy.cGet('#durationbutton').click();
		cy.cGet('#negative-input').check();
		cy.cGet('#years-input').type('1');
		cy.cGet('#days-input').type('2');
		cy.cGet('#seconds-input').type('3');

		// click the sub-dialog ok button
		cy.cGet('#ok.ui-pushbutton-wrapper button').invoke('slice', 1).click();
		helper.processToIdle(this.win); // give a bit of time to spin the loop and update jsdialogs
		cy.cGet('#ok.ui-pushbutton-wrapper button').click();

		// Check property saved
		writerHelper.openFileProperties(this.win);

		cy.cGet('#tabcontrol-3').click();

		cy.cGet('#duration-input').should('have.value', '- Y: 1 M: 0 D: 2 H: 0 M: 0 S: 3');
		cy.cGet('#namebox-input-dialog').should('have.value', 'Received from');
		cy.cGet('#cancel.ui-pushbutton-wrapper button').click();
	});

	it.skip('Add Custom Yes/No Property.', function() {
		writerHelper.openFileProperties(this.win);

		cy.cGet('#tabcontrol-3').click();

		// Add property
		cy.cGet('#add.ui-pushbutton-wrapper').click();
		cy.cGet('#namebox-input-dialog').type('Telephone number');
		cy.cGet('#typebox-input').select('Yes or no');
		cy.cGet('#yes-input').check();
		cy.cGet('#ok.ui-pushbutton-wrapper button').click();

		// Check property saved
		writerHelper.openFileProperties(this.win);

		cy.cGet('#tabcontrol-3').click();

		cy.cGet('#yes-input').should('be.checked');
		cy.cGet('#namebox-input-dialog').should('have.value', 'Telephone number');
		cy.cGet('#cancel.ui-pushbutton-wrapper button').click();
	});
});
