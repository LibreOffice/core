/* global describe it cy beforeEach require */

var helper = require('../../common/helper');
var desktopHelper = require('../../common/desktop_helper');

describe(['tagdesktop'], 'File menu offers Rename Document in Draw', function() {

	beforeEach(function() {
		helper.setupAndLoadDocument('draw/insert_position.fodg');
		desktopHelper.switchUIToCompact();
		cy.getFrameWindow().then(function(win) {
			this.win = win;
		});
	});

	function setRenameAllowed(allowed) {
		cy.getFrameWindow().then(function(win) {
			win.app.map.wopi.SupportsRename = true;
			win.app.map.wopi.UserCanRename = allowed;
			win.app.map.menubar.refresh();
		});
		helper.processToIdle(this.win);
	}

	it('opens the rename dialog from File when the host allows rename', function() {
		setRenameAllowed.call(this, true);

		cy.cGet('#menu-file').click();
		cy.cGet('#menu-renamedocument > a').should('be.visible');
		cy.cGet('#menu-renamedocument > a').click();

		cy.cGet('#rename-modal').should('be.visible');
	});

	it('leaves Rename Document out when the user cannot rename', function() {
		setRenameAllowed.call(this, false);

		cy.cGet('#menu-file').click();
		cy.cGet('#menu-file > ul > li > a:visible').should('have.length.greaterThan', 0);
		cy.cGet('#menu-renamedocument').should('not.exist');
	});
});
