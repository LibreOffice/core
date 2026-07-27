/* -*- js-indent-level: 8 -*- */

/* global describe it cy require beforeEach expect */
var helper = require('../../common/helper');

describe(['tagdesktop'], 'JSDialog Help button test', function() {

	beforeEach(function() {
		helper.setupAndLoadDocument('calc/help_dialog.ods');
		cy.viewport(1920,1080);
	});

	// Skipped: help is disabled by default (empty HELP_URL), so the Help
	// button is hidden and no external help link popup appears. Revive this
	// once builds are configured with --with-help-url again.
	it.skip('JSDialog Help button opens online help', function() {
		// Open Validation dialog (has a Help button)
		cy.cGet('#Data-tab-label').click();
		cy.cGet('.unoValidation').click();

		// Click the Help button
		cy.cGet('#help').click();

		// The "External link" popup should appear with the help URL
		cy.cGet('#modal-dialog-openlink').should('be.visible');
		cy.cGet('#modal-dialog-openlink').should('contain.text', 'External link');
		cy.cGet('#info-modal-label2').should('contain.text', 'help.collaboraoffice.com');

		// Close it
		cy.cGet('#openlink-response').click();
	});

	it('Help button focus ring is not clipped by the dialog', function() {
		// Open a dialog that carries a Help button (Data > Validity).
		cy.cGet('#Data-tab-label').click();
		cy.cGet('.unoValidation').click();
		cy.getFrameWindow().then(function(win) { return helper.processToIdle(win); });

		// Empty HELP_URL hides the Help button in test builds; reveal it so it
		// can take focus and expose its ring.
		cy.cGet('#help').invoke('removeClass', 'hidden');
		cy.cGet('#help-button').focus().should('have.focus');

		cy.cGet('#help-button').then(function($btn) {
			var btn = $btn[0];
			var win = btn.ownerDocument.defaultView;

			// Nearest ancestor that clips overflow (the scrollable dialog content).
			var clip = btn.parentElement;
			while (clip && win.getComputedStyle(clip).overflow === 'visible')
				clip = clip.parentElement;
			expect(clip, 'a clipping ancestor exists').to.not.be.null;

			// The focus ring is drawn outline-offset + outline-width outside the
			// button border box.
			var cs = win.getComputedStyle(btn);
			var ring = (parseFloat(cs.outlineWidth) || 0) + (parseFloat(cs.outlineOffset) || 0);
			expect(ring, 'a focus ring is rendered').to.be.greaterThan(0);

			var b = btn.getBoundingClientRect();
			var c = clip.getBoundingClientRect();

			// The whole ring rectangle must sit inside the clipping container.
			expect(b.left - ring, 'ring left edge').to.be.at.least(c.left);
			expect(b.top - ring, 'ring top edge').to.be.at.least(c.top);
			expect(b.right + ring, 'ring right edge').to.be.at.most(c.right);
			expect(b.bottom + ring, 'ring bottom edge').to.be.at.most(c.bottom);
		});
	});

	it('Settings nav item focus ring is not clipped by the nav column', function() {
		// Open the Options (settings) dialog. Its UI is rendered inside a
		// nested, same-origin iframe.
		cy.getFrameWindow().then(function(win) {
			win.app.map.settings.showSettingsDialog();
		});

		// Wait until the iframe has loaded and its category list is populated.
		cy.cGet('.iframe-settings-modal').should(function($iframe) {
			var doc = $iframe[0].contentDocument;
			expect(doc, 'settings iframe document').to.not.be.null;
			expect(
				doc.querySelectorAll('.settings-nav-item').length,
				'settings nav items loaded'
			).to.be.greaterThan(0);
		});

		// The settings page keeps fetching/rebuilding while it loads, so retry
		// until a nav item is actually laid out, then check its geometry.
		cy.cGet('.iframe-settings-modal').should(function($iframe) {
			var doc = $iframe[0].contentDocument;

			// A category link flush against the nav column edge is the one
			// whose focus ring gets clipped, so exercise the first one.
			var item = doc.querySelector('.settings-nav-item');
			expect(item, 'a settings nav item exists').to.not.be.null;

			var b = item.getBoundingClientRect();
			expect(b.width, 'nav item is rendered').to.be.greaterThan(0);

			// Focus it - the ring only matters when the item is focused.
			// preventScroll keeps focus from scrolling the padding out of view.
			item.focus({ preventScroll: true });
			expect(doc.activeElement, 'nav item is focused').to.equal(item);

			// The nav column clips overflow (overflow-y:auto forces overflow-x
			// to clip too), so it is what would cut off the item's focus ring.
			var clip = doc.getElementById('settings-nav');
			expect(clip, 'nav column exists').to.not.be.null;

			var c = clip.getBoundingClientRect();

			// The item must sit inside the clipping column with room on every
			// side for the focus ring - in particular on the inline-start edge,
			// where it used to be flush (no padding) and the ring was cut off.
			// A couple of pixels of slack is enough to clear a normal ring.
			var slack = 2;
			expect(b.left - c.left, 'gap to nav left edge').to.be.at.least(slack);
			expect(c.right - b.right, 'gap to nav right edge').to.be.at.least(0);
			expect(b.top - c.top, 'gap to nav top edge').to.be.at.least(0);
			expect(c.bottom - b.bottom, 'gap to nav bottom edge').to.be.at.least(0);
		});
	});
});
