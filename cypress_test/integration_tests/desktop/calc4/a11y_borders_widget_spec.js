/* global describe expect it cy before require */

const helper = require('../../common/helper');
const calcHelper = require('../../common/calc_helper');

// A screen reader announces the name of the focused element when focus arrives on it and
// announces it again when the focused element is replaced, so the borders widget keeps its
// element and its name while the user toggles borders from the keyboard.
describe(['tagdesktop'], 'Borders widget', { testIsolation: false }, function () {
	let win;

	before(function () {
		helper.setupAndLoadDocument('calc/switch.ods');

		cy.getFrameWindow().then(function (frameWindow) { win = frameWindow; });

		calcHelper.dblClickOnFirstCell();
		cy.then(function () { win.app.map.sendUnoCommand('.uno:FormatCellDialog'); });
		cy.cGet('.ui-dialog[role="dialog"]').should('have.length', 1);
		cy.then(function () { return helper.processToIdle(win); });
		cy.cGet('.ui-dialog[role="dialog"]').contains('[role="tab"]', 'Border').click();
		cy.then(function () { return helper.processToIdle(win); });
	});

	it('keeps its element and its name while Space toggles a border', function () {
		let widget;

		cy.cGet('#framesel-img').should('have.attr', 'role', 'application').then(function (found) {
			widget = found[0];
			widget.focus();
		});
		cy.then(function () { return helper.processToIdle(win); });

		cy.cGet('#framesel-img').should('have.attr', 'aria-activedescendant').then(function (id) {
			cy.cGet('#' + id).should('have.attr', 'aria-checked', 'false');
		});

		cy.realPress('Space');
		cy.then(function () { return helper.processToIdle(win); });

		cy.cGet('#framesel-img').then(function (found) {
			expect(found[0], 'the widget is the element that had focus').to.equal(widget);
			expect(win.document.activeElement, 'the widget still has focus').to.equal(widget);
		});
		cy.cGet('#framesel-img').should('have.attr', 'aria-labelledby');
		cy.cGet('#framesel-img').invoke('attr', 'aria-activedescendant').then(function (id) {
			cy.cGet('#' + id).should('have.attr', 'aria-checked', 'true');
		});
	});
});
