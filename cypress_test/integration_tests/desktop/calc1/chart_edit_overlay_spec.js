/* global describe it cy beforeEach require expect */

var calcHelper = require('../../common/calc_helper');
var helper = require('../../common/helper');

// The share of the document area that is not plain white. The dark overlay of
// chart edit mode covers the whole sheet around the chart, so it turns most of
// the area grey, while an empty grid with a few numbers stays almost all white.
function readShadedFraction(win) {
	var anchor = win.app.sectionContainer.getDocumentAnchorSection();
	var canvas = win.document.getElementById('document-canvas');
	var data = canvas.getContext('2d')
		.getImageData(anchor.myTopLeft[0], anchor.myTopLeft[1], anchor.size[0], anchor.size[1]).data;

	var shaded = 0;
	for (var i = 0; i < data.length; i += 4) {
		if (data[i] < 240 || data[i + 1] < 240 || data[i + 2] < 240)
			shaded++;
	}
	return shaded / (data.length / 4);
}

describe(['tagdesktop'], 'Chart edit mode dark overlay', function() {
	beforeEach(function() {
		helper.setupAndLoadDocument('calc/empty-selections.ods');
		cy.viewport(1920, 720);
	});

	function insertChartFromColumn(firstRow) {
		calcHelper.selectCellsInRange('A' + firstRow);
		helper.typeIntoDocument('1{enter}2{enter}3{enter}4{enter}5{enter}6{enter}');
		calcHelper.selectCellsInRange('A' + firstRow + ':A' + (firstRow + 5));

		cy.cGet('#Insert-tab-label').click();
		cy.cGet('#Insert .unoInsertObjectChart').eq(0).click();
		cy.cGet('.lokdialog_container').should('be.visible');
		cy.cGet('.lokdialog_container #finish').click();
		cy.cGet('.lokdialog_container').should('not.exist');
	}

	function assertOverlayAppearsOnEdit() {
		cy.getFrameWindow().then(function(win) { helper.processToIdle(win); });
		cy.getFrameWindow().should(function(win) {
			expect(readShadedFraction(win)).to.be.lessThan(0.2);
		});

		// The inserted chart is selected. Enter starts in-place editing.
		helper.typeIntoDocument('{enter}');
		cy.getFrameWindow().then(function(win) { helper.processToIdle(win); });
		cy.getFrameWindow().should(function(win) {
			expect(readShadedFraction(win)).to.be.greaterThan(0.5);
		});
	}

	it('overlay covers the sheet around a chart at the top', function() {
		insertChartFromColumn(1);
		assertOverlayAppearsOnEdit();
	});

	// Far down the sheet the view is scrolled a long way. The overlay has to be
	// drawn relative to the scrolled viewport, or it lands off screen.
	it('overlay covers the sheet around a chart at row 500', function() {
		insertChartFromColumn(500);
		assertOverlayAppearsOnEdit();
	});

	// A zoom animation scales the tiles frame by frame. The overlay holds the
	// chart in pixels of the zoom it started from, and each frame has to scale
	// them the same way, so the chart stays the one area that is not dimmed.
	it('chart stays undimmed while the zoom animates', function() {
		insertChartFromColumn(500);
		assertOverlayAppearsOnEdit();

		// Cypress runs normally commit a zoom at once. Turn the animation back
		// on, stretch it, and zoom one level in.
		cy.getFrameWindow().then(function(win) {
			win.L.Browser.cypressTest = false;
			win.app.zoomControl.zoomDurationMs = 8000;
			win.app.zoomControl.zoomTo(win.app.map.getZoom() + 1);
		});
		cy.wait(3000);

		// Mid-animation the dimming covers the sheet but leaves the chart alone,
		// so a clear share of the document area is still white.
		cy.getFrameWindow().then(function(win) {
			var shaded = readShadedFraction(win);
			expect(shaded).to.be.greaterThan(0.5);
			expect(shaded).to.be.lessThan(0.95);
		});

		cy.wait(6000);
		cy.getFrameWindow().then(function(win) {
			win.L.Browser.cypressTest = true;
			expect(readShadedFraction(win)).to.be.greaterThan(0.5);
			expect(readShadedFraction(win)).to.be.lessThan(0.95);
		});
	});
});
