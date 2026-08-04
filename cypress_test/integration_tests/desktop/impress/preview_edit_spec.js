/* -*- js-indent-level: 8 -*- */
/* global describe it cy require expect beforeEach */

var helper = require('../../common/helper');

describe(['tagdesktop'], 'Edit slides through canvas previews', { testIsolation: false }, function() {

	var desktopHelper = require('../../common/desktop_helper');
	desktopHelper.shareDocumentAcrossTests('impress/preview_edit.odp', {
		notebookbar: true,
	});

	beforeEach(function() {
		cy.getFrameWindow().then((win) => {
			this.win = win;
		});
	});

	function enterPreviewEdit(win, part) {
		cy.then(function() {
			win.app.map.sendUnoCommand('.uno:InsertCanvasSlide');
		});
		cy.then(() => helper.processToIdle(win));
		cy.then(function() {
			expect(win.app.impress.hasOverviewPage).to.equal(true);
		});
		cy.then(function() {
			win.app.map.sendUnoCommand('.uno:EnterPreviewEdit', {
				Part: { type: 'long', value: String(part) },
			});
		});
		cy.then(() => helper.processToIdle(win));
		cy.then(function() {
			expect(win.app.impress.previewEditMode.active).to.equal(true);
			expect(win.app.impress.previewEditMode.part).to.equal(part);
		});
		// The overlay section draws the entered-preview highlight. It has to be
		// visible right away, without waiting for a scroll to place it.
		cy.then(function() {
			const section = win.app.sectionContainer.getSectionWithName('previewEdit');
			expect(section).to.exist;
			expect(section.isVisible).to.equal(true);
		});
	}

	// Click the document canvas at a twips position (in the canvas page's own
	// document space, the frame the preview edit rectangle is reported in). The
	// canvas element only ever renders the currently scrolled-into-view portion
	// of that space, so the CSS pixel offset from the canvas element's own
	// top-left corner has to subtract the current scroll position
	// (viewedRectangle.pX1/pY1, already in core pixels) before dividing down to
	// CSS pixels by dpiScale.
	//
	// A real browser click (not a bare mousedown/mouseup pair) is required: the
	// canvas only forwards a button press to the document through its 'click'
	// handler (MouseControl.onClick), the same as the existing
	// impress/delete_objects_spec.js canvas-click helper uses.
	function clickCanvasAtTwips(win, x, y) {
		const viewed = win.app.activeDocument.activeLayout.viewedRectangle;
		const clientX = (x * win.app.twipsToPixels - viewed.pX1) / win.app.dpiScale;
		const clientY = (y * win.app.twipsToPixels - viewed.pY1) / win.app.dpiScale;
		cy.cGet('#document-canvas').click(clientX, clientY, { force: true });
		cy.then(() => helper.processToIdle(win));
	}

	it('Insert a shape into a slide through its preview', function() {
		enterPreviewEdit(this.win, 1);
		const win = this.win;

		cy.then(function() {
			const mode = win.app.impress.previewEditMode;
			win.app.map.sendUnoCommand('.uno:InsertShapeIntoSlide', {
				ShapeType: { type: 'string', value: 'rectangle' },
				Part: { type: 'long', value: String(mode.part) },
				X: { type: 'long', value: String(Math.round(mode.slideWidth / 4)) },
				Y: { type: 'long', value: String(Math.round(mode.slideHeight / 4)) },
				Width: { type: 'long', value: String(Math.round(mode.slideWidth / 5)) },
				Height: { type: 'long', value: String(Math.round(mode.slideHeight / 5)) },
			});
		});
		cy.then(() => helper.processToIdle(win));

		// Switch to the slide and check the shape is there: select all and expect a
		// graphic selection to appear.
		cy.then(function() {
			win.app.map.setPart(1);
		});
		cy.then(() => helper.processToIdle(win));
		cy.then(function() {
			win.app.map.sendUnoCommand('.uno:SelectAll');
		});
		cy.then(() => helper.processToIdle(win));
		cy.then(function() {
			expect(win.app.definitions.graphicSelection.hasActiveSelection()).to.equal(true);
		});
		// Back to the overview for the next test.
		cy.then(function() {
			win.app.map.setPart(0);
		});
		cy.then(() => helper.processToIdle(win));
	});

	it('Select a shape inside the preview and delete it', function() {
		enterPreviewEdit(this.win, 1);
		const win = this.win;

		// Click the center of the entered preview; the sample slides carry a centered
		// content shape there.
		cy.then(function() {
			const rect = win.app.impress.previewEditMode.rectangle;
			clickCanvasAtTwips(win, rect.x1 + rect.width / 2, rect.y1 + rect.height / 2);
		});
		cy.then(function() {
			expect(win.app.impress.previewShapeSelection).to.not.equal(null);
		});

		helper.typeIntoDocument('{del}');
		cy.then(() => helper.processToIdle(win));
		cy.then(function() {
			expect(win.app.impress.previewShapeSelection).to.equal(null);
			// The mode survives the delete.
			expect(win.app.impress.previewEditMode.active).to.equal(true);
		});
	});

	it('Escape leaves the mode', function() {
		enterPreviewEdit(this.win, 1);
		const win = this.win;
		helper.typeIntoDocument('{esc}');
		cy.then(() => helper.processToIdle(win));
		cy.then(function() {
			expect(win.app.impress.previewEditMode.active).to.equal(false);
		});
	});
});
