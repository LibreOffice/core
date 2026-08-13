/* global describe it cy beforeEach require expect */

var helper = require('../../common/helper');
var desktopHelper = require('../../common/desktop_helper');

describe(['tagdesktop'], 'Table Design tab', function() {

	beforeEach(function() {
		helper.setupAndLoadDocument('writer/table_operation.odt');
		cy.viewport(1920, 1080);

		desktopHelper.switchUIToNotebookbar();

		cy.getFrameWindow().then(function(win) {
			this.win = win;
		});

		// The document opens with the cursor in the first table cell, but the
		// notebookbar stays in the default context, where the Table Design tab
		// is hidden. Leaving the table and coming back switches the context.
		// Ctrl+End only reaches the last cell of the table, so one more step
		// down is needed to get to the paragraph behind it. The row commands
		// are enabled only while the caret sits in a table, so their state
		// says when a move has arrived.
		helper.retryUntil(
			function() {
				helper.typeIntoDocument('{ctrl}{end}');
				helper.typeIntoDocument('{downarrow}');
			},
			function() {
				return cy.getFrameWindow().then(function(win) {
					return win.app.map['stateChangeHandler']
						.getItemValue('.uno:InsertRowsBefore') === 'disabled';
				});
			},
			{ errorMsg: 'the caret never left the table' });

		helper.retryUntil(
			function() {
				helper.typeIntoDocument('{ctrl}{home}');
			},
			function() {
				return cy.cGet('#TableDesign-tab-label').then(function(tab) {
					return !tab.hasClass('hidden');
				});
			},
			{ errorMsg: 'the Table Design tab never appeared' });
	});

	it('Applies a style clicked in the gallery', function() {
		cy.getFrameWindow().then(function(win) {
			cy.wrap(cy.stub(win.app.map, 'sendUnoCommand').callThrough()).as('sendUnoCommand');
		});

		cy.cGet('#TableDesign-tab-label').click();
		cy.cGet('#TableDesign-tab-label').should('have.class', 'selected');

		// The gallery is populated asynchronously from the engine's style
		// list, so wait for the first swatch to render before clicking it.
		cy.cGet('#table-design-styles_0').should('be.visible').click();

		cy.get('@sendUnoCommand').should(function(sendUnoCommand) {
			var call = sendUnoCommand.getCalls().find(function(c) {
				return c.args[0] === '.uno:SetTableStyle';
			});
			expect(call, '.uno:SetTableStyle was sent').to.not.be.undefined;
			expect(call.args[1].SetTableStyle.type).to.equal('string');
			expect(call.args[1].SetTableStyle.value).to.be.a('string').and.not.empty;
		});
	});

	it('Toggles a style option checkbox', function() {
		cy.getFrameWindow().then(function(win) {
			cy.wrap(cy.stub(win.app.map, 'sendUnoCommand').callThrough()).as('sendUnoCommand');
		});

		cy.cGet('#TableDesign-tab-label').click();
		cy.cGet('#TableDesign-tab-label').should('have.class', 'selected');

		cy.cGet('#table-design-header-row-input').click();

		cy.get('@sendUnoCommand').should(function(sendUnoCommand) {
			var call = sendUnoCommand.getCalls().find(function(c) {
				return c.args[0] === '.uno:TableStyleSettings';
			});
			expect(call, '.uno:TableStyleSettings was sent').to.not.be.undefined;
			expect(call.args[1].UseFirstRowStyle.type).to.equal('boolean');
		});
	});
});
