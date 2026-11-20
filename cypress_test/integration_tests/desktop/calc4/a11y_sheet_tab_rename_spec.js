/* global describe it cy before expect require */

const helper = require('../../common/helper');

// Renaming a sheet was a pointer-only gesture: double-click the tab, or reach
// the context menu with the mouse. F2 on the focused tab is the keyboard's way
// in, so the dialog is checked from the tab, not from the menu.
describe(['tagdesktop'], 'Sheet tab rename by keyboard', { testIsolation: false }, function () {
	let win;

	before(function () {
		cy.viewport(1920, 1080);
		helper.setupAndLoadDocument('calc/switch.ods', false, false, undefined, 'vector=false');
		cy.getFrameWindow().then(function (frameWindow) { win = frameWindow; });
		cy.then(function () { return helper.processToIdle(win); });
	});

	function tabs() {
		return win.document.querySelectorAll('#spreadsheet-tab-scroll .spreadsheet-tab');
	}

	it('F2 on the focused tab opens the rename dialog for that sheet', function () {
		cy.then(function () {
			const tab = tabs()[0];
			expect(tab, 'the sheet tab bar has a tab').to.not.be.undefined;
			tab.focus();
			expect(win.document.activeElement.id, 'the tab took the focus').to.equal(tab.id);
		});

		cy.realPress('F2');

		cy.cGet('#rename-calc-sheet').should('be.visible');

		// The dialog names the sheet the focus was on, not whichever is current.
		cy.then(function () {
			const name = tabs()[0].innerText.trim();
			cy.cGet('#rename-calc-sheet input').should('have.value', name);
		});
	});

	it('and the new name reaches the tab', function () {
		cy.cGet('#rename-calc-sheet input').clear().type('Renamed');
		cy.cGet('#rename-calc-sheet .ui-pushbutton.jsdialog.button-primary').click();
		cy.then(function () { return helper.processToIdle(win); });

		cy.cGet('#spreadsheet-tab0').should('have.text', 'Renamed');
	});
});
