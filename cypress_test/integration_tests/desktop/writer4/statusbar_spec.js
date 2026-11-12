/* global describe it cy beforeEach require Cypress */

var helper = require('../../common/helper');
var desktopHelper = require('../../common/desktop_helper');

describe(['tagdesktop', 'tagnextcloud', 'tagproxy'], 'Statusbar tests.', function() {

	beforeEach(function() {
		helper.setupAndLoadDocument('writer/statusbar.odt', false, false, undefined, 'vector=false');
		desktopHelper.switchUIToCompact();

		if (Cypress.env('INTEGRATION') === 'nextcloud') {
			desktopHelper.showStatusBarIfHidden ();
		}

		desktopHelper.shouldHaveZoomLevel('70');
	});

	it('Text selection.', function() {
		cy.cGet('body').contains('#toolbar-down #StateWordCount', '2 words, 9 characters');
		helper.moveCursor('right', 'shift');
		cy.cGet('body').contains('#toolbar-down #StateWordCount', 'Selected: 1 word, 1 character');
	});

	it('Switching page.', function() {
		desktopHelper.assertVisiblePage(1, 1, 1);
		cy.cGet('#menu-insert').click();
		cy.cGet('body').contains('#menu-insert li a', 'Page Break').click();
		desktopHelper.assertVisiblePage(1, 2, 2);
		cy.cGet('#toolbar-down #prevpage').click();
		desktopHelper.assertVisiblePage(1, 1, 2);
		desktopHelper.assertScrollbarPosition('vertical', 0, 10);
		cy.cGet('#toolbar-down #nextpage').click();
		desktopHelper.assertVisiblePage(1, 2, 2);
		desktopHelper.assertScrollbarPosition('vertical', 187, 197);
	});

	it('Switching page after scrolling.', function() {
		cy.cGet('#menu-insert').click();
		cy.cGet('body').contains('#menu-insert li a', 'Page Break').click();
		cy.cGet('#menu-insert').click();
		cy.cGet('body').contains('#menu-insert li a', 'Page Break').click();
		desktopHelper.assertVisiblePage(3, 3, 3);
		helper.typeIntoDocument('{ctrl}{home}');
		desktopHelper.assertVisiblePage(1, 1, 3);
		cy.getFrameWindow().then(function(win) {
			desktopHelper.scrollViewDown(win);
			desktopHelper.scrollViewDown(win);
			desktopHelper.scrollViewDown(win);
		});
		cy.cGet('#toolbar-down #nextpage').click();
		desktopHelper.assertVisiblePage(3, 3, 3);
		cy.cGet('#toolbar-down #prevpage').click();
		desktopHelper.assertVisiblePage(2, 2, 3);
		cy.cGet('.cursor-overlay .blinking-cursor').should('be.visible');
	});

	it('Text entering mode.', function() {
		cy.cGet('#InsertMode').should('have.text', 'Insert');
		helper.typeIntoDocument('{insert}');
		cy.cGet('#InsertMode').should('have.text', 'Insert');
	});

	it('Change zoom level.', function() {
		desktopHelper.fitWidthZoom();
		desktopHelper.shouldHaveZoomLevel('70');
		desktopHelper.zoomIn();
		desktopHelper.shouldHaveZoomLevel('85');
		desktopHelper.zoomOut();
		desktopHelper.shouldHaveZoomLevel('70');
	});

	it('Select zoom level.', function() {
		desktopHelper.fitWidthZoom();
		desktopHelper.shouldHaveZoomLevel('70');
		desktopHelper.selectZoomLevel('280', false);
		desktopHelper.shouldHaveZoomLevel('280');
	});
});
