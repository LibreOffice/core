/* -*- js-indent-level: 8 -*- */
/* global describe it cy require expect beforeEach */

var helper = require('../../common/helper');

describe(['tagmultiuser'], 'Preview selection versus other views', function() {
	var win1, win2;

	beforeEach(function() {
		helper.setupAndLoadDocument('impress/preview_edit.odp', true);
		cy.getFrameWindow('#iframe1').then(function(win) { win1 = win; });
		cy.getFrameWindow('#iframe2').then(function(win) { win2 = win; });
	});

	// Click the document canvas at a twips position (in the canvas page's own
	// document space, the frame the preview edit rectangle is reported in). The
	// canvas element only ever renders the currently scrolled-into-view portion
	// of that space, so the CSS pixel offset from the canvas element's own
	// top-left corner has to subtract the current scroll position
	// (viewedRectangle.pX1/pY1, already in core pixels) before dividing down to
	// CSS pixels by dpiScale. A real click (not a bare mousedown/mouseup pair)
	// is required, since the canvas only forwards a button press to the
	// document through its 'click' handler.
	function clickCanvasAtTwips(win, x, y) {
		var viewed = win.app.activeDocument.activeLayout.viewedRectangle;
		var clientX = (x * win.app.twipsToPixels - viewed.pX1) / win.app.dpiScale;
		var clientY = (y * win.app.twipsToPixels - viewed.pY1) / win.app.dpiScale;
		cy.cGet('#document-canvas').click(clientX, clientY, { force: true });
		cy.then(() => helper.processToIdle(win));
	}

	it('Second view deleting the shape empties the first view selection', function() {
		// View 1 builds the overview page and enters the slide 1 preview.
		cy.cSetActiveFrame('#iframe1');
		cy.then(function() {
			win1.app.map.sendUnoCommand('.uno:InsertCanvasSlide');
		});
		cy.then(() => helper.processToIdle(win1));
		cy.then(function() {
			win1.app.map.sendUnoCommand('.uno:EnterPreviewEdit', {
				Part: { type: 'long', value: '1' },
			});
		});
		cy.then(() => helper.processToIdle(win1));
		cy.then(function() {
			expect(win1.app.impress.previewEditMode.active).to.equal(true);
		});

		// View 1 selects the centered shape inside the preview with a click.
		cy.then(function() {
			var rect = win1.app.impress.previewEditMode.rectangle;
			clickCanvasAtTwips(win1, rect.x1 + rect.width / 2, rect.y1 + rect.height / 2);
		});
		cy.then(function() {
			expect(win1.app.impress.previewShapeSelection).to.not.equal(null);
		});

		// View 2 stays on the slide itself, selects everything on it and deletes.
		cy.cSetActiveFrame('#iframe2');
		cy.then(function() {
			win2.app.map.setPart(1);
		});
		cy.then(() => helper.processToIdle(win2));
		// A click on the canvas moves keyboard focus onto the document itself,
		// so the delete below removes the selected shapes rather than the slide
		// (which is what a focused slide panel would do instead).
		cy.cGet('#document-canvas').click(100, 100);
		cy.then(() => helper.processToIdle(win2));
		cy.then(function() {
			win2.app.map.sendUnoCommand('.uno:SelectAll');
		});
		cy.then(() => helper.processToIdle(win2));
		helper.typeIntoDocument('{del}');
		cy.then(() => helper.processToIdle(win2));

		// View 1's virtual selection empties; its mode survives.
		cy.cSetActiveFrame('#iframe1');
		cy.then(() => helper.processToIdle(win1));
		cy.then(function() {
			expect(win1.app.impress.previewShapeSelection).to.equal(null);
			expect(win1.app.impress.previewEditMode.active).to.equal(true);
		});
	});
});
