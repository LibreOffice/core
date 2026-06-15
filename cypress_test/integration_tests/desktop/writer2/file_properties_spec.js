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

	it('Add File Description.', function() {
		writerHelper.openFileProperties(this.win);

		cy.cGet('#description').click();

		cy.cGet('#title-input.ui-edit').should('be.visible');;
		cy.cGet('#title-input.ui-edit').type('New Title');
		cy.cGet('#comments.ui-textarea').type('New');

		cy.cGet('#ok.ui-pushbutton-wrapper').click();

		writerHelper.openFileProperties(this.win);

		cy.cGet('#description').click();

		cy.cGet('#title-input.ui-edit').should('have.value', 'New Title');
		cy.cGet('#comments.ui-textarea').should('have.value', 'New');

		cy.cGet('#cancel.ui-pushbutton-wrapper button').click();
	});

	it('Add Custom Property.', function() {
		writerHelper.openFileProperties(this.win);

		cy.cGet('#customprops').click();

		// Add property
		cy.cGet('#add.ui-pushbutton-wrapper').click();
		cy.cGet('#namebox-input-dialog').type('Mailstop');

		cy.cGet('#valueedit-input').type('123 Address');
		cy.cGet('#ok.ui-pushbutton-wrapper button').click();

		// Check property saved
		writerHelper.openFileProperties(this.win);

		cy.cGet('#customprops').click();

		cy.cGet('#valueedit-input').should('have.value', '123 Address');
		cy.cGet('#namebox-input-dialog').should('have.value', 'Mailstop');

		cy.cGet('#cancel.ui-pushbutton-wrapper').click();
	});

	it('Add Custom Duration Property.', function() {
		writerHelper.openFileProperties(this.win);

		cy.cGet('#customprops').click();

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

		cy.cGet('#customprops').click();

		cy.cGet('#duration-input').should('have.value', '- Y: 1 M: 0 D: 2 H: 0 M: 0 S: 3');
		cy.cGet('#namebox-input-dialog').should('have.value', 'Received from');
		cy.cGet('#cancel.ui-pushbutton-wrapper button').click();
	});

	it('Add Custom Yes/No Property.', function() {
		writerHelper.openFileProperties(this.win);

		cy.cGet('#customprops').click();

		// Add property
		cy.cGet('#add.ui-pushbutton-wrapper').click();
		cy.cGet('#namebox-input-dialog').type('Telephone number');
		cy.cGet('#typebox-input').select('Yes or no');
		cy.cGet('#yes-input').check();
		cy.cGet('#ok.ui-pushbutton-wrapper button').click();

		// Check property saved
		writerHelper.openFileProperties(this.win);

		cy.cGet('#customprops').click();

		cy.cGet('#yes-input').should('be.checked');
		cy.cGet('#namebox-input-dialog').should('have.value', 'Telephone number');
		cy.cGet('#cancel.ui-pushbutton-wrapper button').click();
	});

	it('General tab hides the file location row in Online.', function() {
		writerHelper.openFileProperties(this.win);

		// Online only has a meaningless jail path and the file name is shown
		// elsewhere, so the whole Location row (label, value and Open button)
		// is hidden.
		cy.cGet('#label8').should('not.be.visible');
		cy.cGet('#showlocation').should('not.be.visible');
		cy.cGet('#btnShowLocation').should('not.be.visible');

		cy.cGet('#cancel.ui-pushbutton-wrapper button').click();
	});

	it('Statistics tab Update fills in the line count.', function() {
		writerHelper.openFileProperties(this.win);

		// The Writer statistics page must render as a JSDialog for its Update
		// button to work.
		cy.cGet('#writerstats').click();

		// The line count is computed on demand, so it is empty until Update is
		// pressed; pressing it must populate the value.
		cy.cGet('#nolines').invoke('text').should('not.match', /[0-9]/);
		cy.cGet('#update.ui-pushbutton-wrapper').click();
		helper.processToIdle(this.win);
		cy.cGet('#nolines').invoke('text').should('match', /[0-9]/);

		cy.cGet('#cancel.ui-pushbutton-wrapper button').click();
	});

	it('Can keep adding custom properties (Add button stays).', function() {
		writerHelper.openFileProperties(this.win);

		cy.cGet('#customprops').click();

		// First property.
		cy.cGet('#add.ui-pushbutton-wrapper').click();
		cy.cGet('#namebox-input-dialog').should('be.visible').type('Prop1');

		// Regression: the Add button used to disappear after the first add, so
		// no further properties could be added.
		cy.cGet('#add.ui-pushbutton-wrapper').should('be.visible');

		// A second property can still be added -> two editable rows.
		cy.cGet('#add.ui-pushbutton-wrapper').click();
		cy.cGet('#properties .ui-combobox-content:visible').should('have.length', 2);
		cy.cGet('#add.ui-pushbutton-wrapper').should('be.visible');

		cy.cGet('#cancel.ui-pushbutton-wrapper button').click();
	});

	it('Duration edit button keeps its "..." label.', function() {
		writerHelper.openFileProperties(this.win);

		cy.cGet('#customprops').click();

		cy.cGet('#add.ui-pushbutton-wrapper').click();
		cy.cGet('#typebox-input').select('Duration');

		// The button label is "..."; the ellipsis-stripping in _cleanText must
		// not erase a label that is nothing but an ellipsis.
		cy.cGet('#durationbutton-button').should('be.visible').and('have.text', '...');

		cy.cGet('#cancel.ui-pushbutton-wrapper button').click();
	});
});
