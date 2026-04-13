/* -*- js-indent-level: 8 -*- */
/* global describe it cy require beforeEach*/

var helper = require('../../common/helper');

describe(['tagdesktop', 'tagnextcloud', 'tagproxy'], 'Slide Navigation', function() {

    beforeEach(function() {
        helper.setupAndLoadDocument('impress/slide_navigation.odp');
    });

    it('Arrow navigation', function() {
        cy.cGet('#preview-img-part-0').click();

        cy.cGet('#preview-img-part-0').type('{downarrow}');
        cy.wait(500);
        helper.assertFocus('id', 'preview-img-part-1');

        cy.cGet('#preview-img-part-1').type('{downarrow}');
        cy.wait(500);
        helper.assertFocus('id', 'preview-img-part-2');
        cy.cGet('#preview-img-part-2').should('have.class', 'preview-img-currentpart');

        cy.cGet('#preview-img-part-2').type('{uparrow}');
        cy.wait(500);
        helper.assertFocus('id', 'preview-img-part-1');

        cy.cGet('#preview-img-part-1').type('{uparrow}');
        cy.wait(500);
        helper.assertFocus('id', 'preview-img-part-0');
        cy.cGet('#preview-img-part-0').should('have.class', 'preview-img-currentpart');
    });
});
