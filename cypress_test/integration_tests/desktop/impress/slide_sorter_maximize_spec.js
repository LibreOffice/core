/* -*- js-indent-level: 8 -*- */
/* global describe it cy require expect beforeEach */

var helper = require('../../common/helper');

describe(['tagdesktop'], 'Maximized slide sorter', function() {

	beforeEach(function() {
		helper.setupAndLoadDocument('impress/slide_navigation.odp', false, false, undefined, 'vector=false');
		cy.getFrameWindow().then(function(win) {
			this.win = win;
		}.bind(this));
	});

	it('double-clicking a slide in the grid returns to the normal layout on that slide', function() {
		cy.cGet('#main-document-content').should('not.have.class', 'panes-expanded');

		cy.cGet('.navigation-expand-button').should('be.visible').click();
		cy.cGet('#main-document-content').should('have.class', 'panes-expanded');
		cy.cGet('#presentation-controls-wrapper').should('have.class', 'parts-preview-grid');
		cy.cGet('.navigation-expand-button').should('have.attr', 'aria-pressed', 'true');
		helper.processToIdle(this.win);

		cy.cGet('#preview-img-part-2').should('be.visible').dblclick();

		cy.cGet('#main-document-content').should('not.have.class', 'panes-expanded');
		cy.cGet('#presentation-controls-wrapper').should('not.have.class', 'parts-preview-grid');
		cy.cGet('.navigation-expand-button').should('have.attr', 'aria-pressed', 'false');

		helper.processToIdle(this.win);
		cy.cGet('#preview-img-part-2').should('have.class', 'preview-img-currentpart');
		cy.getFrameWindow().should(function(win) {
			expect(win.app.map._docLayer._selectedPart).to.equal(2);
		});
		helper.assertFocus('id', 'preview-img-part-2');
		cy.cGet('#preview-img-part-2').should('be.visible');
	});
});
