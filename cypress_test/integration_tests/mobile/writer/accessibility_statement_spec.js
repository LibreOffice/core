/* global describe it cy beforeEach require expect */

var helper = require('../../common/helper');

describe(['tagmobile'], 'Accessibility statement notice', function() {

	beforeEach(function() {
		helper.setupAndLoadDocument('writer/focus.odt');
	});

	it('Says nothing when the deployment has no accessibility statement.', function() {
		cy.getFrameWindow().then(function(win) {
			expect(win.accessibilityStatementUrl).to.be.empty;
		});

		cy.cGet('body').should('not.have.descendants', '.accessibility-notice');
	});
});
