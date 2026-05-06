/* global describe it cy beforeEach require expect */

var helper = require('../../common/helper');
var desktopHelper = require('../../common/desktop_helper');

describe(['tagdesktop'], 'Ruler spacer section tests.', function() {

	beforeEach(function() {
		helper.setupAndLoadDocument('writer/notebookbar.odt');
		desktopHelper.switchUIToNotebookbar();
		cy.cGet('#View-tab-label').click();
	});

	it('RulerSpacer section is created when ruler is shown and removed when hidden', function() {
		// Ensure ruler is hidden initially.
		cy.cGet('#showruler-input').should('not.be.checked');
		cy.cGet('.cool-ruler').should('not.be.visible');

		// Verify ruler spacer test-div does not exist.
		cy.cGet('[id="test-div-ruler spacer"]').should('not.exist');

		// Show the ruler.
		cy.cGet('#showruler-input').check();
		cy.cGet('#showruler-input').should('be.checked');
		cy.cGet('.cool-ruler').should('be.visible');

		// Verify ruler spacer test-div is created with non-zero height.
		cy.cGet('[id="test-div-ruler spacer"]').should('exist');
		cy.cGet('[id="test-div-ruler spacer"]').then(function($el) {
			var height = parseInt($el.css('height'), 10);
			expect(height).to.be.greaterThan(0);
		});

		// Hide the ruler.
		cy.cGet('#showruler-input').uncheck();
		cy.cGet('#showruler-input').should('not.be.checked');
		cy.cGet('.cool-ruler').should('not.be.visible');

		// Verify ruler spacer test-div is removed.
		cy.cGet('[id="test-div-ruler spacer"]').should('not.exist');
	});

	it('Rulers are hidden from assistive technology', function() {
		// Show the ruler.
		cy.cGet('#showruler-input').check();
		cy.cGet('#showruler-input').should('be.checked');
		cy.cGet('.cool-ruler').should('be.visible');

		// The horizontal ruler wrapper must have aria-hidden so screen
		// readers skip it.  The accessible alternative is the Paragraph
		// dialog (Format > Paragraph).
		cy.cGet('.cool-ruler:not(.vruler)')
			.should('have.attr', 'aria-hidden', 'true');

		// Same for the vertical ruler.
		cy.cGet('#vertical-ruler')
			.should('have.attr', 'aria-hidden', 'true');
	});
});
