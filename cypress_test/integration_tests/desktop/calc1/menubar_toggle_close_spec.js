/* global describe it cy beforeEach require */

var helper = require('../../common/helper');
var desktopHelper = require('../../common/desktop_helper');

describe(['tagdesktop'], 'Menubar top-level item closes on repeated click', function () {
	beforeEach(function () {
		cy.viewport(1280, 500);

		helper.setupAndLoadDocument('calc/focus.ods');
		// The classic menubar (#menu-*) only exists in the compact UI; the
		// desktop default renders the notebookbar.
		desktopHelper.switchUIToCompact();
		cy.cGet('#menu-format').should('be.visible');
		cy.getFrameWindow().then((win) => {
			this.win = win;
		});
		cy.then(() => helper.processToIdle(this.win));
	});

	// Clicking an already-open top-level menu used to reopen it straight
	// away: SmartMenus hid the drop-down, then its own click handling saw
	// the (now hidden) submenu and showed it again in the same click.
	it('closes the drop-down when clicked a second time', function () {
		cy.cGet('#menu-format').click();
		cy.cGet('#menu-format > ul').should('be.visible');

		cy.cGet('#menu-format').click();
		cy.cGet('#menu-format > ul').should('not.be.visible');
	});
});
