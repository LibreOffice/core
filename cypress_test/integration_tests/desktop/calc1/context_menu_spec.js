/* global describe it cy beforeEach require expect */

var helper = require('../../common/helper');

describe(['tagdesktop', 'tagnextcloud', 'tagproxy'], 'Check the correctness of context menu', function () {

    beforeEach(function () {
        helper.setupAndLoadDocument('calc/context_menu.ods');
    });

    it('Right click next to misspelled invalid word', function () {
        cy.cGet('#test-div-OwnCellCursor')
            .then(function (items) {
                expect(items).to.have.lengthOf(1);
                const clientRect = items[0].getBoundingClientRect();
                const clickX = clientRect.left + clientRect.width - 5;
                const clickY = clientRect.top + clientRect.height - 5;
                cy.cGet('body').rightclick(clickX, clickY);
            });

        // We should get the longer cell context menu, not the shorter edit context menu
        helper.getContextMenuItemList().its('length').should('be.greaterThan', 10);
    });
});
