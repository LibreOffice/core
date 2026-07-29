/* global describe it cy beforeEach require expect */

var helper = require('../../common/helper');

describe(['tagmobile'], 'Accessibility statement notice', function() {

	beforeEach(function() {
		helper.setupAndLoadDocument('writer/focus.odt');
	});

	function configureStatement(win) {
		win.accessibilityStatementUrl = 'https://example.com/accessibility';
	}

	it('Says nothing when the deployment has no accessibility statement.', function() {
		cy.getFrameWindow().then(function(win) {
			expect(win.accessibilityStatementUrl).to.be.empty;
		});

		cy.cGet('body').should('not.have.descendants', '.accessibility-notice');
	});

	it('Warns about the mobile view and offers the statement.', function() {
		cy.getFrameWindow().then(function(win) {
			configureStatement(win);
			win.app.map.uiManager.showAccessibilityStatementNotice();
		});

		cy.cGet('.accessibility-notice-message')
			.should('have.text', 'This mobile view is not fully accessible.');
		cy.cGet('.accessibility-notice-link')
			.should('have.text', 'Accessibility statement')
			.and('have.attr', 'href', 'https://example.com/accessibility')
			.and('have.attr', 'target', '_blank');
	});

	it('Speaks the warning without moving the focus.', function() {
		cy.getFrameWindow().then(function(win) {
			configureStatement(win);
			win.app.map.uiManager.showAccessibilityStatementNotice();
		});

		// The warning lands in an alert a moment after the notice appears, so
		// that a screen reader speaks it once the page announcement is over.
		cy.cGet('.accessibility-notice-announcement')
			.should('have.attr', 'role', 'alert')
			.and('have.text', '');
		cy.cGet('.accessibility-notice-announcement', { timeout: 10000 })
			.should('have.text', 'This mobile view is not fully accessible.');

		// The notice never takes the focus away from the document.
		cy.cGet('.accessibility-notice a, .accessibility-notice button')
			.should('not.have.focus');
	});

	it('Dismissing the notice takes it off the screen.', function() {
		cy.getFrameWindow().then(function(win) {
			configureStatement(win);
			win.app.map.uiManager.showAccessibilityStatementNotice();
		});

		cy.cGet('.accessibility-notice-dismiss').click();

		cy.cGet('body').should('not.have.descendants', '.accessibility-notice');
	});
});
