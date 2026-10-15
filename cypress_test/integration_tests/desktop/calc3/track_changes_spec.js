/* global describe it cy beforeEach require */

var helper = require('../../common/helper');
var desktopHelper = require('../../common/desktop_helper');
var calcHelper = require('../../common/calc_helper');

describe(['tagdesktop'], 'Accept and reject tracked changes from the Calc Review tab.', function() {

	beforeEach(function() {
		// Wide enough that the Tracking group shows its buttons instead of folding them away.
		cy.viewport(1920, 1080);
		helper.setupAndLoadDocument('calc/empty-selections.ods');
		desktopHelper.switchUIToNotebookbar();
		cy.getFrameWindow().then(function(win) {
			this.win = win;
		}.bind(this));
	});

	// Record two edits, in A1 and A2, and come back to A1.
	function recordTwoEdits(win) {
		desktopHelper.selectNotebookbarTab('Review');
		desktopHelper.getNbItem('TraceChangeMode', 'Review').should('not.have.attr', 'disabled');
		desktopHelper.getNbIcon('TraceChangeMode', 'Review').click();
		helper.processToIdle(win);
		helper.waitForMapState('.uno:TraceChangeMode', 'true');

		calcHelper.enterCellAddressAndConfirm(win, 'A1');
		helper.typeIntoDocument('first{enter}second{enter}');
		calcHelper.assertAddressAfterIdle(win, 'A3');
		calcHelper.enterCellAddressAndConfirm(win, 'A1');
	}

	it('Accept and Reject act on the change at the cursor and move on to the next one', function() {
		recordTwoEdits(this.win);

		desktopHelper.getNbItem('AcceptTrackedChangeToNext', 'Review').should('not.have.attr', 'disabled');
		desktopHelper.getNbIcon('AcceptTrackedChangeToNext', 'Review').click();
		calcHelper.assertAddressAfterIdle(this.win, 'A2');
		cy.cGet('#sc_input_window.formulabar .ui-custom-textarea-text-layer').should('have.text', 'second');

		desktopHelper.getNbItem('RejectTrackedChangeToNext', 'Review').should('not.have.attr', 'disabled');
		desktopHelper.getNbIcon('RejectTrackedChangeToNext', 'Review').click();
		helper.processToIdle(this.win);
		// The rejected edit is gone from the cell.
		cy.cGet('#sc_input_window.formulabar .ui-custom-textarea-text-layer').should('have.text', '');

		// The accepted edit stays, and with nothing left to decide on, the buttons turn off.
		calcHelper.enterCellAddressAndConfirm(this.win, 'A1');
		cy.cGet('#sc_input_window.formulabar .ui-custom-textarea-text-layer').should('have.text', 'first');
		desktopHelper.getNbItem('AcceptTrackedChangeToNext', 'Review').should('have.attr', 'disabled');
		desktopHelper.getNbItem('RejectTrackedChangeToNext', 'Review').should('have.attr', 'disabled');
		desktopHelper.getNbItem('NextTrackedChange', 'Review').should('have.attr', 'disabled');
	});

	it('Next and Previous select the tracked changes in turn', function() {
		recordTwoEdits(this.win);

		desktopHelper.getNbItem('NextTrackedChange', 'Review').should('not.have.attr', 'disabled');
		desktopHelper.getNbIcon('NextTrackedChange', 'Review').click();
		calcHelper.assertAddressAfterIdle(this.win, 'A2');

		desktopHelper.getNbItem('PreviousTrackedChange', 'Review').should('not.have.attr', 'disabled');
		desktopHelper.getNbIcon('PreviousTrackedChange', 'Review').click();
		calcHelper.assertAddressAfterIdle(this.win, 'A1');
	});

	it('Reject All from the Reject menu removes every recorded edit', function() {
		recordTwoEdits(this.win);

		desktopHelper.getNbIconArrow('RejectTrackedChangeToNext', 'Review').click();
		// The menu lives in the document frame, which cy.contains cannot see into.
		cy.cGet('body').contains('.modalpopup [role="option"]', 'Reject All').click();
		helper.processToIdle(this.win);

		cy.cGet('#sc_input_window.formulabar .ui-custom-textarea-text-layer').should('have.text', '');
		calcHelper.enterCellAddressAndConfirm(this.win, 'A2');
		cy.cGet('#sc_input_window.formulabar .ui-custom-textarea-text-layer').should('have.text', '');
		desktopHelper.getNbItem('AcceptTrackedChangeToNext', 'Review').should('have.attr', 'disabled');
	});
});
