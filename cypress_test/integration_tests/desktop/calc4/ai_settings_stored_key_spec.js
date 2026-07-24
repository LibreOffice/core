/* global describe it cy beforeEach afterEach require expect */

var helper = require('../../common/helper');

// On reopen the saved AI key is redacted, so the dialog must list models using
// the key the server still holds instead of leaving the Model dropdown disabled.
describe(['tagdesktop'], 'AI settings reuse the saved provider key to list models', function() {

	function settingsIframeBody() {
		return cy.cframe()
			.find('.iframe-settings-modal')
			.its('0.contentDocument').should('exist')
			.its('body').should('not.be.empty')
			.then(cy.wrap);
	}

	beforeEach(function() {
		cy.task('writeUserViewSetting', {
			userId: 'test',
			settings: {
				aiProviderURL: 'https://api.openai.com',
				aiProviderAPIKey: 'sk-saved-secret',
				aiProviderModel: 'gpt-4o',
			},
		});

		helper.setupAndLoadDocument('calc/cell_cursor.ods');
	});

	afterEach(function() {
		cy.task('removeUserViewSetting', { userId: 'test' });
	});

	it('reopening the dialog re-fetches models with the stored key and enables the Model dropdown', function() {
		// Stub the model list; gpt-4o-mini is offered only here, so seeing it in
		// the dropdown proves the list came from this fetch.
		cy.intercept('POST', '**/browser/dist/fetch-models', function(req) {
			expect(req.body).to.include('currentFileUrl');
			expect(req.body).to.include('aiProviderAPIKey');
			req.reply({
				statusCode: 200,
				body: { data: [{ id: 'gpt-4o' }, { id: 'gpt-4o-mini' }] },
			});
		}).as('fetchModels');

		cy.getFrameWindow().then(function(win) {
			win.app.map.settings.showSettingsDialog('');
		});
		cy.cGet('.iframe-settings-wrap').should('be.visible');

		// The iframe loads the settings and the view settings before a debounce
		// elapses, so allow well over the default five seconds.
		cy.wait('@fetchModels', { timeout: 20000 });

		// Dropdown is live: saved model kept, fetch-only model now selectable.
		settingsIframeBody()
			.find('#aiProviderModel', { timeout: 10000 })
			.should('not.be.disabled')
			.and('have.value', 'gpt-4o');

		settingsIframeBody()
			.find('#aiProviderModel option')
			.should('contain', 'gpt-4o-mini');
	});
});
