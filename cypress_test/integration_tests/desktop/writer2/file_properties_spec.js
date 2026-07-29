/* -*- js-indent-level: 8 -*- */
/* global describe it cy beforeEach require */

var helper = require('../../common/helper');
var writerHelper = require('../../common/writer_helper');
const desktopHelper = require('../../common/desktop_helper');

describe(['tagdesktop', 'tagnextcloud', 'tagproxy'], 'File Property Tests', { testIsolation: false }, function() {

	desktopHelper.shareDocumentAcrossTests('writer/file_properties.odt', {
		notebookbar: true,
		viewport: [1400, 1000],
	});

	// Cypress builds the test context again for every test, so the frame window is
	// fetched per test. Taking it once in before would leave it behind after the
	// first test.
	beforeEach(function() {
		cy.getFrameWindow().then((win) => {
			this.win = win;
		});
	});

	// Every custom property row is built from the same fragment, so the rows have to
	// be told apart by position rather than by name. The dialog keeps a few spare
	// rows hidden instead of throwing them away, so only the ones on screen count.
	// The controls inside a row are reached by class: the ids repeat from row to row,
	// only the first row of the run keeps the plain id, and an id written as #id comes
	// back as a single element however many share it.
	function shownRow(position) {
		return cy.cGet('#properties .ui-grid:visible').eq(position);
	}

	function nameBox(position) {
		return shownRow(position).find('input.ui-combobox-content');
	}

	function typeBox(position) {
		return shownRow(position).find('select.ui-listbox');
	}

	// The value box a row shows depends on the type of property it holds: the plain
	// text box for a text property, the duration box for a duration one.
	function valueBox(position) {
		return shownRow(position).find('input.ui-edit:visible');
	}

	// Yes comes before No in the row.
	function yesButton(position) {
		return shownRow(position).find('.ui-radiobutton input').first();
	}

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

	it('General tab hides the file location row in Online.', function() {
		writerHelper.openFileProperties(this.win);

		// Online only has a meaningless jail path and the file name is shown
		// elsewhere, so the whole Location row (label, value and Open button) is
		// hidden. Each of the three carries the hidden class itself, which says the
		// row was hidden on purpose. Asking whether it is visible would not: the
		// dialog reopens on whichever tab was last visited, and everything on a tab
		// that is not the one in front counts as invisible too.
		cy.cGet('.jsdialog-window #label8').should('have.class', 'hidden');
		cy.cGet('.jsdialog-window #showlocation').should('have.class', 'hidden');
		cy.cGet('.jsdialog-window #btnShowLocation').should('have.class', 'hidden');

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

	// This test and the one after it count the rows on the Custom Properties page,
	// so both need a document that has no custom properties yet. The test that adds
	// some is last in the file, and that file order is the only thing keeping these
	// two ahead of it.
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

	// One property of each type, taken through its whole life: typed in, kept by OK,
	// and read back from the reopened dialog. This is last in the file because it is
	// the only test that leaves properties on the document.
	it('Custom properties of every type come back when the dialog is reopened.', function() {
		writerHelper.openFileProperties(this.win);

		cy.cGet('#customprops').click();

		// A text property.
		cy.cGet('#add.ui-pushbutton-wrapper').click();
		nameBox(0).type('Mailstop');
		valueBox(0).type('123 Address');

		// A duration property, whose value is entered in a sub-dialog. The button
		// that opens it is labelled "...", which tells it from the row's Remove
		// button, the only other button a row has.
		cy.cGet('#add.ui-pushbutton-wrapper').click();
		nameBox(1).type('Received from');
		typeBox(1).select('Duration');
		shownRow(1).find('button').contains('...').click();
		cy.cGet('#negative-input').check();
		cy.cGet('#years-input').type('1');
		cy.cGet('#days-input').type('2');
		cy.cGet('#seconds-input').type('3');
		// Two OK buttons are on screen; the second one belongs to the sub-dialog.
		cy.cGet('#ok.ui-pushbutton-wrapper button').invoke('slice', 1).click();
		helper.processToIdle(this.win); // give a bit of time to spin the loop and update jsdialogs

		// A yes/no property.
		cy.cGet('#add.ui-pushbutton-wrapper').click();
		nameBox(2).type('Telephone number');
		typeBox(2).select('Yes or no');
		yesButton(2).check();

		cy.cGet('#ok.ui-pushbutton-wrapper button').click();

		writerHelper.openFileProperties(this.win);

		cy.cGet('#customprops').click();

		// The reopened page sorts the rows by name, so the three are read back in
		// alphabetical order rather than the order they were typed in.
		nameBox(0).should('have.value', 'Mailstop');
		valueBox(0).should('have.value', '123 Address');

		nameBox(1).should('have.value', 'Received from');
		valueBox(1).should('have.value', '- Y: 1 M: 0 D: 2 H: 0 M: 0 S: 3');

		nameBox(2).should('have.value', 'Telephone number');
		yesButton(2).should('be.checked');

		cy.cGet('#cancel.ui-pushbutton-wrapper button').click();
	});
});
