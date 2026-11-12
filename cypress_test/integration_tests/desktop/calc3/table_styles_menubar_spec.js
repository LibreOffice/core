/* global describe it cy beforeEach require expect */

var helper = require('../../common/helper');
var desktopHelper = require('../../common/desktop_helper');
var calcHelper = require('../../common/calc_helper');

describe(['tagdesktop'], 'Calc table styles from the menu bar', function() {
	beforeEach(function() {
		cy.viewport(1920, 1080);
		helper.setupAndLoadDocument('calc/testfile.xlsx');
		desktopHelper.switchUIToCompact();
		cy.getFrameWindow().then(function(win) {
			this.win = win;
		});
	});

	it('Data > Table > Table Styles opens a dialog and OK applies the picked style', function() {
		var win = this.win;

		calcHelper.selectCellsInRange('A1:C3');
		cy.then(function() {
			win.app.socket.sendMessage('uno .uno:InsertCalcTable');
		});
		cy.cGet('#tablerangedialog').should('be.visible');
		cy.cGet('#tablerangedialog #ok').click();
		cy.cGet('#tablerangedialog').should('not.exist');
		// The menu entry follows the state of Remove Table next to it.
		cy.getFrameWindow().should(function(frameWindow) {
			expect(frameWindow.app.map['stateChangeHandler'].getItemValue('.uno:RemoveCalcTable'))
				.to.equal('enabled');
		});
		cy.then(function() {
			return helper.processToIdle(win);
		});

		cy.wrap(cy.stub(win.app.map, 'sendUnoCommand').callThrough()).as('sendUnoCommand');

		cy.cGet('#menu-data').click();
		// Open the submenu that holds the entry, which is Data > Table.
		cy.cGet('#menu-data #menu-tablestyles').parent('ul').parent('li')
			.children('a.has-submenu').click();
		cy.cGet('#menu-tablestyles > a').should('not.have.class', 'disabled');
		cy.cGet('#menu-tablestyles').click();

		cy.cGet('#TableStylesDialog-mainbox').should('be.visible');
		cy.cGet('#TableStylesDialog #tablestyles_design .ui-iconview-entry')
			.should('have.length.greaterThan', 10);
		cy.cGet('#TableStylesDialog #new').should('exist');
		cy.cGet('#TableStylesDialog #clear').should('exist');

		cy.cGet('#TableStylesDialog #tablestyles_design_1').click();
		cy.cGet('#TableStylesDialog #tablestyles_design_1').should('have.class', 'selected');
		cy.cGet('#TableStylesDialog #ok').click();

		cy.cGet('#TableStylesDialog').should('not.exist');
		cy.get('@sendUnoCommand').should(function(sendUnoCommand) {
			var call = sendUnoCommand.getCalls().find(function(c) {
				return c.args[0] === '.uno:DatabaseSettings';
			});
			expect(call, '.uno:DatabaseSettings was sent').to.not.be.undefined;
		});
	});
});
