/* global describe it cy beforeEach require */

const helper = require('../../common/helper');

// Alone with a document running on your own settings, a change is read again
// as soon as it is saved, so there is nothing to warn about. The note is in
// the page either way and carries its own hidden flag, which is what the
// dialog sets.
describe(['tagdesktop'], 'A user alone gets no settings warning', function() {
	function settingsIframeBody() {
		return cy.cframe()
			.find('.iframe-settings-modal')
			.its('0.contentDocument').should('exist')
			.its('body').should('not.be.empty')
			.then(cy.wrap);
	}

	beforeEach(function() {
		helper.setupAndLoadDocument('writer/presets.odt');
	});

	it('The note is there but says nothing to the only user', function() {
		cy.getFrameWindow().then(function(win) {
			win.app.map.settings.showSettingsDialog('xcu-section');
		});
		cy.cGet('.iframe-settings-wrap').should('be.visible');
		settingsIframeBody().find('#xcu-editor', { timeout: 30000 }).should('exist');
		settingsIframeBody()
			.find('.settings-scope-note')
			.should('have.length.at.least', 1)
			.first()
			.should('have.prop', 'hidden', true);
	});
});
