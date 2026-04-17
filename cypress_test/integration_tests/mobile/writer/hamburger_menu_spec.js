/* global describe it cy beforeEach require expect */

var helper = require('../../common/helper');
var mobileHelper = require('../../common/mobile_helper');
var writerHelper = require('../../common/writer_helper');
var repairHelper = require('../../common/repair_document_helper');

describe.skip(['tagmobile'], 'Trigger hamburger menu options.', function() {
	var newFilePath;

	beforeEach(function() {
		newFilePath = helper.setupAndLoadDocument('writer/hamburger_menu.odt');

		mobileHelper.enableEditingMobile();
	});

	function hideText() {
		// Change text color to white to hide text.
		writerHelper.selectAllTextOfDoc();
		mobileHelper.openMobileWizard();
		cy.cGet('#Color').contains('.ui-header','Font Color').click();
		mobileHelper.selectFromColorPicker('#Color', 0, 7);
		// End remove spell checking red lines
		mobileHelper.selectHamburgerMenuItem(['View', 'Automatic Spell Checking']);
		// Remove any selections.
		helper.moveCursor('left');
	}

	function openPageWizard() {
		mobileHelper.selectHamburgerMenuItem(['Page Setup']);
		cy.cGet('#mobile-wizard-content').should('not.be.empty');
	}

	function closePageWizard() {
		cy.cGet('#mobile-wizard-back').should('have.class', 'close-button');
		cy.cGet('#mobile-wizard-back').click();
		cy.cGet('#mobile-wizard').should('not.be.visible');
	}

	it('Save', { defaultCommandTimeout: 60000 }, function() {
		// Change the document content and save it
		writerHelper.selectAllTextOfDoc();
		cy.wait(1000);
		helper.typeIntoDocument('new');
		writerHelper.selectAllTextOfDoc();
		cy.wait(1000);
		helper.expectTextForClipboard('new');
		mobileHelper.selectHamburgerMenuItem(['File', 'Save']);
		// Reopen the document and check content.
		helper.reloadDocument(newFilePath);
		mobileHelper.enableEditingMobile();
		writerHelper.selectAllTextOfDoc();
		helper.expectTextForClipboard('new');
	});

	it('Print', function() {
		// A new window should be opened with the PDF.
		cy.getFrameWindow()
			.then(function(win) {
				cy.stub(win, 'open');
			});

		mobileHelper.selectHamburgerMenuItem(['File', 'Print']);

		cy.getFrameWindow().its('open').should('be.called');
	});

	it('Download as PDF', function() {
		mobileHelper.selectHamburgerMenuItem(['Download as', 'PDF Document (.pdf)']);
		mobileHelper.pressPushButtonOfDialog('Export');
		cy.cGet('iframe').should('have.attr', 'data-src').should('contain', 'download');
	});

	it('Download as ODT', function() {
		mobileHelper.selectHamburgerMenuItem(['Download as', 'ODF text document (.odt)']);
		cy.cGet('iframe').should('have.attr', 'data-src').should('contain', 'download');
	});

	it('Download as DOC', function() {
		mobileHelper.selectHamburgerMenuItem(['Download as', 'Word 2003 Document (.doc)']);
		cy.cGet('iframe').should('have.attr', 'data-src').should('contain', 'download');
	});

	it('Download as DOCX', function() {
		mobileHelper.selectHamburgerMenuItem(['Download as', 'Word Document (.docx)']);
		cy.cGet('iframe').should('have.attr', 'data-src').should('contain', 'download');
	});

	it('Download as RTF', function() {
		mobileHelper.selectHamburgerMenuItem(['Download as', 'Rich Text (.rtf)']);
		cy.cGet('iframe').should('have.attr', 'data-src').should('contain', 'download');
	});

	it('Download as EPUB', function() {
		mobileHelper.selectHamburgerMenuItem(['Download as', 'EPUB (.epub)']);
		mobileHelper.pressPushButtonOfDialog('OK');
		cy.cGet('iframe').should('have.attr', 'data-src').should('contain', 'download');
	});

	it('Undo/redo.', function() {
		// Type a new character
		helper.typeIntoDocument('q');
		writerHelper.selectAllTextOfDoc();
		cy.cGet('#copy-paste-container p').should('contain.text', 'q');
		// Undo
		mobileHelper.selectHamburgerMenuItem(['Edit', 'Undo']);
		writerHelper.selectAllTextOfDoc();
		cy.cGet('#copy-paste-container p').should('not.contain.text', 'q');
		// Redo
		mobileHelper.selectHamburgerMenuItem(['Edit', 'Redo']);
		writerHelper.selectAllTextOfDoc();
		cy.cGet('#copy-paste-container p').should('contain.text', 'q');
	});


	it('Repair Document', function() {
		helper.typeIntoDocument('Hello World');
		repairHelper.rollbackPastChange('Typing: “World”', undefined, true);
		helper.selectAllText();
		helper.expectTextForClipboard('Hello \n');
	});

	it('Cut.', function() {
		writerHelper.selectAllTextOfDoc();
		mobileHelper.selectHamburgerMenuItem(['Edit', 'Cut']);
		cy.cGet('#mobile-wizard-content-modal-dialog-copy_paste_warning-box').should('exist');
	});

	it('Copy.', function() {
		writerHelper.selectAllTextOfDoc();
		mobileHelper.selectHamburgerMenuItem(['Edit', 'Copy']);
		cy.cGet('#mobile-wizard-content-modal-dialog-copy_paste_warning-box').should('exist');
	});

	it('Paste.', function() {
		writerHelper.selectAllTextOfDoc();
		mobileHelper.selectHamburgerMenuItem(['Edit', 'Paste']);
		cy.cGet('#mobile-wizard-content-modal-dialog-copy_paste_warning-box').should('exist');
	});

	it('Select all.', function() {
		mobileHelper.selectHamburgerMenuItem(['Edit', 'Select All']);
		helper.textSelectionShouldExist();
		cy.cGet('#copy-paste-container p').should('contain.text', 'xxxxxx');
	});

	it('Enable track changes recording.', function() {
		mobileHelper.selectHamburgerMenuItem(['Track Changes', 'Record']);
		// Insert some text and check whether it's tracked.
		helper.typeIntoDocument('q');
		mobileHelper.selectHamburgerMenuItem(['Track Changes', 'Previous']);
		helper.textSelectionShouldExist();
		// We should have 'q' selected.
		helper.expectTextForClipboard('q');
		// Then disable recording.
		mobileHelper.selectHamburgerMenuItem(['Track Changes', 'Record']);
		helper.typeIntoDocument('{rightArrow}w');
		mobileHelper.selectHamburgerMenuItem(['Track Changes', 'Previous']);
		helper.textSelectionShouldExist();
		// We should have 'q' selected.
		helper.expectTextForClipboard('q');
	});

	it('Show track changes.', function() {
		mobileHelper.selectHamburgerMenuItem(['Track Changes', 'Record']);
		// Remove text content.
		cy.cGet('#document-container').click();
		helper.clearAllText();
		// By default track changed are shown.
		writerHelper.selectAllTextOfDoc();
		// No actual text sent from core because of the removal.
		helper.expectTextForClipboard('\n\n');
		// We have a multiline selection
		cy.cGet('.text-selection-handle-start')
			.then(function(firstMarker) {
				cy.cGet('.text-selection-handle-end')
					.then(function(secondMarker) {
						expect(firstMarker.offset().top).to.be.lessThan(secondMarker.offset().top);
						expect(firstMarker.offset().left).to.be.lessThan(secondMarker.offset().left);
					});
			});

		// Remove text selection.
		helper.typeIntoDocument('{leftArrow}');

		// Hide track changes.
		mobileHelper.selectHamburgerMenuItem(['Track Changes', 'Show']);

		// Trigger select all
		helper.typeIntoDocument('{ctrl}a');

		// Both selection markers should be in the same line
		cy.cGet('.text-selection-handle-start')
			.then(function(firstMarker) {
				cy.cGet('.text-selection-handle-end')
					.then(function(secondMarker) {
						expect(firstMarker.offset().top).to.be.equal(secondMarker.offset().top);
						expect(firstMarker.offset().left).to.be.lessThan(secondMarker.offset().left);
					});
			});
	});

	it('Accept all changes.', function() {
		mobileHelper.selectHamburgerMenuItem(['Track Changes', 'Record']);

		// Remove text content.
		cy.cGet('#document-container').click();
		helper.clearAllText();
		// Accept removal.
		mobileHelper.selectHamburgerMenuItem(['Track Changes', 'Accept All']);
		// Check that we don't have the removed content
		helper.typeIntoDocument('{ctrl}a');
		cy.wait(1000);
		// No selection
		helper.textSelectionShouldNotExist();
	});

	it('Reject all changes.', function() {
		mobileHelper.selectHamburgerMenuItem(['Track Changes', 'Record']);
		// Remove text content.
		cy.cGet('#document-container').click();
		helper.clearAllText();
		writerHelper.selectAllTextOfDoc();
		// We don't have actual text content.
		helper.expectTextForClipboard('\n\n');
		// Reject removal.
		mobileHelper.selectHamburgerMenuItem(['Track Changes', 'Reject All']);
		writerHelper.selectAllTextOfDoc();
		// We get back the content.
		cy.cGet('body').contains('#copy-paste-container p', 'xxxxxxx').should('exist');
	});

	it('Go to next/prev change.', function() {
		mobileHelper.selectHamburgerMenuItem(['Track Changes', 'Record']);
		// First change
		helper.typeIntoDocument('q');
		// Second change
		helper.typeIntoDocument('{rightArrow}w');
		// Find second change using prev.
		mobileHelper.selectHamburgerMenuItem(['Track Changes', 'Previous']);
		helper.expectTextForClipboard('w');
		// Find first change using prev.
		mobileHelper.selectHamburgerMenuItem(['Track Changes', 'Previous']);
		helper.expectTextForClipboard('q');
		// Find second change using next.
		mobileHelper.selectHamburgerMenuItem(['Track Changes', 'Next']);
		helper.expectTextForClipboard('w');
	});

	it('Search some word.', function() {
		mobileHelper.selectHamburgerMenuItem(['Search']);
		// Search bar become visible
		cy.cGet('#mobile-wizard-content').should('not.be.empty');
		// Search for some word
		helper.typeIntoInputField('#searchterm', 'a');
		cy.cGet('#search').should('not.have.attr', 'disabled');
		cy.cGet('#search').click();
		// Part of the text should be selected
		helper.textSelectionShouldExist();
		helper.expectTextForClipboard('a');
		cy.cGet('#copy-paste-container p b').should('not.exist');
	});

	it('Check word counts.', function() {
		writerHelper.selectAllTextOfDoc();
		mobileHelper.selectHamburgerMenuItem(['Word Count...']);
		// Selected counts
		cy.cGet('#selectwords').should('have.text', '106');
		cy.cGet('#selectchars').should('have.text', '1,174');
		cy.cGet('#selectcharsnospaces').should('have.text', '1,069');
		cy.cGet('#selectcjkchars').should('have.text', '0');
		// General counts
		cy.cGet('#docwords').should('have.text', '106');
		cy.cGet('#docchars').should('have.text', '1,174');
		cy.cGet('#doccharsnospaces').should('have.text', '1,069');
		cy.cGet('#doccjkchars').should('have.text', '0');
	});

	it('Page setup: change paper size.', function() {
		// For now, we don't/can't actually check if the size of the document is updated or not.
		// That can be checked with a unit test.
		openPageWizard();
		cy.cGet('#papersize').click();
		cy.cGet('#papersize').contains('.mobile-wizard.ui-combobox-text', 'A3').click();
		closePageWizard();
		// Check that the page wizard shows the right value after reopen.
		openPageWizard();
		cy.cGet('#papersize .ui-header-left').should('have.text', 'A3');
	});

	it('Page setup: change paper width.', function() {
		// For now, we don't/can't actually check if the size of the document is updated or not.
		// That can be checked with a unit test.
		openPageWizard();
		helper.typeIntoInputField('#paperwidth .spinfield', '12');
		closePageWizard();
		// Check that the page wizard shows the right value after reopen.
		openPageWizard();
		cy.cGet('#papersize .ui-header-left').should('have.text', 'User');
		cy.cGet('#paperwidth .spinfield').should('have.value', '12');
	});

	it('Page setup: change paper height.', function() {
		// For now, we don't/can't actually check if the size of the document is updated or not.
		// That can be checked with a unit test.
		openPageWizard();
		helper.typeIntoInputField('#paperheight .spinfield', '3.0');
		closePageWizard();
		// Check that the page wizard shows the right value after reopen.
		openPageWizard();
		cy.cGet('#papersize .ui-header-left').should('have.text', 'User');
		cy.cGet('#paperheight .spinfield').should('have.value', '3');
	});

	it('Page setup: change orientation.', function() {
		// For now, we don't/can't actually check if the size of the document is updated or not.
		// That can be checked with a unit test.
		openPageWizard();
		cy.cGet('#paperorientation').click();
		cy.cGet('#paperorientation').contains('.mobile-wizard.ui-combobox-text', 'Landscape').click();
		closePageWizard();
		// Check that the page wizard shows the right value after reopen.
		openPageWizard();
		cy.cGet('#paperorientation .ui-header-left').should('have.text', 'Landscape');
	});

	it('Page setup: change margin.', function() {
		// We use the cursor horizontal position as indicator of margin change.
		helper.typeIntoDocument('{home}');
		cy.cGet('.blinking-cursor').should('be.visible');
		helper.getCursorPos('left', 'cursorOrigLeft');
		openPageWizard();
		cy.cGet('#marginLB').click();
		cy.cGet('#marginLB').contains('.mobile-wizard.ui-combobox-text', 'None').click();
		closePageWizard();

		// Text is moved leftward by margin removal.
		cy.get('@cursorOrigLeft')
			.then(function(cursorOrigLeft) {
				cy.cGet('.blinking-cursor')
					.should(function(cursor) {
						expect(cursor.offset().left).to.be.lessThan(cursorOrigLeft);
					});
			});

		// Check that the page wizard shows the right value after reopen.
		openPageWizard();
		cy.cGet('#marginLB .ui-header-left').should('have.text', 'None');
	});

	it('Show formatting marks.', function() {
		// Hide text so the document is full white.
		hideText();
		helper.isCanvasWhite(true);
		// Enable it first -> spaces will be visible.
		mobileHelper.selectHamburgerMenuItem(['View', 'Formatting Marks']);
		helper.isCanvasWhite(false);
		// Then disable it again.
		mobileHelper.selectHamburgerMenuItem(['View', 'Formatting Marks']);
		helper.typeIntoDocument('{home}{end}');
		helper.isCanvasWhite(true);
	});

	it('Automatic spell checking.', function() {
		// Hide text so the document is full white.
		hideText();
		helper.isCanvasWhite(true);
		// Enable it first.
		mobileHelper.selectHamburgerMenuItem(['View', 'Automatic Spell Checking']);
		helper.isCanvasWhite(false);
		// Then disable it again.
		mobileHelper.selectHamburgerMenuItem(['View', 'Automatic Spell Checking']);
		helper.typeIntoDocument('{home}{end}');
		helper.isCanvasWhite(true);
	});

	it('Check version information.', function() {
		mobileHelper.selectHamburgerMenuItem(['About']);
		cy.cGet('#mobile-wizard-content').should('exist');
		// Check the version
		cy.cGet('body').find('#coolwsd-version').should('exist');
		// Close about dialog
		cy.cGet('div.mobile-wizard.jsdialog-overlay.cancellable').click({force : true});
	});
});
