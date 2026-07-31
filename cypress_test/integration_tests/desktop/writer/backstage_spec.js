/* -*- js-indent-level: 8 -*- */
/* global describe it cy before beforeEach afterEach require */

var helper = require('../../common/helper');

// Backstage view is normally instantiated only by UIManager when running
// inside a CODA app (mode.isCODesktop()). In a regular browser Cypress
// run, app.map.backstageView is undefined. We construct one explicitly so
// the templates exercise the same code path they would in CODA.
function ensureBackstage(win) {
	if (!win.app.map.backstageView) {
		win.app.map.backstageView = new win.L.Control.BackstageView(win.app.map);
	}
	return win.app.map.backstageView;
}

describe(['tagdesktop'], 'Backstage View Tests', { testIsolation: false }, function() {

	let win;

	before(function() {
		helper.setupAndLoadDocument('writer/help_dialog.odt');
		cy.getFrameWindow().then((frameWindow) => {
			win = frameWindow;
		});
	});

	beforeEach(function() {
		// Cypress restores the configured viewport before every test, so the
		// size the tests were written for is set again here.
		cy.viewport(1400, 1000);
	});

	afterEach(function() {
		// Hide the backstage view in case a test fails partway through, so
		// document container is restored for subsequent tests.
		cy.then(() => {
			if (win.app.map.backstageView) {
				win.app.map.backstageView.hide();
			}
		});
	});

	// The backstage view is built once and stays for the whole file, so the
	// test that reads it as first built comes before the ones that switch tabs.
	it('Shows backstage chrome with sidebar tabs and content area', function() {
		cy.then(() => {
			ensureBackstage(win).show();
		});

		cy.cGet('.backstage-view').should('be.visible');
		cy.cGet('.backstage-view').should('not.have.class', 'hidden');
		cy.cGet('.backstage-header').should('be.visible');
		cy.cGet('.backstage-header-title').should('contain.text', 'Collabora Office');
		cy.cGet('.backstage-sidebar').should('be.visible');
		cy.cGet('.backstage-content').should('be.visible');

		// Tabs that should always exist when not in starter mode.
		cy.cGet('#backstage-home').should('exist');
		cy.cGet('#backstage-new').should('exist');
		cy.cGet('#backstage-open').should('exist');
		cy.cGet('#backstage-info').should('exist');
		cy.cGet('#backstage-export').should('exist');
		cy.cGet('#backstage-about').should('exist');

		// Home tab is active by default.
		cy.cGet('#backstage-home').should('have.class', 'active');
	});

	it('Switches between view tabs', function() {
		cy.then(() => {
			ensureBackstage(win).show();
		});

		cy.cGet('#backstage-new').click();
		cy.cGet('#backstage-new').should('have.class', 'active');
		cy.cGet('#backstage-home').should('not.have.class', 'active');
		cy.cGet('.backstage-template-explorer').should('exist');

		cy.cGet('#backstage-info').click();
		cy.cGet('#backstage-info').should('have.class', 'active');
		cy.cGet('.backstage-info-properties').should('be.visible');
		cy.cGet('.backstage-properties-list').should('be.visible');
		cy.cGet('.backstage-property-item').should('have.length.at.least', 1);

		cy.cGet('#backstage-export').click();
		cy.cGet('#backstage-export').should('have.class', 'active');
		// Export grid only renders when the notebookbar builder reports
		// download formats. We still expect the section header to appear
		// when at least one format is available; if none are, the content
		// area may be empty. Just assert that no other view is leaking.
		cy.cGet('.backstage-info-properties').should('not.exist');
		cy.cGet('.backstage-template-explorer').should('not.exist');

		cy.cGet('#backstage-home').click();
		cy.cGet('#backstage-home').should('have.class', 'active');
	});

	it('Filters templates via the search input', function() {
		cy.then(() => {
			ensureBackstage(win).show();
		});

		cy.cGet('#backstage-new').click();
		cy.cGet('.backstage-template-explorer').should('exist');

		// Type a query that almost certainly matches no template name.
		cy.cGet('.template-search-input').type('zzz_no_such_template_xyz');
		cy.cGet('.template-grid-empty')
			.should('be.visible')
			.and('contain.text', 'No templates match your search');

		// Clearing the search should remove the empty-state message.
		cy.cGet('.template-search-input').clear();
		cy.cGet('.template-grid-empty').should('not.exist');
	});

	it('Closes via the sidebar back button', function() {
		cy.then(() => {
			ensureBackstage(win).show();
		});

		cy.cGet('.backstage-view').should('not.have.class', 'hidden');
		cy.cGet('.backstage-sidebar-back').click();
		cy.cGet('.backstage-view').should('have.class', 'hidden');
	});

	it('Toggles via the public API', function() {
		cy.then(() => {
			const bv = ensureBackstage(win);
			bv.show();
		});
		cy.cGet('.backstage-view').should('not.have.class', 'hidden');

		cy.then(() => {
			win.app.map.backstageView.hide();
		});
		cy.cGet('.backstage-view').should('have.class', 'hidden');

		cy.then(() => {
			win.app.map.backstageView.toggle();
		});
		cy.cGet('.backstage-view').should('not.have.class', 'hidden');
	});
});

// Whether the header carries a close button is settled when the view is built,
// so each of these two tests needs a session of its own with the setting it
// asks about.
describe(['tagdesktop'], 'Backstage View close button', function() {

	beforeEach(function() {
		cy.viewport(1400, 1000);
		helper.setupAndLoadDocument('writer/help_dialog.odt');
		cy.getFrameWindow().then((win) => {
			this.win = win;
		});
	});

	it('Closes via the header close button', function() {
		cy.then(() => {
			// The header close button only appears when closing the
			// document is enabled for this session.
			this.win.L.Params.closeButtonEnabled = true;
			ensureBackstage(this.win).show();
		});

		cy.cGet('.backstage-view').should('not.have.class', 'hidden');
		cy.cGet('.backstage-header-close').click();
		cy.cGet('.backstage-view').should('have.class', 'hidden');
	});

	it('Omits the header close button when closing the document is disabled', function() {
		cy.then(() => {
			this.win.L.Params.closeButtonEnabled = false;
			ensureBackstage(this.win).show();
		});

		cy.cGet('.backstage-view').should('not.have.class', 'hidden');
		cy.cGet('.backstage-header-close').should('not.exist');
		// The sidebar back button still returns to the document.
		cy.cGet('.backstage-sidebar-back').should('be.visible');
	});
});
