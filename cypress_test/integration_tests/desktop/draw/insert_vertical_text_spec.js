/* global describe it cy beforeEach require */

var helper = require('../../common/helper');
var desktopHelper = require('../../common/desktop_helper');
var impressHelper = require('../../common/impress_helper');

describe(['tagdesktop'], 'Insert vertical text', function() {

	beforeEach(function() {
		helper.setupAndLoadDocument('draw/vertical_text.fodg');
		desktopHelper.switchUIToNotebookbar();
	});

	it('Vertical text box is inserted with the keyboard alone', function() {
		cy.viewport(1920, 1080);

		cy.getFrameWindow().its('app.UI.notebookbarAccessibility.initialized')
			.should('be.true');

		// Alt raises the notebookbar accelerators, N picks the Insert tab and
		// VT the Vertical Text item.
		helper.typeIntoDocument('{alt}');
		cy.cGet('#accessibilityInputElement').should('be.focused');

		cy.realPress('N');
		cy.cGet('#Insert-tab-label').should('have.class', 'selected');

		cy.realPress('V');
		cy.realPress('T');

		impressHelper.assertInTextEditMode();

		// Only a vertical box advances the caret downwards as you type.
		cy.getFrameWindow().its('app.file.textCursor.rectangle.y1').then(function(caretTop) {
			helper.typeIntoDocument('ab');
			cy.getFrameWindow().its('app.file.textCursor.rectangle.y1')
				.should('be.greaterThan', caretTop);
		});
	});
});
