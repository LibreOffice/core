/* global describe it cy beforeEach require expect */

var helper = require('../../common/helper');
var desktopHelper = require('../../common/desktop_helper');
var calcHelper = require('../../common/calc_helper');

describe(['tagdesktop'], 'Top toolbar tests.', function() {
	var newFilePath;

	beforeEach(function() {
		newFilePath = helper.setupAndLoadDocument('calc/top_toolbar.ods');
		desktopHelper.switchUIToCompact();
		helper.typeIntoInputField(helper.addressInputSelector, 'A1');
		cy.getFrameWindow().then((win) => {
			this.win = win;
		});
	});

	it('Save.', function () {
		desktopHelper.getCompactIcon('Bold').click();
		cy.cGet('#save').click();

		helper.reloadDocument(newFilePath);

		cy.cGet(helper.addressInputSelector)
		.should('exist');

		desktopHelper.switchUIToCompact();
		calcHelper.clickOnFirstCell();

		cy.cGet(helper.addressInputSelector)
			.should('exist');

		helper.setDummyClipboardForCopy();
		calcHelper.selectEntireSheet();
		helper.copy();
		cy.cGet('#copy-paste-container table td b').should('exist');
	});

	it('Clone Formatting.', function() {
		helper.setDummyClipboardForCopy();
		helper.typeIntoDocument('{downarrow}');

		// Apply bold and try to clone it to the whole word.
		desktopHelper.getCompactIcon('Bold').click();
		desktopHelper.getCompactIcon('FormatPaintbrush').click();

		calcHelper.clickOnFirstCell(true,false);

		helper.typeIntoDocument('{shift}{downarrow}');
		helper.copy();

		cy.wait(1000);

		cy.cGet('#copy-paste-container tbody').find('td b').each(($el) => {
			cy.wrap($el)
				.should('exist');
		});
	});

	it('Clone Formatting persistent mode via double-click.', function() {
		// Move to A2 and apply bold.
		helper.typeIntoDocument('{downarrow}');
		desktopHelper.getCompactIcon('Bold').click();

		// Double-click FormatPaintbrush for persistent mode.
		desktopHelper.getCompactIcon('FormatPaintbrush').dblclick();

		cy.cGet('#document-canvas').should('have.class', 'bucket-cursor');

		// Apply formatting to A1.
		calcHelper.clickOnFirstCell(true, false);
		helper.processToIdle(this.win);

		// Bucket cursor should remain - this is persistent mode.
		cy.cGet('#document-canvas').should('have.class', 'bucket-cursor');

		// Press Escape to exit persistent mode.
		helper.typeIntoDocument('{esc}');
		cy.cGet('#document-canvas').should('not.have.class', 'bucket-cursor');
	});

	it('Print', function() {
		// A new window should be opened with the PDF.
		cy.getFrameWindow()
			.then(function(win) {
				cy.stub(win, 'open').as('windowOpen');
			});

		cy.cGet('#toolbar-up #printoptions .arrowbackground').click();
		cy.cGet('body').contains('.ui-combobox-entry', 'Active Sheet').click();

		cy.get('@windowOpen').should('be.called');
	});

	it('Enable text wrapping.', function() {
		cy.cGet(helper.addressInputSelector).should('have.prop', 'value', 'A1');
		// Get cursor position at end of line before wrap
		calcHelper.dblClickOnFirstCell();
		helper.moveCursor('end');
		helper.getCursorPos('left', 'currentTextEndPos');

		cy.get('@currentTextEndPos').should('be.greaterThan', 0);

		helper.initAliasToNegative('originalTextEndPos');
		cy.get('@currentTextEndPos').then(function(pos) {
			cy.wrap(pos).as('originalTextEndPos');
		});

		// Leave cell
		helper.typeIntoDocument('{enter}');
		// Wait for enter to work before clicking on first cell again
		cy.cGet(helper.addressInputSelector).should('have.prop', 'value', 'A2');
		helper.processToIdle(this.win);

		// Turn text wrap on
		calcHelper.clickOnFirstCell();
		desktopHelper.getCompactIcon('WrapText').click();

		// Leave cell
		helper.typeIntoDocument('{enter}');
		// Wait for enter to work before clicking on first cell again
		cy.cGet(helper.addressInputSelector).should('have.prop', 'value', 'A2');
		helper.processToIdle(this.win);

		// Get cursor position at end of line after wrap
		calcHelper.dblClickOnFirstCell();
		helper.moveCursor('end');
		helper.getCursorPos('left', 'currentTextEndPos');

		cy.get('@currentTextEndPos').then(function(currentTextEndPos) {
			cy.get('@originalTextEndPos').then(function(originalTextEndPos) {
				expect(currentTextEndPos).to.be.lessThan(originalTextEndPos);
			});
		});
	});

	it.skip('Merge cells', function() {

		// Select the full column
		calcHelper.selectFirstColumn();

		// Despite the selection is there, merge cells needs more time here.
		cy.wait(1000);

		desktopHelper.getCompactIcon('ToggleMergeCells').click();

		desktopHelper.checkDialogAndClose('Merge Cells');
	});

	it('Clear Direct formatting.', function() {
		helper.setDummyClipboardForCopy();
		desktopHelper.getCompactIcon('Bold').click();

		calcHelper.selectEntireSheet();
		helper.copy();

		cy.cGet('#copy-paste-container table td b').should('exist');
		desktopHelper.getCompactIcon('ResetAttributes').click();

		calcHelper.selectEntireSheet();
		helper.copy();

		cy.cGet('#copy-paste-container table td b').should('not.exist');
	});

	it('Apply font style.', function() {
		helper.setDummyClipboardForCopy();
		cy.cGet('#toolbar-up #fontnamecombobox .ui-combobox-button').click();
		desktopHelper.selectFromListbox('Alef');
		calcHelper.selectEntireSheet();
		helper.copy();
		cy.cGet('#copy-paste-container table td font').should('have.attr', 'face', 'Alef');
	});

	it('Apply font size.', function() {
		helper.setDummyClipboardForCopy();
		cy.cGet('#toolbar-up #fontsizecombobox .ui-combobox-button').click();
		desktopHelper.selectFromListbox('12');
		calcHelper.selectEntireSheet();
		helper.copy();
		cy.cGet('#copy-paste-container table td font').should('have.attr', 'size', '3');
	});

	it('Apply bold font.', function() {
		helper.setDummyClipboardForCopy();
		desktopHelper.getCompactIcon('Bold').click();
		calcHelper.selectEntireSheet();
		helper.copy();
		cy.cGet('#copy-paste-container table td b').should('exist');
	});

	it('Apply underline.', function() {
		helper.setDummyClipboardForCopy();
		desktopHelper.getCompactIcon('Underline').click();
		calcHelper.selectEntireSheet();
		helper.copy();
		cy.cGet('#copy-paste-container table td u').should('exist');
	});

	it('Apply italic.', function() {
		helper.setDummyClipboardForCopy();
		desktopHelper.getCompactIcon('Italic').click();
		calcHelper.selectEntireSheet();
		helper.copy();
		cy.cGet('#copy-paste-container table td i').should('exist');
	});

	it('Apply strikethrough.', function() {
		helper.setDummyClipboardForCopy();
		desktopHelper.getCompactIcon('Strikeout').click();
		calcHelper.selectEntireSheet();
		helper.copy();
		cy.cGet('#copy-paste-container table td s').should('exist');
	});

	it('Apply highlight color.', function() {
		helper.setDummyClipboardForCopy();
		desktopHelper.getCompactIconArrow('BackgroundColor').click();
		desktopHelper.selectColorFromPalette('3FAF46');
		calcHelper.selectEntireSheet();
		helper.copy();
		cy.cGet('#copy-paste-container table td').should('have.attr', 'bgcolor', '#3FAF46');
	});

	it('Apply font color.', function() {
		helper.setDummyClipboardForCopy();
		desktopHelper.getCompactIconArrow('Color').click();
		desktopHelper.selectColorFromPalette('FFB66C');
		calcHelper.selectEntireSheet();
		helper.copy();
		cy.cGet('#copy-paste-container table td font').should('have.attr', 'color', '#FFB66C');
	});

	it('Add/Delete decimal places', function() {
		helper.setDummyClipboardForCopy();
		// Add decimal place
		desktopHelper.getCompactIconArrow('DefaultNumbering').click();
		desktopHelper.getCompactIcon('NumberFormatIncDecimals').click();
		calcHelper.selectEntireSheet();
		helper.copy();

		var regex = new RegExp(';0;0.0$');
		cy.cGet('#copy-paste-container table td')
			.should('have.attr', 'sdnum')
			.should('match', regex);

		// Delete Decimal place
		calcHelper.clickOnFirstCell();

		desktopHelper.getCompactIconArrow('DefaultNumbering').click();
		desktopHelper.getCompactIcon('NumberFormatDecDecimals').click();

		calcHelper.selectEntireSheet();
		helper.copy();
		regex = new RegExp(';0;0$');
		cy.cGet('#copy-paste-container table td')
			.should('have.attr', 'sdnum')
			.should('match', regex);
	});

	it('Format as currency.', function() {
		helper.setDummyClipboardForCopy();
		desktopHelper.getCompactIconArrow('DefaultNumbering').click();
		desktopHelper.getCompactIcon('NumberFormatCurrency').click();

		calcHelper.selectEntireSheet();
		helper.copy();

		var regex = new RegExp(';0;\\[\\$\\$-409]#,##0.00;\\[RED]-\\[\\$\\$-409]#,##0.00$');
		cy.cGet('#copy-paste-container table td')
			.should('have.attr', 'sdnum')
			.should('match', regex);
	});

	it('Format as Percent.', function() {
		helper.setDummyClipboardForCopy();
		desktopHelper.getCompactIconArrow('DefaultNumbering').click();
		desktopHelper.getCompactIcon('NumberFormatPercent').click();

		calcHelper.selectEntireSheet();
		helper.copy();

		var regex = new RegExp(';0;0.00%$');
		cy.cGet('#copy-paste-container table td')
			.should('have.attr', 'sdnum')
			.should('match', regex);
	});

	it('Apply left/right alignment', function() {
		helper.setDummyClipboardForCopy();
		// Set right alignment first
		cy.cGet('#textalign .arrowbackground').click();
		cy.cGet('body').contains('.ui-combobox-entry', 'Align Right').click();
		calcHelper.selectEntireSheet();
		helper.copy();
		cy.cGet('#copy-paste-container table td').should('have.attr', 'align', 'right');

		// Change alignment back
		calcHelper.clickOnFirstCell();

		cy.cGet('#textalign .arrowbackground').click();
		cy.cGet('body').contains('.ui-combobox-entry', 'Align Left').click();

		calcHelper.selectEntireSheet();
		helper.copy();
		cy.cGet('#copy-paste-container table td').should('have.attr', 'align', 'left');
	});

});
