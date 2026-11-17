/* -*- js-indent-level: 8 -*- */
/* global describe it cy require expect beforeEach */

var helper = require('../../common/helper');

// A read-only presentation shows all slides stacked one under the other. The first zoom fits
// one slide in the window, the same as in an editable view, and the reader scrolls down
// through the rest.
describe(['tagdesktop'], 'Impress: zoom of a read-only presentation', function() {

	beforeEach(function() {
		cy.viewport(1280, 800);
		var filePath = helper.setupDocument('impress/slide_navigation.odp');
		helper.loadDocument(filePath, true, undefined, undefined, 'permission=readonly');
		cy.getFrameWindow().then(function(win) {
			this.win = win;
		}.bind(this));
	});

	it('opens with one whole slide filling the window', function() {
		cy.cGet('#document-canvas').should('be.visible');
		helper.processToIdle(this.win);

		cy.getFrameWindow().should(function(win) {
			var docLayer = win.app.map._docLayer;
			var frame = win.app.activeDocument.activeLayout.frameSize;
			var slideWidth = win.app.activeDocument.fileSize.pX;
			var slideHeight = docLayer._partHeightTwips * win.app.twipsToPixels;
			var zoomPercent = win.app.activeDocument.getZoomPercent();

			// The measured view, repeated in each message so a failure explains itself.
			var summary = 'one slide shows at ' + Math.round(slideWidth) + ' x '
				+ Math.round(slideHeight) + ' px in a ' + frame.pX + ' x ' + frame.pY
				+ ' px window, at ' + zoomPercent + '% zoom, slide '
				+ (docLayer._selectedPart + 1) + ' of ' + docLayer._parts;

			expect(win.app.file.fileBasedView,
				'precondition: all slides are shown in one stack').to.equal(true);
			expect(win.app.activeDocument.fileSize.pY,
				'precondition: the stack of slides is taller than the window')
				.to.be.greaterThan(frame.pY);

			var widthShare = slideWidth / frame.pX;
			var heightShare = slideHeight / frame.pY;
			var filledPercent = Math.round(Math.max(widthShare, heightShare) * 100);
			expect(filledPercent, 'percent of the window that one slide fills; ' + summary)
				.to.be.greaterThan(60);
			expect(zoomPercent, 'zoom in percent, 20 is the lowest zoom; ' + summary)
				.to.be.greaterThan(20);
			expect(slideWidth, 'one slide is wider than the window; ' + summary)
				.to.be.at.most(frame.pX);
			expect(slideHeight, 'one slide is taller than the window; ' + summary)
				.to.be.at.most(frame.pY);
		});
	});
});
