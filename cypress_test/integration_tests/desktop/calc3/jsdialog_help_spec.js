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
});
