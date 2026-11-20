/* global describe it cy beforeEach require */

var helper = require('../../common/helper');
var desktopHelper = require('../../common/desktop_helper');

describe(['tagdesktop'], 'Menubar drop-down stays open while the bars update', function () {
	beforeEach(function () {
		cy.viewport(1280, 500);

		helper.setupAndLoadDocument('calc/focus.ods', false, false, undefined, 'vector=false');
		// The classic menubar (#menu-*) only exists in the compact UI; the
		// desktop default renders the notebookbar.
		desktopHelper.switchUIToCompact();
		cy.cGet('#menu-format').should('be.visible');
		cy.getFrameWindow().then((win) => {
			this.win = win;
		});
		cy.then(() => helper.processToIdle(this.win));
	});

	// Status bar and toolbar updates refresh the scrollable bars, and they
	// can arrive while the user has a menu open.
	it('keeps an open drop-down open when the scrollable bars refresh', function () {
		cy.cGet('#menu-format').click();
		cy.cGet('#menu-format > ul').should('be.visible');

		cy.then(() => this.win.JSDialog.RefreshScrollables());

		cy.cGet('#menu-format > ul').should('be.visible');
	});
});
