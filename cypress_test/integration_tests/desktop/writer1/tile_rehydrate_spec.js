/* global describe it cy require expect */

var helper = require('../../common/helper');

describe(['tagdesktop'], 'Tile rehydrate from cache.', function() {

	it('Rehydrates a visible cached tile stuck in needsFetch state.', function() {
		// Regression test for the viewing-mode tile bug.
		//
		// Bug pattern: a sham invalidatetiles message (no real content
		// change) sets every cached tile to wireId == invalidFrom, which
		// makes needsFetch() return true permanently. A subsequent
		// reclaimGraphicsMemory clears tile.image. When the tile enters
		// the visible area on scroll, getMissingTiles puts it in the
		// fetch queue but core does not send fresh content (oldwid
		// matches), so the tile stays blank.
		//
		// Fix: when a visible tile has cached rawDeltas but no image,
		// rehydrate it from the cache instead of waiting for a tile
		// message that never arrives.
		helper.setupAndLoadDocument('writer/copy_paste_simple.odt');

		cy.getFrameWindow().then(function(win) {
			helper.processToIdle(win);
		});

		// Pick a visible tile that has both a keyframe and an image,
		// then force it into the bug state (image cleared, wireId ==
		// invalidFrom). This mirrors what happens after a sham
		// invalidate followed by a graphics-memory reclaim.
		//
		// Also pin lastRequestTime to "now" so requestingTooFast() will
		// filter out the tile in sendTileCombineRequest. Without this,
		// the test environment's core would happily answer the
		// tilecombine and onTileMsg's distance-zero branch would
		// rehydrate the tile via the network path, masking the bug
		// (and the fix).
		var stuckKey;
		cy.getFrameWindow().then(function(win) {
			var tiles = win.RenderManager.getTiles();
			var stuck = null;
			tiles.forEach(function(tile, key) {
				if (stuck) return;
				if (tile.distanceFromView === 0 &&
				    tile.hasKeyframe() &&
				    tile.image) {
					stuck = tile;
					stuckKey = key;
				}
			});
			expect(stuck, 'a visible cached tile to corrupt').to.not.be.null;

			stuck.image.close();
			stuck.image = null;
			stuck.decompressedId = 0;
			stuck.lastPendingId = 0;
			// forceKeyframe(wireId) effect: pin invalidFrom to wireId so
			// needsFetch() returns true and the tile is routed into the
			// missing queue.
			stuck.invalidFrom = stuck.wireId;
			// Keep cooldown active so the network path is closed; only
			// the in-process rehydrate (the fix) can recover this tile.
			stuck.updateLastRequest(new Date());

			expect(stuck.needsFetch(), 'needsFetch after corruption').to.be.true;
			expect(stuck.image, 'image after corruption').to.be.null;
			expect(stuck.requestingTooFast(new Date()),
				'requestingTooFast after corruption').to.be.true;

			win.RenderManager.update();
		});

		cy.getFrameWindow().then(function(win) {
			helper.processToIdle(win);
		});

		// After update + worker decompression, the fix must have
		// rehydrated the tile from its cached rawDeltas. Without the
		// fix tile.image stays null because the network path is blocked
		// by the cooldown we pinned above.
		cy.getFrameWindow().then(function(win) {
			cy.wrap(null).should(function() {
				var tile = win.RenderManager.getTiles().get(stuckKey);
				expect(tile, 'tile still present after update').to.not.be.undefined;
				expect(tile.image, 'tile rehydrated from cache').to.not.be.null;
			});
		});
	});

	it('Shows a cached tile while the scroll path waits to fetch it again.', function() {
		// The same broken tile state as in the test above, but on the path
		// every scroll takes, requestVisibleTiles. The fetch has to wait for
		// the request cooldown, and the tile must be drawn from its cached
		// deltas in the meantime.
		helper.setupAndLoadDocument('writer/copy_paste_simple.odt');

		cy.getFrameWindow().then(function(win) {
			helper.processToIdle(win);
		});

		var stuckKey;
		cy.getFrameWindow().then(function(win) {
			var tiles = win.RenderManager.getTiles();
			var stuck = null;
			tiles.forEach(function(tile, key) {
				if (stuck) return;
				if (tile.distanceFromView === 0 &&
				    tile.hasKeyframe() &&
				    tile.image) {
					stuck = tile;
					stuckKey = key;
				}
			});
			expect(stuck, 'a visible cached tile to corrupt').to.not.be.null;

			stuck.image.close();
			stuck.image = null;
			stuck.decompressedId = 0;
			stuck.lastPendingId = 0;
			stuck.invalidFrom = stuck.wireId;
			// A request time in the future keeps the cooldown active for the
			// whole test, so only the decode in the browser can restore the
			// tile, not the retry timer.
			stuck.updateLastRequest(new Date(Date.now() + 60 * 1000));

			expect(stuck.needsFetch(), 'needsFetch after corruption').to.be.true;
			expect(stuck.image, 'image after corruption').to.be.null;

			win.RenderManager.requestVisibleTiles(
				win.app.activeDocument.activeLayout.getCurrentCoordList());
		});

		cy.getFrameWindow().then(function(win) {
			helper.processToIdle(win);
		});

		cy.getFrameWindow().then(function(win) {
			cy.wrap(null).should(function() {
				var tile = win.RenderManager.getTiles().get(stuckKey);
				expect(tile, 'tile still present').to.not.be.undefined;
				expect(tile.image, 'tile decoded from its cached deltas').to.not.be.null;
			});
		});
	});

	it('Takes an empty answer at the invalidation id as current content.', function() {
		// A tile invalidated with its own wire id is requested again with
		// that id. The server has nothing newer, so it answers with an empty
		// delta and the same id, and the tile must be up to date after that.
		helper.setupAndLoadDocument('writer/copy_paste_simple.odt');

		cy.getFrameWindow().then(function(win) {
			helper.processToIdle(win);
		});

		var staleKey;
		cy.getFrameWindow().then(function(win) {
			var tiles = win.RenderManager.getTiles();
			var stale = null;
			tiles.forEach(function(tile, key) {
				if (stale) return;
				if (tile.distanceFromView === 0 &&
				    tile.hasKeyframe() &&
				    tile.image) {
					stale = tile;
					staleKey = key;
				}
			});
			expect(stale, 'a visible cached tile to mark stale').to.not.be.null;

			stale.invalidFrom = stale.wireId;
			// The request has to be sent immediately in this test.
			stale.allowFastRequest();

			expect(stale.needsFetch(), 'needsFetch after marking').to.be.true;

			win.RenderManager.update();
		});

		cy.getFrameWindow().then(function(win) {
			helper.processToIdle(win);
		});

		cy.getFrameWindow().then(function(win) {
			cy.wrap(null).should(function() {
				var tile = win.RenderManager.getTiles().get(staleKey);
				expect(tile, 'tile still present').to.not.be.undefined;
				expect(tile.needsFetch(), 'tile current after the answer').to.be.false;
				expect(tile.image, 'tile still drawn').to.not.be.null;
			});
		});
	});
});
