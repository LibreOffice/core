/* global describe it cy require expect beforeEach */

// A style added in one view is registered on the shared document, so the other
// users of that document can pick it too. Each case adds a style in the first
// view by a different route and checks the second view is told about it.

var helper = require('../../common/helper');
var desktopHelper = require('../../common/desktop_helper');
var calcHelper = require('../../common/calc_helper');

describe(['tagmultiuser'], 'Calc table styles across views', function() {

	beforeEach(function() {
		helper.setupAndLoadDocument('calc/table_style_swatch.fods', true);
	});

	// Make a table in the first view so the Table Design tab and its gallery are
	// offered there.
	function insertTableInFirstView() {
		cy.cSetActiveFrame('#iframe1');
		desktopHelper.switchUIToNotebookbar();
		calcHelper.selectCellsInRange('A1:C2');
		cy.getFrameWindow().then(function(win) {
			win.app.socket.sendMessage('uno .uno:InsertCalcTable');
		});
		cy.cGet('#tablerangedialog').should('be.visible');
		cy.cGet('#tablerangedialog #ok').click();
		cy.cGet('#tablerangedialog').should('not.exist');
	}

	// Enter the shared table in the second view, which brings up its own Table
	// Design gallery and asks for the current style list.
	function countCustomStylesInSecondView(nExpected) {
		cy.cSetActiveFrame('#iframe2');
		desktopHelper.switchUIToNotebookbar();
		calcHelper.clickOnFirstCell();

		cy.getFrameWindow().its('app.map.stateChangeHandler').should(function(handler) {
			var state = handler.getItemValue('.uno:TableStyles');
			var names = (state && state.TableStyles ? state.TableStyles : []).map(function(s) { return s.Name; });
			var customs = names.filter(function(n) { return n.indexOf('TableStyleCustom') === 0; });
			expect(customs).to.have.length(nExpected);
		});

		cy.cGet('#tablestyles_design').contains('.ui-iconview-separator', 'Custom')
			.should('exist');
	}

	it('a style copied in one view reaches the other', function() {
		insertTableInFirstView();

		cy.cGet('#tablestyles_design .ui-iconview-entry:visible').eq(1).rightclick();
		cy.cGet('body').contains('Duplicate Style').should('be.visible').click();
		cy.getFrameWindow().then(function(win) {
			helper.processToIdle(win);
		});

		// The copy is in the style list the other view holds, so it is offered
		// there as well as in the view that made it.
		countCustomStylesInSecondView(1);
	});

	it('a style created in one view reaches the other', function() {
		// Creating a style goes the same way as copying one, so the other view has
		// to be told about this route too.
		insertTableInFirstView();
		cy.getFrameWindow().then(function(win) {
			helper.processToIdle(win);
		});

		cy.cGet('#tablestyles_design-iconview-list-expand-button').click();
		cy.cGet('[id^="tablestyles_design-iconview-list-dropdown"] [modelid="new-table-style"] button').click();
		cy.cGet('#NewTableStyleDialog').should('be.visible');
		cy.cGet('#NewTableStyleDialog #ok').click();
		cy.cGet('#NewTableStyleDialog').should('not.exist');
		cy.getFrameWindow().then(function(win) {
			helper.processToIdle(win);
		});

		countCustomStylesInSecondView(1);
	});
});
