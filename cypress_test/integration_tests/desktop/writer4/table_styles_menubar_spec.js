/* global describe it cy beforeEach require expect */

var helper = require('../../common/helper');
var desktopHelper = require('../../common/desktop_helper');

describe(['tagdesktop'], 'Table styles from the menu bar', function() {

	beforeEach(function() {
		cy.viewport(1920, 1080);
		helper.setupAndLoadDocument('writer/table_operation.odt');
		desktopHelper.switchUIToCompact();

		cy.getFrameWindow().then(function(win) {
			this.win = win;
		});
	});

	// The menu entry follows the state of Table Properties next to it.
	function tableDialogState() {
		return cy.getFrameWindow().then(function(win) {
			return win.app.map['stateChangeHandler'].getItemValue('.uno:TableDialog');
		});
	}

	// The document opens with the caret in the first table cell.
	function waitForTable() {
		cy.getFrameWindow().should(function(win) {
			expect(win.app.map['stateChangeHandler'].getItemValue('.uno:TableDialog'))
				.to.equal('enabled');
		});
	}

	// Ctrl+End only reaches the last cell, so one more step down gets to the paragraph behind
	// the table.
	function leaveTable() {
		helper.retryUntil(
			function() {
				helper.typeIntoDocument('{ctrl}{end}');
				helper.typeIntoDocument('{downarrow}');
			},
			function() {
				return tableDialogState().then(function(state) {
					return state === 'disabled';
				});
			},
			{ errorMsg: 'the caret never left the table' });
	}

	function openDialog() {
		cy.cGet('#menu-table').click();
		cy.cGet('#menu-tablestyles > a').should('not.have.class', 'disabled');
		cy.cGet('#menu-tablestyles').click();
		cy.cGet('#TableStylesDialog-mainbox').should('be.visible');
	}

	function setTableStyleCalls(sendUnoCommand) {
		return sendUnoCommand.getCalls().filter(function(c) {
			return c.args[0] === '.uno:SetTableStyle';
		});
	}

	it('Table Styles in the Table menu opens a dialog and OK applies the picked style', function() {
		waitForTable();
		cy.then(function() {
			return helper.processToIdle(this.win);
		});
		cy.wrap(cy.stub(this.win.app.map, 'sendUnoCommand').callThrough()).as('sendUnoCommand');

		openDialog();
		cy.cGet('#TableStylesDialog #table-design-styles_0').click();
		cy.cGet('#TableStylesDialog #table-design-styles_0').should('have.class', 'selected');
		// A click only picks the style.
		cy.get('@sendUnoCommand').then(function(sendUnoCommand) {
			expect(setTableStyleCalls(sendUnoCommand)).to.have.length(0);
		});

		cy.cGet('#TableStylesDialog #ok').click();
		cy.cGet('#TableStylesDialog').should('not.exist');
		cy.get('@sendUnoCommand').should(function(sendUnoCommand) {
			var calls = setTableStyleCalls(sendUnoCommand);
			expect(calls, '.uno:SetTableStyle was sent').to.have.length(1);
			expect(calls[0].args[1].SetTableStyle.value).to.be.a('string').and.not.empty;
		});
	});

	it('Cancel in the Table Styles dialog applies nothing', function() {
		waitForTable();
		cy.then(function() {
			return helper.processToIdle(this.win);
		});
		cy.wrap(cy.stub(this.win.app.map, 'sendUnoCommand').callThrough()).as('sendUnoCommand');

		openDialog();
		cy.cGet('#TableStylesDialog #table-design-styles_0').click();
		cy.cGet('#TableStylesDialog #table-design-styles_0').should('have.class', 'selected');
		cy.cGet('#TableStylesDialog #cancel').click();
		cy.cGet('#TableStylesDialog').should('not.exist');
		cy.get('@sendUnoCommand').then(function(sendUnoCommand) {
			expect(setTableStyleCalls(sendUnoCommand)).to.have.length(0);
		});
	});

	it('Table Styles in the Table menu is disabled outside a table', function() {
		leaveTable();

		cy.cGet('#menu-table').click();
		cy.cGet('#menu-tablestyles > a').should('have.class', 'disabled');
	});
});
