/* global describe it cy beforeEach require expect Cypress */

var helper = require('../../common/helper');
var calcHelper = require('../../common/calc_helper');

// Get the textviewselection messages about a view that the client received.
// The socket's _onMessage must be spied on.
function textViewSelections(win, viewId) {
	return win.app.socket._onMessage.getCalls().filter(function(call) {
		var textMsg = call.args[0] && call.args[0].textMsg;
		if (typeof textMsg !== 'string' || !textMsg.startsWith('textviewselection:'))
			return false;
		var selection = JSON.parse(textMsg.substring('textviewselection:'.length));
		return parseInt(selection.viewId) === viewId;
	});
}

describe(['tagmultiuser'], 'Local cell cursor with two views', function() {

	beforeEach(function() {
		// Turn on the local cell cursor in both views, which is off by default.
		helper.setupAndLoadDocument('calc/cell_cursor_jump.ods', true, false, undefined, 'localcellcursor=true');
	});

	it('draws the cell selection of the other view from cell ranges', function() {
		var viewId1;

		// Listen in the second view, before the first view selects.
		cy.cSetActiveFrame('#iframe2');
		cy.getFrameWindow().then(function(win2) {
			helper.processToIdle(win2);
			Cypress.sinon.spy(win2.app.socket, '_onMessage');
		});

		// Select B2:D4 in the first view.
		cy.cSetActiveFrame('#iframe1');
		cy.getFrameWindow().then(function(win1) {
			viewId1 = win1.app.map._docLayer._viewId;
			calcHelper.enterCellAddressAndConfirm(win1, 'B2:D4');
		});

		cy.cSetActiveFrame('#iframe2');
		cy.getFrameWindow().then(function(win2) {
			helper.processToIdle(win2);

			cy.wrap(win2).should(function(win) {
				// The second view should draw B2:D4 for the first view.
				var docLayer = win.app.map._docLayer;
				var expected = docLayer._cellRangeToRectangle([1, 1, 3, 3]).toArray();
				var view = win.app.activeDocument.getView(viewId1);

				// hasTextSelection is true whenever that view has at least
				// one selection drawn.
				expect(view.hasTextSelection, 'selection of the first view').to.be.true;
				expect(view.selectionRectangles).to.deep.equal([expected]);

				// The engine should not have sent the same selection as
				// rectangles via textviewselection.
				expect(textViewSelections(win, viewId1)).to.have.lengthOf(0);
			});
		});

		// Move the cursor of the first view to F6 to clear its selection.
		cy.cSetActiveFrame('#iframe1');
		cy.getFrameWindow().then(function(win1) {
			calcHelper.enterCellAddressAndConfirm(win1, 'F6');
		});

		cy.cSetActiveFrame('#iframe2');
		cy.getFrameWindow().then(function(win2) {
			helper.processToIdle(win2);

			cy.wrap(win2).should(function(win) {
				var view = win.app.activeDocument.getView(viewId1);
				expect(view.hasTextSelection, 'selection of the first view').to.be.false;
				expect(textViewSelections(win, viewId1)).to.have.lengthOf(0);
			});
		});
	});
});
