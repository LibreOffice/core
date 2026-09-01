/* global describe it cy beforeEach require expect */

var helper = require('../../common/helper');
var desktopHelper = require('../../common/desktop_helper');

describe(['tagdesktop'], 'Options view toggles persist across reload', function () {
	var newFilePath;
	var win;

	function settingsIframeBody() {
		return cy.cframe()
			.find('.iframe-settings-modal')
			.its('0.contentDocument').should('exist')
			.its('body').should('not.be.empty')
			.then(cy.wrap);
	}

	beforeEach(function () {
		newFilePath = helper.setupAndLoadDocument('writer/annotation.odt');
		cy.getFrameWindow().then(function (w) {
			win = w;
		});
		cy.cGet('div.leaflet-layer').should('exist');
	});

	it('remembers Show Formatting Marks after reload', function () {
		// Marks start off. Turn them on through the same command the menu uses.
		cy.then(function () {
			win.app.map.sendUnoCommand('.uno:ControlCodes');
		});
		// Core confirms marks are on, which is what gets persisted.
		cy.getFrameWindow().then(function (w) {
			cy.wrap(w.app.map['stateChangeHandler'])
				.invoke('getItemValue', '.uno:ControlCodes')
				.should('eq', 'true');
		});
		cy.getFrameWindow()
			.its('prefs')
			.invoke('getBoolean', 'text.ShowFormattingMarks', false)
			.should('eq', true);

		helper.reloadDocument(newFilePath);
		cy.cGet('div.leaflet-layer').should('exist');

		// The marks are actually shown again, not just the stored flag. This
		// is the live state reported by core, which the early-toggle bug left
		// off even though the preference said on.
		cy.getFrameWindow().then(function (w) {
			cy.wrap(w.app.map['stateChangeHandler'])
				.invoke('getItemValue', '.uno:ControlCodes')
				.should('eq', 'true');
		});
		cy.getFrameWindow()
			.its('prefs')
			.invoke('getBoolean', 'text.ShowFormattingMarks', false)
			.should('eq', true);
	});

	it('remembers hidden comments after reload', function () {
		cy.then(function () {
			win.app.map.showComments(false);
		});
		cy.getFrameWindow()
			.its('prefs')
			.invoke('getBoolean', 'text.ShowAnnotations', true)
			.should('eq', false);

		helper.reloadDocument(newFilePath);
		cy.cGet('div.leaflet-layer').should('exist');

		// Comments stay hidden, and the in-document state reflects it.
		cy.getFrameWindow()
			.its('prefs')
			.invoke('getBoolean', 'text.ShowAnnotations', true)
			.should('eq', false);
		cy.getFrameWindow().then(function (w) {
			var state = w.app.map['stateChangeHandler'].getItemValue('showannotations');
			expect(state).to.eq('false');
		});
	});

	it('hides the comments as soon as the Options dialog is saved', function () {
		desktopHelper.switchUIToNotebookbar();
		desktopHelper.insertComment();
		cy.cGet('#comment-container-1').should('exist');

		// The comments start out shown, so switching the option off below is a
		// real change.
		cy.getFrameWindow().then(function (w) {
			var state = w.app.map['stateChangeHandler'].getItemValue('showannotations');
			expect(state).to.eq('true');
		});

		cy.then(function () {
			win.app.map.settings.showSettingsDialog('browser-setting');
		});
		cy.cGet('.iframe-settings-wrap').should('be.visible');

		// The Interface Settings open on the Calc tab; wait for that, otherwise
		// the tab it opens with replaces the Writer options picked below.
		settingsIframeBody()
			.find('#bs-tab-spreadsheet.active', { timeout: 20000 })
			.should('exist');
		settingsIframeBody().find('#bs-tab-text').click();

		// The comment toggle sits on the Writer tab, on because the comments are
		// shown.
		settingsIframeBody()
			.find('#text-ShowAnnotations-input')
			.should('be.checked')
			.click({ force: true });

		cy.cGet('#iframe-settings-save').click();

		// The comment goes away with the save, no reload needed, and the choice is
		// recorded the same way the Show Comments button records it.
		cy.cGet('#comment-container-1').should('be.not.visible');
		cy.getFrameWindow()
			.its('prefs')
			.invoke('getBoolean', 'text.ShowAnnotations', true)
			.should('eq', false);
	});
});
