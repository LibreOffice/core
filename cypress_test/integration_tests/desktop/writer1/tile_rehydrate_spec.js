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
			var tiles = win.TileManager.getTiles();
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

			win.TileManager.update();
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
				var tile = win.TileManager.getTiles().get(stuckKey);
				expect(tile, 'tile still present after update').to.not.be.undefined;
				expect(tile.image, 'tile rehydrated from cache').to.not.be.null;
			});
		});
	});
});
