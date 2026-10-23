/* global describe it cy beforeEach require expect */

var helper = require('../../common/helper');
var desktopHelper = require('../../common/desktop_helper');
var aichatHelper = require('../../common/aichat_helper');

describe(['tagdesktop'], 'AI Chat Sidebar', { testIsolation: false }, function() {

	desktopHelper.shareDocumentAcrossTests('writer/help_dialog.odt');

	beforeEach(function() {
		cy.getFrameWindow().then((win) => {
			this.win = win;
			aichatHelper.resetAIChat(win);
		});
	});

	describe('Tone picker - short panel scrolls', function() {
		it('Tone form Save and Cancel stay reachable when the panel is short', function() {
			// A short viewport reproduces the same geometry as high browser
			// zoom: the form is taller than the space left in the panel.
			cy.viewport(1280, 400);
			cy.clearLocalStorage();
			aichatHelper.enableAIAndStubSocket(this.win, {});
			aichatHelper.openAIChat();
			aichatHelper.openTonePicker();
			cy.cGet('#aichat-tone-add button').click();
			helper.waitUntilLayoutingIsIdle(this.win);
			cy.cGet('#aichat-tone-form').should('exist');
			// The picker is the scroll container for the form content.
			cy.cGet('#aichat-tone-picker').should(($el) => {
				expect($el[0].scrollHeight).to.be.greaterThan($el[0].clientHeight);
				expect(getComputedStyle($el[0]).overflowY).to.equal('auto');
			});
			cy.cGet('#aichat-tone-form-save button').scrollIntoView().should('be.visible');
			cy.cGet('#aichat-tone-form-cancel button').scrollIntoView().click();
			helper.waitUntilLayoutingIsIdle(this.win);
			cy.cGet('#aichat-tone-form').should('not.exist');
			aichatHelper.closeTonePicker();
		});

		it('Tone picker keeps a usable height in a very short panel', function() {
			// 270px is about what a 1080p screen leaves at 400% browser zoom.
			// The picker takes part in flex shrinking, so without a floor it
			// collapsed to its padding and the tones were unreadable.
			cy.viewport(1280, 270);
			cy.clearLocalStorage();
			aichatHelper.enableAIAndStubSocket(this.win, {});
			aichatHelper.openAIChat();
			aichatHelper.openTonePicker();
			helper.waitUntilLayoutingIsIdle(this.win);
			cy.cGet('#aichat-tone-picker').should(($el) => {
				expect($el[0].getBoundingClientRect().height).to.be.at.least(170);
			});
			// The panel is what scrolls once the picker stops shrinking.
			cy.cGet('#aichat-main').should(($el) => {
				expect($el[0].scrollHeight).to.be.greaterThan($el[0].clientHeight);
			});
			cy.cGet('#aichat-tone-preset-natural').scrollIntoView().should('be.visible');
			aichatHelper.closeTonePicker();
		});

		it('Emoji overlay fits inside a short viewport', function() {
			cy.viewport(1280, 400);
			cy.clearLocalStorage();
			aichatHelper.enableAIAndStubSocket(this.win, {});
			aichatHelper.openAIChat();
			aichatHelper.openTonePicker();
			cy.cGet('#aichat-tone-add button').click();
			helper.waitUntilLayoutingIsIdle(this.win);
			// The + icon button opens the emoji overlay.
			cy.cGet('#aichat-tone-form-emoji-more button').click();
			helper.waitUntilLayoutingIsIdle(this.win);
			// 45vh of a 400px viewport is 180px, below the 320px fixed cap.
			cy.cGet('#aichat-tone-form-emoji-picker').should(($el) => {
				expect($el[0].getBoundingClientRect().height).to.be.at.most(181);
			});
		});
	});
});
