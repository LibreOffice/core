/* global describe it cy require beforeEach Cypress */

var helper = require('../../common/helper');
var desktopHelper = require('../../common/desktop_helper');
var writerHelper = require('../../common/writer_helper');

describe(['tagdesktop'], 'Sidebar style combo box', function() {

	beforeEach(function() {
		cy.viewport(1280, 1024);
		helper.setupAndLoadDocument('writer/top_toolbar.odt');
		// The sidebar is already open by default, except under the nextcloud
		// integration, which starts with it closed.
		if (Cypress.env('INTEGRATION') === 'nextcloud') {
			desktopHelper.showSidebar();
		}
		cy.cGet('#applystyle .ui-combobox-content').should('exist');

		cy.getFrameWindow().then((win) => {
			this.win = win;
		});
	});

	it('Typing a name with no matching style creates nothing', function() {
		var bogusName = 'ZzNoSuchStyleXyz';

		cy.cGet('#applystyle .ui-combobox-content').click().clear();
		// One character at a time with a delay, the way a user searches the
		// list by typing slowly, then commit with Return.
		helper.typeText('#applystyle .ui-combobox-content', bogusName, 100);
		cy.cGet('#applystyle .ui-combobox-content').type('{enter}');

		cy.then(() => { return helper.processToIdle(this.win); });

		cy.cGet('#applystyle .ui-combobox-button').click();
		desktopHelper.getDropdown('applystyle').should('not.contain.text', bogusName);
	});

	it('Typing an existing style name still applies it', function() {
		helper.setDummyClipboardForCopy();

		cy.cGet('#applystyle .ui-combobox-content').click().clear().type('Title{enter}');

		writerHelper.selectAllTextOfDoc();
		helper.copy();
		cy.cGet('#copy-paste-container p font font').should('have.attr', 'style', 'font-size: 28pt');
	});
});
