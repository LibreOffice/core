/* -*- js-indent-level: 8 -*- */
/* global describe it cy require beforeEach */

var helper = require('../../common/helper');

describe(['tagdesktop', 'tagnextcloud', 'tagproxy'], 'Slide panel accessibility', function() {

	beforeEach(function() {
		helper.setupAndLoadDocument('impress/slide_navigation.odp');
		// Wait until the panel has rendered its previews before asserting.
		cy.cGet('#slide-sorter .preview-img').should('have.length.at.least', 3);
	});

	it('exposes the slide panel as a labelled listbox of options', function() {
		cy.cGet('#slide-sorter').should('have.attr', 'role', 'listbox');
		cy.cGet('#slide-sorter').should('have.attr', 'aria-multiselectable', 'true');
		// A presentation has slides, so the list is named "Slides".
		cy.cGet('#slide-sorter').should('have.attr', 'aria-label', 'Slides');

		// The reorder drop target is not a slide, so it is hidden from the list.
		cy.cGet('#first-drop-site').should('have.attr', 'aria-hidden', 'true');

		// Every preview is an option that carries a selected state.
		cy.cGet('#preview-img-part-0').should('have.attr', 'role', 'option');
		cy.cGet('#preview-img-part-1').should('have.attr', 'role', 'option');
		cy.cGet('#preview-img-part-2').should('have.attr', 'role', 'option');
	});

	it('marks the current slide selected and as the single tab stop', function() {
		// The first slide is current on load: selected and the only tab stop.
		cy.cGet('#preview-img-part-0').should('have.attr', 'aria-selected', 'true');
		cy.cGet('#preview-img-part-0').should('have.attr', 'tabindex', '0');

		// Every other slide is unselected and kept out of the tab order.
		cy.cGet('#preview-img-part-1').should('have.attr', 'aria-selected', 'false');
		cy.cGet('#preview-img-part-1').should('have.attr', 'tabindex', '-1');

		cy.cGet('#preview-img-part-2').should('have.attr', 'aria-selected', 'false');
		cy.cGet('#preview-img-part-2').should('have.attr', 'tabindex', '-1');
	});

	it('moves the selected state and the tab stop with arrow navigation', function() {
		cy.cGet('#preview-img-part-0').click();

		cy.cGet('#preview-img-part-0').type('{downarrow}');
		cy.wait(500);
		helper.assertFocus('id', 'preview-img-part-1');

		// The selected state and the roving tab stop follow the current slide.
		cy.cGet('#preview-img-part-1').should('have.attr', 'aria-selected', 'true');
		cy.cGet('#preview-img-part-1').should('have.attr', 'tabindex', '0');

		// The slide left behind is no longer selected nor a tab stop.
		cy.cGet('#preview-img-part-0').should('have.attr', 'aria-selected', 'false');
		cy.cGet('#preview-img-part-0').should('have.attr', 'tabindex', '-1');
	});
});
