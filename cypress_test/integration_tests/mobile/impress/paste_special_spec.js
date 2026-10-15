/* global describe it cy beforeEach require */

var helper = require('../../common/helper');
var mobileHelper = require('../../common/mobile_helper');

describe(['tagmobile'], 'Paste special.', function() {

	beforeEach(function() {
		helper.setupAndLoadDocument('impress/hamburger_menu.odp');
		mobileHelper.enableEditingMobile();
	});

	it('Formats of copied text.', function() {
		cy.cGet('#document-canvas').click('center');
		cy.getFrameWindow().then(function(win) {
			helper.waitForTimers(win, 'clicktimer');
		});
		cy.cGet('#document-canvas').dblclick('center');
		cy.cGet('.leaflet-cursor-container').should('be.visible');

		helper.typeIntoDocument('Hello world');
		helper.typeIntoDocument('{shift}{home}');
		cy.getFrameWindow().then(function(win) {
			win.app.map.sendUnoCommand('.uno:Copy');
			helper.processToIdle(win);
		});
		helper.typeIntoDocument('{end}');

		// right click on the second word, the first one is misspelled and would
		// give the spelling menu
		cy.getFrameWindow().then(function(win) {
			win.app.map._docLayer._postMouseEvent('buttondown', 2500, 1850, 1, 4, 0);
		});
		cy.cGet('#mobile-wizard-content').should('be.visible');
		cy.cGet('body').contains('.context-menu-link', 'Paste Special').click();

		cy.cGet('#modal-dialog-paste_special_dialog-box-yesbutton').click();

		cy.cGet('#mobile-wizard-title').should('have.text', 'Paste Special');
		cy.cGet('#PasteSpecialDialog .ui-treeview-cell-text:contains("Unformatted text")').should('exist');
		cy.cGet('#PasteSpecialDialog .ui-treeview-cell-text:contains("Rich text formatting (RTF)")').should('exist');
	});
});
