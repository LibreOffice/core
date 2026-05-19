/* global describe it cy beforeEach require */

var helper = require('../../common/helper');
var mobileHelper = require('../../common/mobile_helper');

describe(['tagmobile', 'tagnextcloud', 'tagproxy'], 'Mobile wizard state tests', function() {

	beforeEach(function() {
		helper.setupAndLoadDocument('writer/mobile_wizard_state.odt');
	});

	it('Open and close mobile wizard by toolbar item.', function() {
		// Click on edit button
		mobileHelper.enableEditingMobile();
		mobileHelper.openMobileWizard();
		// Close mobile wizard
		mobileHelper.closeMobileWizard();
		// Open mobile wizard again
		mobileHelper.openMobileWizard();
	});

	it('Close mobile wizard by hamburger menu.', function() {
		// Click on edit button
		mobileHelper.enableEditingMobile();
		mobileHelper.openMobileWizard();
		// Open hamburger menu
		mobileHelper.openHamburgerMenu();
		cy.cGet('body').contains('.ui-header.level-0.mobile-wizard.ui-widget .menu-entry-with-icon', 'Track Changes').should('be.visible');
		// Close hamburger menu
		mobileHelper.closeHamburgerMenu();
		// Open mobile wizard again
		mobileHelper.openMobileWizard();
	});

	// Regression: after closing the hamburger menu, a single click on
	// the mobile wizard toolbar button must reopen the wizard. With the
	// legacy refresh-sidebar pathway removed, core's sidebar state
	// could fall out of sync with the browser's after the hamburger
	// menu took it over, and the first reopen click would no-op
	// (forcing the user to click twice).
	it('Mobile wizard reopens on first click after hamburger menu.', function() {
		mobileHelper.enableEditingMobile();
		mobileHelper.openMobileWizard();
		mobileHelper.openHamburgerMenu();
		mobileHelper.closeHamburgerMenu();
		// Single click - not through openMobileWizard which would
		// drain core and mask the race.
		cy.cGet('#toolbar-up #mobile_wizard')
			.should('not.have.class', 'disabled')
			.click();
		cy.cGet('#mobile-wizard-content').should('not.be.empty');
		cy.cGet('#toolbar-up #mobile_wizard').should('have.class', 'selected');
	});

	it('Close mobile wizard by context wizard.', function() {
		// Click on edit button
		mobileHelper.enableEditingMobile();
		mobileHelper.openMobileWizard();

		// Open context wizard by right click on document
		// Longpress is naturally supported now.  So we use contextmenu event to simulate right click.
		cy.cGet('#document-canvas').trigger('contextmenu', 40, 40);

		cy.cGet('body').contains('.ui-header.level-0.mobile-wizard.ui-widget .menu-entry-with-icon', 'Paste').should('be.visible');

		// TODO: fix this bug
		//cy.get('#toolbar-up #mobile_wizard table')
		//	.should('not.have.class', 'checked');

		// Open mobile wizard again
		cy.cGet('#toolbar-up #mobile_wizard').click();

		// TODO: fix this bug
		//cy.get('#mobile-wizard-content')
		//	.should('not.be.empty');
		//cy.get('#toolbar-up #mobile_wizard table')
		//	.should('have.class', 'checked');
	});
});

