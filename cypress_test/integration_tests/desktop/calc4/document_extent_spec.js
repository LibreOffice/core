/* global describe it cy require beforeEach expect */

var helper = require('../../common/helper');
var desktopHelper = require('../../common/desktop_helper');

describe(['tagdesktop'], 'Calc document extent', { testIsolation: false }, function() {

	desktopHelper.shareDocumentAcrossTests('calc/empty-selections.ods');

	beforeEach(function() {
		cy.getFrameWindow().then((win) => {
			this.win = win;
		});
	});

	function documentSpansTheWindow(win, label) {
		cy.cGet('#document-canvas').should(() => {
			const layout = win.app.activeDocument.activeLayout;

			expect(layout.viewSize.pX, label + ': scrollable width against the frame')
				.to.be.at.least(layout.frameSize.pX);
			expect(layout.viewSize.pY, label + ': scrollable height against the frame')
				.to.be.at.least(layout.frameSize.pY);
		});
	}

	// A sheet with no data in it still has to fill the window: zooming out shows
	// more cells, so the scrollable area has to cover at least what is on
	// screen. Before the fix an empty sheet kept the extent it was given at 100
	// percent, and zooming out squeezed the cells into a small box in the corner
	// of the window.
	it('An empty sheet spans the window when zoomed out', function() {
		desktopHelper.selectZoomLevel('20');
		helper.processToIdle(this.win);

		documentSpansTheWindow(this.win, 'empty sheet');
	});

	// The last used column and row are counted from zero, so a sheet whose data
	// ends in column A reports the same zero as an empty sheet does and used to
	// keep the same stale extent. Five cells in the first column are enough for
	// the sheet to have data while the last column stays A.
	it('A sheet with data only in the first column spans the window when zoomed out', function() {
		desktopHelper.resetZoomLevel();
		helper.processToIdle(this.win);

		helper.typeIntoInputField(helper.addressInputSelector, 'A1');
		helper.typeIntoDocument('1{enter}2{enter}3{enter}4{enter}5{enter}');
		helper.processToIdle(this.win);

		desktopHelper.selectZoomLevel('20');
		helper.processToIdle(this.win);

		documentSpansTheWindow(this.win, 'first column only');
	});
});
