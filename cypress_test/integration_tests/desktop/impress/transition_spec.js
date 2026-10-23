/* global describe it cy beforeEach require */

var helper = require('../../common/helper');
var desktopHelper = require('../../common/desktop_helper');

describe(['tagdesktop'], 'Impress Transition Tab Tests', function() {

	beforeEach(function() {
		helper.setupAndLoadDocument('impress/slideshow.odp');
		desktopHelper.switchUIToNotebookbar();
	});

	it('Transition tab can be selected', function() {
		cy.cGet('#Transition-tab-label').click();
		cy.cGet('#Transition-tab-label').should('have.class', 'selected');
	});
});
