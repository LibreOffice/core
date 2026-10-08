/* global describe it cy beforeEach require expect */

var helper = require('../../common/helper');

describe(['tagdesktop'], 'Menubar refresh', function () {
        beforeEach(function () {
                // A read-only view has no notebookbar, which the usual load checks wait for.
                var filePath = helper.setupDocument('calc/focus.ods');
                helper.loadDocument(filePath, true, undefined, undefined, 'permission=readonly');
                cy.getFrameWindow().should((win) => {
                        expect(win.app.map._docLoaded, 'document loaded').to.equal(true);
                        this.win = win;
                });
                cy.then(() => helper.processToIdle(this.win));
        });

        // Every menubar refresh initialized SmartMenus again on #main-menu without
        // removing the previous instance, so the click handlers piled up and one
        // click on Help > About could open the dialog twice.
        it('does not stack click handlers on the menubar', function () {
                cy.cGet('#main-menu').should('exist');
                cy.then(() => {
                        var win = this.win;
                        var count = () => win.$._data(
                                win.document.getElementById('main-menu'), 'events').click.length;
                        var before = count();
                        win.app.map.menubar.refresh();
                        win.app.map.menubar.refresh();
                        expect(count(), 'click handlers after two refreshes').to.equal(before);
                });
        });
});