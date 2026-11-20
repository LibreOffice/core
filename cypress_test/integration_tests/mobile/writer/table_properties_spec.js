/* global describe it cy require expect */

var helper = require('../../common/helper');
var mobileHelper = require('../../common/mobile_helper');
var writerHelper = require('../../common/writer_helper');

describe(['tagmobile', 'tagnextcloud', 'tagproxy'], 'Change table properties / layout via mobile wizard.', function() {

	function before(filePath) {
		helper.setupAndLoadDocument(filePath, false, false, undefined, 'vector=false');

		// Click on edit button
		mobileHelper.enableEditingMobile();
	}

	function openTablePanel() {
		mobileHelper.openMobileWizard();

		cy.cGet('#TableEditPanel > .ui-header')
			.should('not.have.class','disabled')
			.click();

		cy.cGet('.unoInsertRowsBefore').should('be.visible');
	}

	function selectFullTable() {
		cy.cGet('.unoSelectTable').click();
		cy.cGet('#split_merge .unoMergeCells').should('not.have.attr', 'disabled');
		helper.copy();
		cy.cGet('#copy-paste-container table').should('exist');
	}

	it('Insert row before.', function() {
		before('writer/table_properties.odt');
		helper.setDummyClipboardForCopy();
		openTablePanel();
		cy.cGet('.unoInsertRowsBefore').click();
		cy.cGet('.table-row-resize-marker').should('have.length', 4);
		selectFullTable();
		// Check rows / columns
		cy.cGet('#copy-paste-container tr')
			.should(function(rows) {
				expect(rows).to.have.lengthOf(4);
				expect(rows[0].textContent).to.not.have.string('text');
				expect(rows[1].textContent).to.have.string('text');
			});
		cy.cGet('#copy-paste-container td')
			.should('have.length', 8);
	});

	it('Insert row after.', function() {
		before('writer/table_properties.odt');
		helper.setDummyClipboardForCopy();
		openTablePanel();
		cy.cGet('.unoInsertRowsAfter').click();
		cy.cGet('.table-row-resize-marker').should('have.length', 4);
		selectFullTable();
		// Check rows / columns
		cy.cGet('#copy-paste-container tr')
			.should(function(rows) {
				expect(rows).to.have.lengthOf(4);
				expect(rows[0].textContent).to.have.string('text');
				expect(rows[1].textContent).to.not.have.string('text');
			});
		cy.cGet('#copy-paste-container td')
			.should('have.length', 8);
	});

	it('Insert column before.', function() {
		before('writer/table_properties.odt');
		helper.setDummyClipboardForCopy();
		openTablePanel();
		cy.cGet('.unoInsertColumnsBefore').click();
		cy.cGet('.table-column-resize-marker').should('have.length', 4);
		selectFullTable();
		// Check rows / columns
		cy.cGet('#copy-paste-container tr').should('have.length', 3);
		cy.cGet('#copy-paste-container td')
			.should(function(columns) {
				expect(columns).to.have.lengthOf(9);
				expect(columns[0].textContent).to.not.have.string('text');
				expect(columns[1].textContent).to.have.string('text');
			});
	});

	it('Insert column after.', function() {
		before('writer/table_properties.odt');
		helper.setDummyClipboardForCopy();
		openTablePanel();
		cy.cGet('.unoInsertColumnsAfter').click();
		cy.cGet('.table-column-resize-marker').should('have.length', 4);
		selectFullTable();
		// Check rows / columns
		cy.cGet('#copy-paste-container tr').should('have.length', 3);
		cy.cGet('#copy-paste-container td')
			.should(function(columns) {
				expect(columns).to.have.lengthOf(9);
				expect(columns[0].textContent).to.have.string('text');
				expect(columns[1].textContent).to.not.have.string('text');
			});
	});

	it('Delete row.', function() {
		before('writer/table_properties.odt');
		helper.setDummyClipboardForCopy();
		openTablePanel();
		cy.cGet('.unoDeleteRows').click();
		cy.cGet('.table-row-resize-marker').should('have.length', 2);

		selectFullTable();

		// Check rows / columns
		cy.cGet('#copy-paste-container tr')
			.should(function(rows) {
				expect(rows).to.have.lengthOf(2);
				expect(rows[0].textContent).to.not.have.string('text');
				expect(rows[1].textContent).to.not.have.string('text');
			});
		cy.cGet('#copy-paste-container td').should('have.length', 4);
	});

	it('Delete column.', function() {
		before('writer/table_properties.odt');
		// Insert column first
		openTablePanel();
		cy.cGet('.unoInsertColumnsBefore').click();
		cy.cGet('.table-column-resize-marker').should('have.length', 4);
		// Then delete it
		mobileHelper.closeMobileWizard();
		openTablePanel();
		cy.cGet('.unoDeleteColumns').click();
		cy.cGet('.table-column-resize-marker').should('have.length', 3);
	});

	it('Delete table.', function() {
		before('writer/table_properties.odt');
		openTablePanel();
		cy.cGet('.unoDeleteTable').click();
		cy.cGet('.table-column-resize-marker').should('not.exist');
		mobileHelper.closeMobileWizard();
		writerHelper.selectAllTextOfDoc();
		// Check markers are in the same row (we have text selection only)
		cy.cGet('.text-selection-handle-start').then(function(startHandle) {
				expect(startHandle).to.have.lengthOf(1);
				cy.cGet('.text-selection-handle-end').should(function(endHandle) {
					expect(endHandle).to.have.lengthOf(1);
					expect(startHandle[0].getBoundingClientRect().top).to.equal(endHandle[0].getBoundingClientRect().top);
				});
			});
	});

	it('Merge cells.', function() {
		before('writer/table_properties.odt');
		helper.setDummyClipboardForCopy();
		// Select 2x2 part of the table.
		helper.moveCursor('down', 'shift');
		helper.moveCursor('right', 'shift');
		// We use cursor position as the indicator of layout change.
		helper.getCursorPos('top', 'origCursorPos');
		openTablePanel();
		cy.cGet('#mobile-wizard .unoMergeCells').click();
		// Cursor was in the second row originally.
		// With merging two rows, the cursor is moved into the first row.
		cy.get('@origCursorPos')
			.then(function(origCursorPos) {
				cy.cGet('.blinking-cursor')
					.should(function(cursor) {
						expect(cursor.offset().top).to.be.lessThan(origCursorPos);
					});
			});

		selectFullTable();

		// Check rows / columns
		cy.cGet('#copy-paste-container tr').should('have.length', 2);
		cy.cGet('#copy-paste-container td').should('have.length', 3);
	});

	it('Change row height.', function() {
		before('writer/table_properties.odt');
		helper.setDummyClipboardForCopy();
		openTablePanel();
		cy.cGet('#rowheight .spinfield').should('have.value', '0');
		helper.typeIntoInputField('#rowheight .spinfield', '1.4', true, false);
		selectFullTable();
		// Check row height
		cy.cGet('#copy-paste-container td').should('have.attr', 'height', '125');
	});

	it('Change column width.', function() {
		before('writer/table_properties.odt');
		helper.setDummyClipboardForCopy();
		openTablePanel();
		cy.cGet('#columnwidth .spinfield').should('have.value', '1.42');
		helper.typeIntoInputField('#columnwidth .spinfield', '1.6', true, false);
		selectFullTable();
		// Check column width
		cy.cGet('#copy-paste-container td').should('have.attr', 'width', '145');
	});

	it('Set minimal row height.', function() {
		before('writer/table_with_text.odt');
		helper.setDummyClipboardForCopy();
		openTablePanel();
		selectFullTable();
		// Check old row height
		cy.cGet('#copy-paste-container td').first().should('have.attr', 'height');

		cy.cGet('#mobile-wizard .unoSetMinimalRowHeight').click();
		// Table still selected
		helper.copy();
		// Check new row height
		cy.cGet('#copy-paste-container td').should('not.have.attr', 'height');
	});

	it('Set optimal row height.', function() {
		before('writer/table_with_text.odt');
		helper.setDummyClipboardForCopy();
		openTablePanel();
		selectFullTable();
		cy.cGet('#copy-paste-container tr:nth-of-type(1) td:nth-of-type(1)').should('have.attr', 'height', '33');
		cy.cGet('#copy-paste-container tr:nth-of-type(2) td:nth-of-type(1)').should('not.have.attr', 'height');
		cy.cGet('#copy-paste-container tr:nth-of-type(3) td:nth-of-type(1)').should('not.have.attr', 'height');

		cy.cGet('#mobile-wizard .unoSetOptimalRowHeight').click();
		selectFullTable();

		cy.cGet('#copy-paste-container table td').should('have.length', 6);
		// Check new row height
		cy.cGet('#copy-paste-container td:nth-of-type(2n+1)').should('have.attr', 'height');
	});

	it('Distribute rows.', function() {
		before('writer/table_with_text.odt');
		helper.setDummyClipboardForCopy();

		// Select full table (3x2)
		openTablePanel();
		selectFullTable();

		cy.cGet('#mobile-wizard .unoDistributeRows').should('not.have.attr','disabled');
		cy.cGet('#mobile-wizard .unoDistributeRows').click();

		selectFullTable();

		// Check new row height
		cy.cGet('#copy-paste-container td')
			.should(function(items) {
				expect(items).have.lengthOf(6);
				for (var i = 0; i < items.length; i++) {
					if (i == 0 || i == 4)
						expect(items[i]).have.attr('height', '33');
					else if (i == 2)
						expect(items[i]).have.attr('height', '34');
					else
						expect(items[i]).not.have.attr('height');
				}
			});
	});

	it('Set minimal column width.', function() {
		before('writer/table_with_text.odt');
		helper.setDummyClipboardForCopy();

		// Select full table (3x2)
		openTablePanel();
		selectFullTable();

		cy.cGet('#mobile-wizard .unoSetMinimalColumnWidth').should('not.have.attr','disabled');
		cy.cGet('#mobile-wizard .unoSetMinimalColumnWidth').click();

		selectFullTable();

		cy.cGet('#copy-paste-container td').should('have.attr', 'width', '24');
	});

	it('Set optimal column width.', function() {
		before('writer/table_with_text.odt');
		helper.setDummyClipboardForCopy();

		// Select full table (3x2)
		openTablePanel();
		selectFullTable();

		cy.cGet('#mobile-wizard .unoEntireRow').should('not.have.attr','disabled');
		cy.cGet('#mobile-wizard .unoEntireRow').click();

		cy.cGet('#copy-paste-container table').should('exist');
		cy.cGet('.unoSetOptimalColumnWidth').click();
		selectFullTable();
		cy.cGet('#copy-paste-container td:nth-of-type(1n)').should('have.attr', 'width', '324');
		cy.cGet('#copy-paste-container td:nth-of-type(2n)').should('have.attr', 'width', '323');
	});

	it('Distribute columns.', function() {
		before('writer/table_with_text.odt');
		helper.setDummyClipboardForCopy();

		// Select full table (3x2)
		openTablePanel();
		selectFullTable();

		cy.cGet('#mobile-wizard .unoDistributeColumns').should('not.have.attr','disabled');
		cy.cGet('#mobile-wizard .unoDistributeColumns').click();

		selectFullTable();

		cy.cGet('#copy-paste-container td')
			.should('have.attr', 'width', '323');
	});
	//TODO: add split cell test
	//bug: https://github.com/CollaboraOnline/online/issues/3962
});
