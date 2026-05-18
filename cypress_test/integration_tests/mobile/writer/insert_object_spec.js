/* global describe it cy beforeEach require expect */

var helper = require('../../common/helper');
var mobileHelper = require('../../common/mobile_helper');
var writerHelper = require('../../common/writer_helper');

describe(['tagmobile', 'tagnextcloud'], 'Insert objects via insertion wizard.', function() {

	beforeEach(function() {
		helper.setupAndLoadDocument('writer/insert_object.odt');
		// Click on edit button
		mobileHelper.enableEditingMobile();
		cy.getFrameWindow().then((win) => {
			this.win = win;
		});
	});

	it('Insert local image.', function() {
		mobileHelper.openInsertionWizard();
		// We can't use the menu item directly, because it would open file picker.
		cy.cGet('body').contains('.menu-entry-with-icon', 'Local Image...').should('be.visible');
		cy.cGet('#insertgraphic[type=file]').attachFile('/mobile/writer/image_to_insert.png');
		cy.cGet('#document-container svg g.Graphic').should('exist');
	});

	it('Insert comment.', function() {
		mobileHelper.openInsertionWizard();
		cy.cGet('body').contains('.menu-entry-with-icon', 'Comment').click();
		// Comment insertion dialog is opened
		cy.cGet('.cool-annotation-table').should('exist');
		// Add some comment
		cy.cGet('#input-modal-input').type('some text');
		cy.cGet('#response-ok').click();
		cy.cGet('#comment-container-1').should('exist');
		cy.cGet('#annotation-content-area-1').should('have.text', 'some text');
	});

	it('Insert default table.', function() {
		helper.setDummyClipboardForCopy();
		mobileHelper.openInsertionWizard();
		// Open Table submenu
		cy.cGet('body').contains('.ui-header.level-0.mobile-wizard.ui-widget', 'Table').click();
		cy.cGet('.mobile-wizard.ui-text').should('be.visible');
		// Push insert table button
		cy.cGet('.inserttablecontrols button').should('be.visible').click();
		// Table is inserted with the markers shown
		cy.cGet('.table-column-resize-marker').should('exist');
		// The marker can render before the table cells are in the document
		// model, so a bare ctrl+a/copy sequence selects nothing.
		helper.processToIdle(this.win);
		helper.typeIntoDocument('{ctrl}a');
		helper.copy();
		// Two rows
		cy.cGet('#copy-paste-container tr').should('have.length', 2);
		// Four cells
		cy.cGet('#copy-paste-container td').should('have.length', 4);
	});

	it('Insert custom table.', function() {
		helper.setDummyClipboardForCopy();
		mobileHelper.openInsertionWizard();
		// Open Table submenu
		cy.cGet('body').contains('.ui-header.level-0.mobile-wizard.ui-widget', 'Table').click();
		cy.cGet('.mobile-wizard.ui-text').should('be.visible');
		// Change rows and columns
		cy.cGet('.inserttablecontrols #rows .spinfieldcontrols .plus').click();
		cy.cGet('.inserttablecontrols #cols .spinfieldcontrols .plus').click();
		// Push insert table button
		cy.cGet('.inserttablecontrols button').should('be.visible').click();
		// Table is inserted with the markers shown
		cy.cGet('.table-column-resize-marker').should('exist');
		// Same insert-then-select race as 'Insert default table'.
		helper.processToIdle(this.win);
		helper.typeIntoDocument('{ctrl}a');
		helper.copy();
		// Three rows
		cy.cGet('#copy-paste-container tr').should('have.length', 3);
		// Nine cells
		cy.cGet('#copy-paste-container td').should('have.length', 9);
	});

	it('Insert header.', function() {
		// Get the blinking cursor pos
		helper.typeIntoDocument('xx{enter}');
		helper.typeText('body', 'xxxx', 500);
		helper.getCursorPos('left', 'cursorOrigLeft');
		mobileHelper.openInsertionWizard();
		// Open header/footer submenu
		cy.cGet('body').contains('.menu-entry-with-icon', 'Header and Footer').click();
		cy.cGet('.ui-header.level-1.mobile-wizard.ui-widget').should('be.visible');
		// Open header submenu
		cy.cGet('body').contains('.ui-header.level-1.mobile-wizard.ui-widget', 'Header').click();
		// Insert header for All
		cy.cGet('body').contains('.menu-entry-no-icon', 'All').click();
		cy.get('@cursorOrigLeft')
			.then(function(cursorOrigLeft) {
				cy.cGet('.blinking-cursor')
					.should(function(cursor) {
						expect(cursor.offset().left).to.be.lessThan(cursorOrigLeft);
					});
			});
	});

	it('Insert footer.', function() {
		// Get the blinking cursor pos
		helper.typeIntoDocument('xxxx');
		helper.getCursorPos('top', 'cursorOrigTop');
		mobileHelper.openInsertionWizard();
		// Open header/footer submenu
		cy.cGet('body').contains('.menu-entry-with-icon', 'Header and Footer').click();
		cy.cGet('.ui-header.level-1.mobile-wizard.ui-widget').should('be.visible');
		// Open footer submenu
		cy.cGet('body').contains('.ui-header.level-1.mobile-wizard.ui-widget', 'Footer').click();
		// Insert footer for All
		cy.cGet('body').contains('.ui-content.level-1.mobile-wizard[title~="Footer"] .ui-header.level-2.mobile-wizard.ui-widget .menu-entry-no-icon', 'All').click();
		helper.processToIdle(this.win);
		// Check that the cursor was moved
		cy.get('@cursorOrigTop')
			.then(function(cursorOrigTop) {
				cy.cGet('.blinking-cursor')
					.should(function(cursor) {
						expect(cursor.offset().top).to.be.greaterThan(cursorOrigTop);
					});
			});
	});

	it('Insert footnote.', function() {
		// Get the blinking cursor pos
		helper.typeIntoDocument('xxxx');
		helper.getCursorPos('top', 'cursorOrigTop');
		mobileHelper.openInsertionWizard();
		// Insert footnote
		cy.cGet('body').contains('.menu-entry-with-icon', 'Footnote').click();
		// Check that the cursor was moved
		cy.get('@cursorOrigTop')
			.then(function(cursorOrigTop) {
				cy.cGet('.blinking-cursor')
					.should(function(cursor) {
						expect(cursor.offset().top).to.be.greaterThan(cursorOrigTop);
					});
			});
	});

	it('Insert endnote.', function() {
		// Get the blinking cursor pos
		helper.typeIntoDocument('xxxx');
		helper.getCursorPos('top', 'cursorOrigTop');
		mobileHelper.openInsertionWizard();
		// Insert endnote
		cy.cGet('body').contains('.menu-entry-with-icon', 'Endnote').click();
		// Check that the cursor was moved
		cy.get('@cursorOrigTop')
			.then(function(cursorOrigTop) {
				cy.cGet('.blinking-cursor')
					.should(function(cursor) {
						expect(cursor.offset().top).to.be.greaterThan(cursorOrigTop);
					});
			});
	});

	it('Insert page break.', function() {
		// Get the blinking cursor pos
		helper.typeIntoDocument('xxxx');
		helper.getCursorPos('top', 'cursorOrigTop');
		mobileHelper.openInsertionWizard();
		// Insert page break
		cy.cGet('body').contains('.menu-entry-with-icon', 'Page Break').click();
		// Check that the cursor was moved
		cy.get('@cursorOrigTop')
			.then(function(cursorOrigTop) {
				cy.cGet('.blinking-cursor')
					.should(function(cursor) {
						expect(cursor.offset().top).to.be.greaterThan(cursorOrigTop);
					});
			});
	});

	it('Insert column break.', function() {
		// Get the blinking cursor pos
		helper.typeIntoDocument('xxxx');
		helper.getCursorPos('top', 'cursorOrigTop');
		mobileHelper.openInsertionWizard();
		// Do insertion
		cy.cGet('body').contains('.menu-entry-with-icon', 'Column Break').click();
		// Check that the cursor was moved
		cy.get('@cursorOrigTop')
			.then(function(cursorOrigTop) {
				cy.cGet('.blinking-cursor')
					.should(function(cursor) {
						expect(cursor.offset().top).to.be.greaterThan(cursorOrigTop);
					});
			});
	});

	it('Insert hyperlink.', function() {
		helper.setDummyClipboardForCopy();
		mobileHelper.openInsertionWizard();
		// Open hyperlink dialog
		cy.cGet('body').contains('.menu-entry-with-icon', 'Hyperlink...').click();
		// Dialog is opened
		cy.cGet('#target-input').should('exist');
		// Type text and link
		cy.cGet('#indication-input').type('some text');
		cy.cGet('#target-input').type('www.something.com');
		// Insert
		cy.cGet('#ok').click();
		writerHelper.selectAllTextOfDoc();
		helper.copy();
		helper.expectTextForClipboard('some text');
		cy.cGet('#copy-paste-container p a').should('have.attr', 'href', 'http://www.something.com/');
	});

	it('Open inserted hyperlink.', function() {
		mobileHelper.openInsertionWizard();
		// Open hyperlink dialog
		cy.cGet('body').contains('.menu-entry-with-icon', 'Hyperlink...').click();
		// Dialog is opened
		cy.cGet('#target-input').should('exist');
		// Type text and link
		cy.cGet('#indication-input').type('some text');
		cy.cGet('#target-input').type('www.something.com');
		// Insert
		cy.cGet('#ok').click();
		helper.typeIntoDocument('{leftArrow}');
		cy.cGet('#hyperlink-pop-up').click();
		cy.cGet('#info-modal-label2').should('have.text', 'http://www.something.com/');
	});

	it('Insert shape.', function() {
		mobileHelper.openInsertionWizard();
		// Do insertion
		cy.cGet('body').contains('.menu-entry-with-icon', 'Shape').click();
		cy.cGet('.col.w2ui-icon.basicshapes_rectangle').click();
		// Check that the shape is there
		cy.cGet('#document-container svg g').should('exist');
		cy.cGet('#canvas-container > svg > svg').then(function(svg) {
			expect(parseInt(svg[0].style.width.replace('px', ''))).to.be.greaterThan(0);
			expect(parseInt(svg[0].style.height.replace('px', ''))).to.be.greaterThan(0);
		});
	});
});
