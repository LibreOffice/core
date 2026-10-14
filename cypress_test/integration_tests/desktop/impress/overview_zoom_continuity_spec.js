/* global describe it cy require expect beforeEach */

describe(['tagdesktop'], 'Overview zoom continuity', { testIsolation: false }, function() {
	var helper = require('../../common/helper');
	var desktopHelper = require('../../common/desktop_helper');

	desktopHelper.shareDocumentAcrossTests('impress/overview_gestures.odp', {
		notebookbar: true,
	});

	beforeEach(function() {
		cy.getFrameWindow().then((win) => { this.win = win; });
	});

	// Inserts the overview page when missing. Part 0 is the overview page, parts 1 and 2
	// are the real slides of the fixture.
	function ensureOverviewPage(win) {
		cy.then(function() {
			if (!win.app.impress.hasOverviewPage)
				win.app.map.sendUnoCommand('.uno:InsertCanvasSlide');
		});
		cy.then(() => helper.processToIdle(win));
		cy.then(function() {
			expect(win.app.impress.hasOverviewPage).to.equal(true);
			expect(win.app.impress.previewRects).to.not.equal(null);
		});
	}

	function goToPart(win, part) {
		cy.then(function() { win.app.map.setPart(part); });
		cy.then(() => helper.processToIdle(win));
	}

	function previewRect(win, part) {
		return win.app.impress.previewRects.find((p) => p.part === part).rectangle;
	}

	// Converts canvas-page twips to client (CSS) coordinates for cy events on the
	// document container.
	function toClient(win, x, y) {
		const viewed = win.app.activeDocument.activeLayout.viewedRectangle;
		return {
			x: (x * win.app.twipsToPixels - viewed.pX1) / win.app.dpiScale,
			y: (y * win.app.twipsToPixels - viewed.pY1) / win.app.dpiScale,
		};
	}

	// Passes when the preview of the given part is centered in the visible area (within
	// 5% of the viewport on each axis) and fills most of the tighter viewport axis. The
	// integer zoom steps make the exact fill vary, so the fill check is a range; when the
	// wanted fill would need a zoom above the 400% ceiling, the clamped zoom is accepted
	// instead of the fill.
	function assertPreviewCentered(win, part, minFill, maxFill) {
		cy.then(() => helper.retryUntil(
			() => helper.processToIdle(win),
			() => {
				const rect = previewRect(win, part);
				const viewed = win.app.activeDocument.activeLayout.viewedRectangle;
				const dx = Math.abs(rect.center[0] - viewed.center[0]);
				const dy = Math.abs(rect.center[1] - viewed.center[1]);
				if (dx > viewed.width * 0.05 || dy > viewed.height * 0.05)
					return false;
				if (win.app.map.getZoom() >= win.app.map.getMaxZoom())
					return true;
				const fill = Math.max(rect.width / viewed.width,
					rect.height / viewed.height);
				return fill >= minFill && fill <= maxFill;
			},
			{ errorMsg: 'preview ' + part + ' never centered and filled the view' }
		));
	}

	it('Fit slide to view command centers the given preview', function() {
		const win = this.win;
		ensureOverviewPage(win);
		goToPart(win, 0);
		cy.then(function() {
			win.app.dispatcher.dispatch('fitslidetoview', { part: 2 });
		});
		// Fill target is 0.9; one 1.2 zoom step below is 0.75, so accept 0.6..0.95.
		assertPreviewCentered(win, 2, 0.6, 0.95);
	});

	it('Zoom to Fit Slide statusbar button fits the nearest preview', function() {
		const win = this.win;
		ensureOverviewPage(win);
		goToPart(win, 0);
		cy.then(function() {
			win.app.dispatcher.dispatch('fitslidetoview', { part: 1 });
		});
		assertPreviewCentered(win, 1, 0.6, 0.95);
		// Zoom out two steps; zoomTo without an anchor keeps the view center, so
		// preview 1 stays the nearest one but no longer fills the view.
		cy.then(function() {
			win.app.zoomControl.zoomTo(win.app.map.getZoom() - 2, undefined, false);
		});
		cy.then(() => helper.processToIdle(win));
		desktopHelper.makeZoomItemsVisible();
		cy.cGet('#toolbar-down #fitwidthzoom-impress').click();
		assertPreviewCentered(win, 1, 0.6, 0.95);
	});

	it('Zooming out at the floor lands centered on the slide you left', function() {
		const win = this.win;
		ensureOverviewPage(win);
		goToPart(win, 2);
		desktopHelper.selectZoomLevel('20', false);
		cy.then(function() {
			win.app.zoomControl.zoomTo(win.app.map.getZoom() - 1, undefined, false);
		});
		cy.then(() => helper.processToIdle(win));
		cy.then(function() {
			expect(win.app.map._docLayer._selectedPart).to.equal(0);
		});
		// Landing fill target is 0.85; one 1.2 zoom step below is 0.71.
		assertPreviewCentered(win, 2, 0.55, 0.95);
	});

	it('Zooming in at the ceiling returns to the preview nearest the center', function() {
		const win = this.win;
		ensureOverviewPage(win);
		// Visit part 1 last, so the previous-part fallback of the fade would pick 1;
		// the nearest-preview logic must pick 2 instead.
		goToPart(win, 1);
		goToPart(win, 0);
		cy.then(function() {
			win.app.dispatcher.dispatch('fitslidetoview', { part: 2 });
		});
		assertPreviewCentered(win, 2, 0.6, 0.95);
		cy.then(function() {
			win.app.zoomControl.zoomTo(win.app.map.getMaxZoom(), undefined, false);
		});
		cy.then(() => helper.processToIdle(win));
		cy.then(function() {
			win.app.zoomControl.zoomTo(win.app.map.getZoom() + 1, undefined, false);
		});
		cy.then(() => helper.retryUntil(
			() => helper.processToIdle(win),
			() => win.app.map._docLayer._selectedPart === 2,
			{ errorMsg: 'never switched to the slide nearest the viewport center' }
		));
	});

	it('Preview context menu offers Fit slide to view', function() {
		const win = this.win;
		ensureOverviewPage(win);
		goToPart(win, 0);
		// At 50% both previews are in view and neither fills it, so the click finds preview 2
		// and the fit that follows has to move the view.
		desktopHelper.selectZoomLevel('50', false);
		cy.then(() => helper.processToIdle(win));
		cy.then(function() {
			const rect = previewRect(win, 2);
			const point = toClient(win, rect.center[0], rect.center[1]);
			// A right click alone does not select the shape under the pointer, so
			// click once to select the preview before opening its context menu.
			cy.cGet('#document-canvas').click(point.x, point.y, { force: true });
			cy.cGet('#document-canvas').rightclick(point.x, point.y, { force: true });
		});
		cy.then(() => helper.processToIdle(win));
		cy.then(function() {
			expect(win.app.impress.selectedPreviewPart).to.equal(2);
		});
		helper.getContextMenuItem('Fit slide to view').click();
		assertPreviewCentered(win, 2, 0.6, 0.95);
	});
});
