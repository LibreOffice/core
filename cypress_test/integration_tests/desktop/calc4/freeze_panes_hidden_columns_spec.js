/* global describe it cy require expect */

var helper = require('../../common/helper');

// cool#13295: "Freeze Rows and Columns" froze no columns at all when columns
// before the cursor were hidden first. Core's freeze-at-cursor handler
// (ScTabViewShell::Execute, SID_WINDOW_FIX) correctly set the freeze index to
// the cursor's column, but the browser then converted that index to a pixel
// position and back to an index (to compute the split divider's screen
// position) and re-sent the resulting index to core. At an exact span
// boundary created by a run of hidden columns, that round trip rounded down
// to the count of leading hidden columns, silently overwriting the correct
// freeze index core had just reported.
describe(['tagdesktop'], 'Calc: freeze columns with leading hidden columns (cool#13295)', function() {

	it('freezes at the cursor column, not at the number of hidden columns before it', function() {
		helper.setupAndLoadDocument('calc/empty-selections.ods');
		cy.getFrameWindow().then(function(win) {
			var sg = win.app.map._docLayer.sheetGeometry;
			var colELeftBeforeHide = sg.getCellRect(4, 0).min.x;

			// Hide the first three columns (A-C).
			helper.typeIntoInputField(helper.addressInputSelector, 'A1:C1');
			cy.then(function() {
				win.app.map.sendUnoCommand('.uno:HideColumn');
				return helper.processToIdle(win);
			});

			// The geometry update from HideColumn can arrive after the command's
			// own round trip, so poll until column E has actually moved left.
			cy.wrap(sg).should(function(sgLive) {
				expect(sgLive.getCellRect(4, 0).min.x).to.be.lessThan(colELeftBeforeHide);
			});
			// A wrong follow-up correction, if the client sends one, goes out while it handles
			// core's reply. The second idle round trip makes core process that correction and
			// report the result before the test moves on.
			cy.then(function() {
				return helper.processToIdle(win);
			});
			cy.then(function() {
				return helper.processToIdle(win);
			});

			// Select E2 (column index 4, row index 1), matching the issue's
			// "select column 5, row 2", and freeze there.
			helper.typeIntoInputField(helper.addressInputSelector, 'E2');
			cy.then(function() {
				win.app.map.sendUnoCommand('.uno:FreezePanes');
				return helper.processToIdle(win);
			});
			// Same reasoning as above: one more idle round trip lets a wrong follow-up
			// correction, if any, reach core and come back before the assertion.
			cy.then(function() {
				return helper.processToIdle(win);
			});

			// Without the fix, this settled on splitCell.x === 3 (the count of
			// leading hidden columns) instead of 4 (the cursor's own column).
			cy.wrap(win.app.map._docLayer._splitPanesContext).should(function(ctx) {
				expect(ctx._splitCell.x, 'frozen column index').to.eq(4);
				expect(ctx._splitCell.y, 'frozen row index').to.eq(1);
			});
		});
	});
});
