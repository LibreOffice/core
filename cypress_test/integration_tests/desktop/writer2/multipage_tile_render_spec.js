/* global describe it cy beforeEach require expect */

var helper = require('../../common/helper');

describe(['tagdesktop', 'tagnextcloud', 'tagproxy'], 'Writer multi-page view tile rendering.', function() {

	beforeEach(function() {
		helper.setupAndLoadDocument('writer/scrolling.odt');
		cy.viewport(1920, 1080);
	});

	// Capture every drawTileToCanvasCrop call made by drawForViewLayoutMultiPage
	// and return them together with the layout, so individual tests can assert on
	// the geometry the function produced.
	function collectTileDraws(win) {
		var section = win.app.sectionContainer.getSectionWithName(win.app.CSections.Tiles.name);
		var view = win.app.activeDocument.activeLayout;

		var calls = [];
		var original = section.drawTileToCanvasCrop;
		// Record args only; skip the real blit so the test does not depend on
		// the canvas state and cannot be affected by the actual draw.
		section.drawTileToCanvasCrop = function(tile, canvas, sx, sy, sw, sh, dx, dy, dWidth, dHeight) {
			var tilePos = tile.coords.getPosSimplePoint();
			calls.push({
				tilePos: tilePos,
				sx: sx, sy: sy, sw: sw, sh: sh,
				dx: dx, dy: dy, dWidth: dWidth, dHeight: dHeight,
			});
		};

		try {
			section.drawForViewLayoutMultiPage();
		} finally {
			section.drawTileToCanvasCrop = original;
		}

		return { calls: calls, view: view, tileSize: win.RenderManager.tileSize };
	}

	// Find a document page rectangle that fully contains the given document-space
	// rectangle, allowing for sub-pixel rounding.
	function containingPage(view, x1, y1, x2, y2) {
		var eps = 1;
		for (var p = 0; p < view.documentRectangles.length; p++) {
			var pr = view.documentRectangles[p];
			if (x1 >= pr.pX1 - eps && y1 >= pr.pY1 - eps &&
				x2 <= pr.pX2 + eps && y2 <= pr.pY2 + eps) {
				return p;
			}
		}
		return -1;
	}

	// Regression test for: "Writer multi-page view: Fix view when pages have
	// different sizes." drawForViewLayoutMultiPage clips every core tile to each
	// page it overlaps in document space and blits only that intersection. The
	// invariant the fix guarantees is that every blitted fragment lands fully
	// inside a single page rectangle. The previous code added a 3px "magic"
	// overflow at page seams, which violated this.
	it('Every drawn tile fragment stays within a page rectangle.', function() {
		cy.cGet('#multi-page-view-button').click();

		cy.getFrameWindow().then(function(win) {
			helper.processToIdle(win);
		});

		cy.getFrameWindow().then(function(win) {
			var layout = win.app.activeDocument.activeLayout;
			expect(layout.type).to.equal('ViewLayoutMultiPage');
			// scrolling.odt spans several pages, so we exercise page seams.
			expect(layout.documentRectangles.length, 'page count').to.be.greaterThan(1);

			// Retry until the visible tiles have loaded and produced fragments.
			cy.wrap(null).should(function() {
				var result = collectTileDraws(win);

				expect(result.calls.length, 'tile fragments drawn').to.be.greaterThan(0);

				result.calls.forEach(function(c) {
					// Blits must be 1:1 - the multi-page path never scales tiles.
					expect(c.sw, 'source width == dest width').to.equal(c.dWidth);
					expect(c.sh, 'source height == dest height').to.equal(c.dHeight);

					// Source crop must lie inside the tile.
					expect(c.sx, 'sx >= 0').to.be.at.least(0);
					expect(c.sy, 'sy >= 0').to.be.at.least(0);
					expect(c.sx + c.sw, 'sx + sw <= tileSize').to.be.at.most(result.tileSize + 1);
					expect(c.sy + c.sh, 'sy + sh <= tileSize').to.be.at.most(result.tileSize + 1);

					// The fragment, mapped back to document space, must be fully
					// contained in one page rectangle - no overflow into the
					// inter-page gap or a neighbouring page.
					var x1 = c.tilePos.pX + c.sx;
					var y1 = c.tilePos.pY + c.sy;
					var x2 = x1 + c.sw;
					var y2 = y1 + c.sh;

					var page = containingPage(result.view, x1, y1, x2, y2);
					expect(page, 'fragment [' + x1 + ',' + y1 + ',' + x2 + ',' + y2 + '] within a page').to.not.equal(-1);
				});
			});
		});
	});

	// A core tile sitting across a document-space page boundary must be split
	// into one fragment per page, rather than being drawn as a single tile.
	it('A tile spanning two pages is split into separate fragments.', function() {
		cy.cGet('#multi-page-view-button').click();

		cy.getFrameWindow().then(function(win) {
			helper.processToIdle(win);
		});

		cy.getFrameWindow().then(function(win) {
			// Retry until tiles have loaded and at least one straddles a seam.
			cy.wrap(null).should(function() {
				var result = collectTileDraws(win);

				// Count how many fragments each tile produced. A tile straddling
				// a page seam yields more than one fragment.
				var fragmentsPerTile = {};
				result.calls.forEach(function(c) {
					var key = c.tilePos.pX + ':' + c.tilePos.pY;
					fragmentsPerTile[key] = (fragmentsPerTile[key] || 0) + 1;
				});

				var splitTiles = Object.keys(fragmentsPerTile).filter(function(key) {
					return fragmentsPerTile[key] > 1;
				});

				expect(splitTiles.length, 'tiles split across page boundaries').to.be.greaterThan(0);
			});
		});
	});
});
