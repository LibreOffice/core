/* -*- js-indent-level: 8 -*- */
/* global describe it cy before beforeEach afterEach require expect */

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
	var originalCall = null;
	var originalMessage = null;

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
				delete win.app.map.backstageView.hasUserTemplates;
			}
			if (originalCall) {
				win.postMobileCall = originalCall;
				originalCall = null;
			}
			if (originalMessage) {
				win.postMobileMessage = originalMessage;
				originalMessage = null;
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

	it('Offers four blank documents on Home', function() {
		cy.then(() => {
			ensureBackstage(win).show();
		});

		cy.cGet('#backstage-home').click();
		cy.cGet('.backstage-home-templates-row .backstage-template-card').should(function(cards) {
			var ids = cards.toArray().map(function(card) { return card.id; });
			expect(ids).to.deep.equal([
				'blank-writer', 'blank-calc', 'blank-impress', 'blank-draw',
			]);
		});
	});

	it('Filters templates by type with the chips', function() {
		cy.then(() => {
			ensureBackstage(win).show();
		});

		cy.cGet('#backstage-new').click();
		cy.cGet('.template-type-button.active').should('have.text', 'All');
		cy.cGet('.backstage-template-sections .backstage-template-card')
			.should('have.length.greaterThan', 0);

		cy.cGet('body').contains('.template-type-button', 'Presentation').click();
		cy.cGet('.template-type-button.active').should('have.text', 'Presentation');
		cy.cGet('.backstage-template-sections .backstage-template-card').should(function(cards) {
			expect(cards.length).to.be.greaterThan(0);
			cards.toArray().forEach(function(card) {
				expect(card.id).to.match(/^(impress-|introdoc-impress$)/);
			});
		});

		// The blank documents are not templates and stay whatever the filter.
		cy.cGet('.template-blank-row .backstage-template-card').should('have.length', 4);

		// No built-in template is a drawing.
		cy.cGet('body').contains('.template-type-button', 'Drawing').click();
		cy.cGet('.template-grid-empty').should('contain.text', 'No templates of this type');

		cy.cGet('body').contains('.template-type-button', 'All').click();
		cy.cGet('.template-grid-empty').should('not.exist');
	});

	it('Shows no My templates section outside the Qt app', function() {
		cy.then(() => {
			ensureBackstage(win).show();
		});

		cy.cGet('#backstage-new').click();
		cy.cGet('body').contains('.backstage-template-section-title', 'Built-in templates');
		cy.cGet('.backstage-my-templates-grid').should('not.exist');
	});

	it('Lists the templates of the user in the Qt app, and manages them', function() {
		cy.then(() => {
			// Only the Qt app keeps templates of the user, so this test stands in for it.
			var backstage = ensureBackstage(win);
			backstage.hasUserTemplates = function() { return true; };
			originalCall = win.postMobileCall;
			win.postMobileCall = function(message) {
				if (message === 'GETUSERTEMPLATES')
					return Promise.resolve([
						{ id: 'A1', name: 'Invoice', type: 'calc', path: '/templates/A1/Invoice.ots', size: 2048 },
					]);
				return Promise.resolve(undefined);
			};
			backstage.show();
		});

		cy.cGet('#backstage-new').click();
		cy.cGet('.backstage-my-templates-grid .backstage-template-card').should(function(cards) {
			expect(cards.toArray().map(function(card) { return card.id; }))
				.to.deep.equal(['', 'user-template-A1']);
		});
		cy.cGet('.backstage-add-template-card').should('contain.text', 'Add template');

		// The chips filter the templates of the user too, but the Add template card stays.
		cy.cGet('body').contains('.template-type-button', 'Document').click();
		cy.cGet('#user-template-A1').should('not.exist');
		cy.cGet('.backstage-add-template-card').should('exist');
		cy.cGet('body').contains('.template-type-button', 'All').click();
		cy.cGet('#user-template-A1').should('exist');

		cy.cGet('.backstage-manage-templates-button').click();
		cy.cGet('#backstage-new').should('have.class', 'active');
		cy.cGet('#backstage-user-template-A1 .backstage-manage-templates-name').should('have.text', 'Invoice');
		cy.cGet('#backstage-user-template-A1 .backstage-template-type-tag').should('have.text', 'Spreadsheet');

		cy.cGet('#backstage-user-template-A1 .backstage-remove-template-button').click();
		cy.cGet('.backstage-modal-subtitle').should('contain.text', '"Invoice" will be removed');
		cy.cGet('body').contains('.backstage-modal .button', 'Cancel').click();
		cy.cGet('.backstage-modal-overlay').should('not.exist');

		cy.cGet('.backstage-manage-templates-back').click();
		cy.cGet('.backstage-my-templates-grid').should('exist');
	});

	it('Creates a new document from a template of the user, named after the template', function() {
		var messages = [];
		cy.then(() => {
			var backstage = ensureBackstage(win);
			backstage.hasUserTemplates = function() { return true; };
			originalCall = win.postMobileCall;
			win.postMobileCall = function(message) {
				if (message === 'GETUSERTEMPLATES')
					return Promise.resolve([
						{ id: 'B2', name: 'Monthly Letter', type: 'writer', path: '/templates/B2/Monthly Letter.dotx', size: 1024 },
					]);
				return Promise.resolve(undefined);
			};
			originalMessage = win.postMobileMessage;
			win.postMobileMessage = function(message) { messages.push(message); };
			backstage.show();
		});

		cy.cGet('#backstage-new').click();
		cy.cGet('#user-template-B2').click();
		cy.wrap(messages).should('deep.equal', [
			'newdoc type=writer template=%2Ftemplates%2FB2%2FMonthly%20Letter.dotx basename=Monthly%20Letter',
		]);
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
