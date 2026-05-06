/* global describe it cy beforeEach require */

var helper = require('../../common/helper');
var desktopHelper = require('../../common/desktop_helper');

describe(['tagdesktop'], 'Sidebar tests', function() {

	beforeEach(function() {
		helper.setupAndLoadDocument('writer/sidebar.odt');
		cy.viewport(1920,1080);
	});

	it('Sidebar visual test', function() {
		cy.viewport(1000,660);
		cy.wait(500); // wait to make fully rendered
		cy.cGet('#sidebar-dock-wrapper').scrollTo(0,0,{ ensureScrollable: false });
		cy.wait(500); // wait for animations
		cy.cGet('#sidebar-dock-wrapper').compareSnapshot('sidebar_writer', 0.07);
	});

	it('Show table panel multiple times', function() {
		cy.cGet('.TableEditPanel').should('not.exist');

		cy.cGet('#Insert-tab-label').click();
		cy.cGet('#Insert .unoInsertTable').click();
		cy.cGet('.inserttable-grid > .row > .col').eq(0).click();

		cy.cGet('.TableEditPanel').should('exist');
		cy.cGet('#sidebar-dock-wrapper').scrollTo(0, 300, { ensureScrollable: false });
		cy.cGet('.TableEditPanel').should('be.visible');

		helper.typeIntoDocument('{downArrow}');
		cy.cGet('#sidebar-dock-wrapper').scrollTo(0, 300, { ensureScrollable: false });
		cy.cGet('.TableEditPanel').should('not.be.visible');

		helper.typeIntoDocument('{upArrow}');
		cy.cGet('#sidebar-dock-wrapper').scrollTo(0, 300, { ensureScrollable: false });
		cy.cGet('.TableEditPanel').should('be.visible');
	});

	function checkMathElementsVisibility() {
		cy.viewport(2000, 1080);

		cy.cGet('#Insert-tab-label').click();
		cy.cGet('#Insert .unoInsertObjectStarMath').click();

		cy.cGet('.MathElementsPanel').should('be.visible');
	}

	it('Show formula Elements sidebar when closed', function() {
		// close sidebar first, expect it to auto-open
		desktopHelper.sidebarToggle();
		cy.cGet('#sidebar-dock-wrapper').should('not.be.visible');

		checkMathElementsVisibility();
	});

	it('Show formula Elements sidebar', function() {
		checkMathElementsVisibility();
	});

	it('Show document accessibility sidebar', function() {
		cy.cGet('#Help-tab-label').click();
		cy.cGet('#Help .unoAccessibilityCheck').click();

		cy.cGet('#updateLinkButton').click();

		cy.cGet('#expand_document-label').should('exist');
		cy.cGet('#expand_other').should('exist');
	});

	it('Focus on the first focusable child', function() {
		cy.cGet('#sidebar-dock-wrapper.visible').then(function(sidebar) {
			helper.containsFocusElement(sidebar[0], false);
		});

		// Hide
		desktopHelper.sidebarToggle();
		cy.cGet('#sidebar-dock-wrapper').should('not.be.visible');

		// Show
		desktopHelper.sidebarToggle();
		cy.cGet('#sidebar-panel').should('be.visible');
		cy.cGet('#sidebar-panel').should('not.be.empty');

		cy.wait(1000);

		cy.cGet('#sidebar-dock-wrapper.visible').then(function(sidebar) {
			helper.containsFocusElement(sidebar[0], true)
		});
	});
});
