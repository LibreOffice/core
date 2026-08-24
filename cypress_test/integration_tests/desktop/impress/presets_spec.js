/* global describe it cy beforeEach require expect Cypress */

var helper = require('../../common/helper');

describe(['tagdesktop', 'tagnextcloud', 'tagproxy'], 'Preset tests.', function() {

	beforeEach(function() {
		helper.setupAndLoadDocument('impress/testfile.fodp');
	});

	// Ensure that the shared template was installed.
	it('Verify shared template install.', function() {
		cy.getFrameWindow().then(function(win) {
			return new Cypress.Promise(function(resolve, reject) {
				var timer = setTimeout(function() {
					win.app.map.off('commandvalues', onValues);
					reject(new Error('No .uno:GetDesignTemplates reply arrived'));
				}, 20000);

				function onValues(e) {
					if (e.commandName !== '.uno:GetDesignTemplates')
						return;
					clearTimeout(timer);
					win.app.map.off('commandvalues', onValues);
					resolve(e.commandValues && e.commandValues.templates ?
						e.commandValues.templates : []);
				}

				win.app.map.on('commandvalues', onValues);
				win.app.socket.sendMessage(
					'commandvalues command=.uno:GetDesignTemplates');
			});
		}).then(function(templates) {
			expect(templates.map(function(entry) { return entry.name; }))
				.to.include('templateshared');
		});
	});
});
