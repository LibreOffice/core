/* global describe it cy beforeEach expect require */

var helper = require('../../common/helper');
var desktopHelper = require('../../common/desktop_helper');

describe(['tagdesktop'], 'Context toolbar tests.', function() {

	beforeEach(function() {
		cy.viewport(1920, 1080);
		helper.setupAndLoadDocument('writer/top_toolbar.odt');
		desktopHelper.switchUIToNotebookbar();
	});

	it('Font name combobox has previews.', function() {
		// Double-click on text to select a word and trigger context toolbar
		helper.getBlinkingCursorPosition('cursorPos');
		helper.clickAt('cursorPos', true);

		// Context toolbar should appear
		cy.cGet('#context-toolbar').should('not.have.class', 'hidden');

		// Open font name combobox in context toolbar
		cy.cGet('#context-toolbar #fontnamecombobox .ui-combobox-button').click();

		// Dropdown should appear with font preview images
		cy.cGet('[id$="-dropdown"].modalpopup').should('be.visible');
		cy.cGet('[id$="-dropdown"].modalpopup img').should('have.length.greaterThan', 0);
	});

	it('View mode offers Switch to Editing.', function() {
		// Sample the position while there is still a cursor to sample.
		helper.getBlinkingCursorPosition('cursorPos');

		cy.getFrameWindow().its('app').then(function(app) {
			app.map.setPermission('readonly');
		});
		cy.cGet('#viewModeDropdownButton-button').should('have.text', 'Viewing');

		helper.clickAt('cursorPos', true);

		cy.cGet('#context-toolbar').should('not.have.class', 'hidden');
		cy.cGet('#context-toolbar #context-switchtoedit').should('be.visible');
		cy.cGet('#context-toolbar #fontnamecombobox').should('not.exist');

		// The advertised key has to stay in step with the binding.
		cy.cGet('#context-toolbar #context-switchtoedit')
			.should('have.attr', 'data-cooltip')
			.and('contain', 'Ctrl+Shift+E');

		cy.cGet('#context-toolbar #context-switchtoedit').click();
		cy.cGet('#viewModeDropdownButton-button').should('have.text', 'Editing');

		// The toolbar builds its item list once, so the formatting set has to
		// come back after the mode change.
		helper.getBlinkingCursorPosition('editCursorPos');
		helper.clickAt('editCursorPos', true);

		cy.cGet('#context-toolbar').should('not.have.class', 'hidden');
		cy.cGet('#context-toolbar #fontnamecombobox').should('be.visible');
		cy.cGet('#context-toolbar #context-switchtoedit').should('not.exist');
	});

	it('Ctrl+Shift+E switches in view mode and is inert in edit mode.', function() {
		helper.getBlinkingCursorPosition('cursorPos');

		// Edit mode: the descriptor is ViewType.ReadOnly, so it must not fire.
		cy.getFrameWindow().then(function(win) {
			cy.spy(win.app.dispatcher, 'dispatch').as('dispatch');
		});
		helper.typeIntoDocument('{ctrl}{shift}E');
		cy.get('@dispatch').should('not.be.calledWith', 'switchtoedit');

		cy.getFrameWindow().its('app').then(function(app) {
			app.map.setPermission('readonly');
		});
		helper.typeIntoDocument('{ctrl}{shift}E');
		cy.cGet('#viewModeDropdownButton-button').should('have.text', 'Editing');
	});

	it('View mode with no write access shows nothing.', function() {
		helper.getBlinkingCursorPosition('cursorPos');

		cy.getFrameWindow().then(function(win) {
			win.app.map.setPermission('readonly');
			// The only way from a cypress session to reach a document that was
			// read-only at open.
			win.app.file.readOnly = true;
		});

		helper.clickAt('cursorPos', true);

		cy.cGet('#context-toolbar').should('have.class', 'hidden');
		cy.cGet('#context-toolbar #context-switchtoedit').should('not.exist');
	});
});

describe(['tagdesktop'], 'Context toolbar tooltip tests.', function() {

	beforeEach(function() {
		cy.viewport(1920, 1080);
		helper.setupAndLoadDocument('writer/top_toolbar.odt', false, false, undefined,
			'tooltips=true');
		desktopHelper.switchUIToNotebookbar();
	});

	it('A tooltip over another button goes away when that button is clicked.', function() {
		var missingFont = 'No Such Font Name';
		var warning = 'The current font is not available and will be substituted.';

		helper.getBlinkingCursorPosition('cursorPos');
		helper.clickAt('cursorPos', true);

		// The font name box has a tooltip only while the font of the text is one the server
		// does not have.
		cy.cGet('#Home-container #fontnamecombobox .ui-combobox-content')
			.type('{selectall}{backspace}' + missingFont + '{enter}');
		helper.waitForMapState('.uno:CharFontName', missingFont);

		helper.clickAt('cursorPos', true);
		cy.cGet('#context-toolbar').should('not.have.class', 'hidden');
		cy.cGet('#context-toolbar #fontnamecombobox').should('have.attr', 'data-cooltip', warning);

		// The tooltip of the font name box is drawn over the Bold button.
		cy.cGet('#context-toolbar #fontnamecombobox').realHover();
		cy.cGet('#cooltip').should('be.visible').and('have.text', warning);

		cy.cGet('#context-toolbar [id^="home-bold"]').then(function(bold) {
			cy.cGet('#cooltip').then(function(tooltip) {
				var boldRect = bold[0].getBoundingClientRect();
				var tooltipRect = tooltip[0].getBoundingClientRect();
				var left = Math.max(boldRect.left, tooltipRect.left);
				var right = Math.min(boldRect.right, tooltipRect.right);
				var top = Math.max(boldRect.top, tooltipRect.top);
				var bottom = Math.min(boldRect.bottom, tooltipRect.bottom);
				expect(right, 'tooltip covers Bold horizontally').to.be.greaterThan(left);
				expect(bottom, 'tooltip covers Bold vertically').to.be.greaterThan(top);
				cy.wrap({ x: (left + right) / 2, y: (top + bottom) / 2 }).as('overlap');
			});
		});

		// On the tooltip the pointer is reading it, so it stays although Bold is under it.
		cy.get('@overlap').then(function(point) {
			cy.cGet('body').realMouseMove(point.x, point.y);
		});
		cy.cGet('#cooltip').should('be.visible').and('have.text', warning);

		// The click goes through the tooltip to Bold, and the tooltip goes away.
		cy.get('@overlap').then(function(point) {
			cy.cGet('body').realClick({ x: point.x, y: point.y });
		});
		cy.cGet('#cooltip').should('not.be.visible');
		helper.waitForMapState('.uno:Bold', 'true');
	});
});
