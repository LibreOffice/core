/* global describe it cy beforeEach expect require */

var helper = require('../../common/helper');
var impressHelper = require('../../common/impress_helper');
var mobileHelper = require('../../common/mobile_helper');

describe(['tagmobile', 'tagproxy'], 'Focus tests', function() {

	beforeEach(function() {
		helper.setupAndLoadDocument('writer/focus.odt');
		cy.getFrameWindow().then((win) => {
			this.win = win;
		});
	});

	it('Basic document focus.', function() {
		// Click on edit button
		mobileHelper.enableEditingMobile();
		// Click in the document
		cy.cGet('#document-canvas').click();
		helper.assertFocus('className', 'clipboard');
	});

	it('Focus with opened mobile wizard.', function() {
		// Click on edit button
		mobileHelper.enableEditingMobile();
		// Click in the document
		cy.cGet('#document-container').click();
		// Clipboard has the focus -> can type in the document
		helper.assertFocus('className', 'clipboard');
		mobileHelper.openMobileWizard();
		// Body should have the focus (no focus on document)
		helper.assertFocus('tagName', 'BODY');
		mobileHelper.closeMobileWizard();
	});

	it('Keyboard comes back when the mobile wizard closes.', function() {
		mobileHelper.enableEditingMobile();
		cy.cGet('#document-container').click();
		helper.assertHaveKeyboardInput();
		mobileHelper.openMobileWizard();
		helper.assertNoKeyboardInput();
		mobileHelper.closeMobileWizard();
		helper.assertHaveKeyboardInput();
	});

	it('Keyboard stays down while one wizard replaces another.', function() {
		mobileHelper.enableEditingMobile();
		cy.cGet('#document-container').click();
		helper.assertHaveKeyboardInput();
		mobileHelper.openInsertionWizard();
		helper.assertNoKeyboardInput();
		cy.cGet('#toolbar-up #mobile_wizard').click();
		// the sidebar arrives later, no keyboard in between
		cy.cGet('div.clipboard').then(function(clipboard) {
			expect(clipboard.attr('data-accept-input')).to.equal('false');
		});
		cy.cGet('#toolbar-up #mobile_wizard').should('have.class', 'selected');
		cy.cGet('#mobile-wizard-content').should('not.be.empty');
		helper.assertNoKeyboardInput();
		mobileHelper.closeMobileWizard();
		helper.assertHaveKeyboardInput();
	});

	it('Tapping the document closes the mobile wizard and keeps the cursor.', function() {
		mobileHelper.enableEditingMobile();
		cy.cGet('#document-container').click();
		helper.assertHaveKeyboardInput();
		// Typed text moves the cursor away from the start of the paragraph, where a tap on the
		// top left of the page would put it.
		helper.typeIntoDocument('abc');
		cy.getFrameWindow().then((win) => {
			return helper.processToIdle(win).then(() => {
				this.cursorBefore = [win.app.file.textCursor.rectangle.x1, win.app.file.textCursor.rectangle.y1];
			});
		});
		mobileHelper.openMobileWizard();
		helper.assertNoKeyboardInput();
		// A tap on the part of the document above the wizard.
		cy.cGet('#document-container').click(20, 20);
		cy.cGet('#mobile-wizard').should('not.be.visible');
		cy.cGet('#toolbar-up #mobile_wizard').should('not.have.class', 'selected');
		// The tap only put the wizard away: the cursor stays and the keyboard is back.
		helper.assertHaveKeyboardInput();
		cy.getFrameWindow().then((win) => {
			return helper.processToIdle(win).then(() => {
				expect([win.app.file.textCursor.rectangle.x1, win.app.file.textCursor.rectangle.y1])
					.to.deep.equal(this.cursorBefore);
			});
		});
	});

	it('Focus inside mobile wizard.', function() {
		// Click on edit button
		mobileHelper.enableEditingMobile();
		mobileHelper.openMobileWizard();
		// Open paragraph properties
		cy.cGet('#Paragraph').click();
		cy.cGet('#aboveparaspacing .spinfield').should('have.value', '0');
		cy.cGet('#aboveparaspacing .spinfield').click();
		// The spinfield should have the focus now.
		helper.assertFocus('className', 'spinfield');
		mobileHelper.closeMobileWizard();
	});

	it('Focus after insertion.', function() {
		// Click on edit button
		mobileHelper.enableEditingMobile();
		mobileHelper.openInsertionWizard();
		// Select More Fields
		cy.cGet('body').contains('.ui-header.level-0.mobile-wizard.ui-widget', 'More Fields...').click();
		// Insert a field
		cy.cGet('body').contains('.menu-entry-with-icon', 'Page Number').click();
		cy.cGet('#mobile-wizard').should('not.be.visible');
		// After insertion the document gets the focus
		helper.assertFocus('className', 'clipboard');
	});

	it('Shape related focus.', function() {
		// Click on edit button
		mobileHelper.enableEditingMobile();
		mobileHelper.openInsertionWizard();
		// Do insertion
		cy.cGet('body').contains('.menu-entry-with-icon', 'Shape').click();
		cy.cGet('.col.w2ui-icon.basicshapes_rectangle').click();
		// Check that the shape is there
		cy.cGet('#canvas-container > svg').should('exist');
		helper.processToIdle(this.win);
		// One tap on the shape
		cy.cGet('#canvas-container > svg > svg').should('exist');
		helper.getShapeSVGCenter().then(function(pos) {
			cy.cGet('#document-canvas').click(pos.x, pos.y);
		});

		helper.processToIdle(this.win);
		// No focus on the document
		helper.assertFocus('tagName', 'BODY');

		// Double tap on the shape to enter text editing
		impressHelper.dblclickOnSelectedShape();
		helper.processToIdle(this.win);
		cy.cGet('.blinking-cursor').should('be.visible');
		// Document still has the focus
		helper.assertFocus('className', 'clipboard');
		helper.assertHaveKeyboardInput();
	});

	it('Focus with hamburger menu.', function() {
		// Click on edit button
		mobileHelper.enableEditingMobile();
		// Click in the document
		cy.cGet('#document-container').click();
		// Clipboard has the focus -> can type in the document
		helper.assertFocus('className', 'clipboard');
		// Open hamburger menu
		mobileHelper.openHamburgerMenu();
		// Close hamburger menu
		mobileHelper.closeHamburgerMenu();
	});

	it('Focus after applying font change.', function() {
		// Click on edit button
		mobileHelper.enableEditingMobile();
		// Click in the document
		cy.cGet('#document-container').click();
		// Clipboard has the focus -> can type in the document
		helper.assertFocus('className', 'clipboard');
		mobileHelper.openMobileWizard();
		// No focus
		helper.assertFocus('tagName', 'BODY');
		// Apply bold
		cy.cGet('#mobile-wizard .unoBold').click();
		cy.cGet('#mobile-wizard .unoBold').should('have.class', 'selected');
		// No focus
		helper.assertFocus('tagName', 'BODY');
		mobileHelper.closeMobileWizard();
	});

	it('Apply bold, check keyboard.', function() {
		// Click on edit button
		mobileHelper.enableEditingMobile();
		// Grab focus to the document
		helper.typeIntoDocument('x');
		helper.selectAllText();
		cy.cGet('#toolbar-down .unoBold').should('not.have.class', 'selected');
		helper.assertHaveKeyboardInput();
		cy.cGet('#toolbar-down .unoBold').click();
		cy.cGet('#toolbar-down .unoBold').should('have.class', 'selected');
		helper.assertHaveKeyboardInput();
	});

	it('Apply italic, check keyboard.', function() {
		// Click on edit button
		mobileHelper.enableEditingMobile();
		// Grab focus to the document
		helper.typeIntoDocument('x');
		helper.selectAllText();
		cy.cGet('#toolbar-down .unoItalic').should('not.have.class', 'selected');
		helper.assertHaveKeyboardInput();
		cy.cGet('#toolbar-down .unoItalic').click();
		cy.cGet('#toolbar-down .unoItalic').should('have.class', 'selected');
		helper.assertHaveKeyboardInput();
	});

	it('Apply underline, check keyboard.', function() {
		// Click on edit button
		mobileHelper.enableEditingMobile();
		// Grab focus to the document
		helper.typeIntoDocument('x');
		helper.selectAllText();
		cy.cGet('#toolbar-down .unoUnderline').should('not.have.class', 'selected');
		helper.assertHaveKeyboardInput();
		cy.cGet('#toolbar-down .unoUnderline').click();
		cy.cGet('#toolbar-down .unoUnderline').should('have.class', 'selected');
		helper.assertHaveKeyboardInput();
	});
});
