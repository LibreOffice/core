/* global describe it cy beforeEach require expect */

var helper = require('../../common/helper');

describe(['tagdesktop'], 'Options view toggles persist across reload', function () {
	var newFilePath;
	var win;

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
});
