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

    it('Slide change emits Doc_PartChanged postMessage', function() {
        cy.getFrameWindow().then(win => {
            cy.stub(win.parent, 'postMessage').as('postMessage');
        });

        cy.cGet('#preview-img-part-1').click();

        cy.get('@postMessage').should(stub => {
            const found = stub.getCalls().some(call => {
                const msg = JSON.parse(call.args[0]);
                return msg.MessageId === 'Doc_PartChanged'
                    && msg.Values
                    && msg.Values.Part === 2
                    && Number.isInteger(msg.Values.PartCount)
                    && msg.Values.PartCount >= 2
                    && msg.Values.DocType === 'presentation';
            });

            expect(found, 'Doc_PartChanged was not posted for slide change').to.be.true;
        });
    });
});
