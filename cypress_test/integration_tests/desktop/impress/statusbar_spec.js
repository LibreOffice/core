/* global describe it cy beforeEach require Cypress */

var helper = require('../../common/helper');
var desktopHelper = require('../../common/desktop_helper');

describe(['tagdesktop', 'tagnextcloud', 'tagproxy'], 'Statusbar tests.', function() {

	beforeEach(function() {
		helper.setupAndLoadDocument('impress/statusbar.odp');
		if (Cypress.env('INTEGRATION') === 'nextcloud') {
			desktopHelper.showStatusBarIfHidden ();
		}
		cy.viewport(1920, 1080);
	});

	it('Selected slide.', function() {
		cy.cGet('#SlideStatus').should('have.text', 'Slide 1 of 2');
		cy.cGet('#toolbar-down #nextpage').click();
		cy.cGet('#SlideStatus').should('have.text', 'Slide 2 of 2');
		cy.cGet('#toolbar-down #prevpage').click();
		cy.cGet('#SlideStatus').should('have.text', 'Slide 1 of 2');
	});

	it('Change zoom level.', function() {
		desktopHelper.fitWidthZoom();
		desktopHelper.shouldHaveZoomLevel('100');
		desktopHelper.zoomIn();
		desktopHelper.shouldHaveZoomLevel('120');
		desktopHelper.zoomOut();
		desktopHelper.shouldHaveZoomLevel('100');
	});

	it('Select zoom level.', function() {
		desktopHelper.fitWidthZoom();
		desktopHelper.shouldHaveZoomLevel('100');
		desktopHelper.selectZoomLevel('280', false);
		desktopHelper.shouldHaveZoomLevel('280');
	});

	it('Dynamic Zoom', function () {
		desktopHelper.fitWidthZoom();
		desktopHelper.shouldHaveZoomLevel('100');

		cy.viewport(1420, 1080);
		desktopHelper.fitWidthZoom();
		desktopHelper.shouldHaveZoomLevel('70');

		desktopHelper.zoomIn();
		desktopHelper.zoomIn();
		desktopHelper.fitWidthZoom();
		desktopHelper.shouldHaveZoomLevel('70');
	});
});
