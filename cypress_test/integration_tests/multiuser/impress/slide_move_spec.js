/* global describe it cy beforeEach require expect */

var helper = require('../../common/helper');

describe(['tagmultiuser'], 'Multiuser slide reorder and delete', function() {
	var win1, win2;

	beforeEach(function() {
		helper.setupAndLoadDocument('impress/slide-move.fodp', true);
		cy.getFrameWindow('#iframe1').then(function(win) { win1 = win; });
		cy.getFrameWindow('#iframe2').then(function(win) { win2 = win; });
	});

	// The slide list arrives after the document is otherwise ready, so wait for
	// the known 3-slide layout.
	function waitForLayout(win) {
		cy.wrap(null).should(function() {
			expect(win.app.impress.partList).to.have.length(3);
		});
	}

	// Click the thumbnail of the slide at the given part index. The preview
	// img element ids are creation-order ids, not part indexes, so pick the
	// thumbnail by position: frame 0 is the reorder drop-site, frame k is
	// slide k.
	function clickSlideThumbnail(part) {
		cy.cGet('.preview-frame').eq(part + 1).find('img').scrollIntoView().click();
	}

	// Collect the ready tiles cached for one slide, as the live Tile objects.
	// Tiles are keyed by the slide's stable unique id, so the same key finds
	// them wherever the slide sits in the order.
	function readyTilesForSlide(win, uniqueId) {
		var tiles = [];
		win.RenderManager.getTiles().forEach(function(tile) {
			if (tile.coords.part === uniqueId && tile.isReadyToDraw())
				tiles.push(tile);
		});
		return tiles;
	}

	it('moving a slide keeps the other view in place', function() {
		cy.cSetActiveFrame('#iframe1');
		waitForLayout(win1);
		cy.cSetActiveFrame('#iframe2');
		waitForLayout(win2);

		// User A parks on the last slide, index 2, which User B's move leaves in
		// place because B only reorders the two slides above it.
		cy.cSetActiveFrame('#iframe1');
		clickSlideThumbnail(2);
		helper.processToIdle(win1);
		cy.wrap(null).should(function() {
			expect(win1.app.map.getCurrentPartNumber()).to.equal(2);
		});

		// Watch every slide index User A's status handler settles on from here
		// on. A correct run only ever sees A's own slide, index 2.
		cy.then(function() {
			var docLayer = win1.app.map._docLayer;
			win1.__seenParts = [];
			var original = docLayer._onStatusMsg;
			docLayer._onStatusMsg = function(textMsg) {
				original.call(docLayer, textMsg);
				win1.__seenParts.push(docLayer._selectedPart);
			};
		});

		// User B selects slide index 0 and moves it to position 1, so B ends up
		// on slide index 1. This is the wire message a slide-sorter drop sends.
		cy.cSetActiveFrame('#iframe2');
		clickSlideThumbnail(0);
		helper.processToIdle(win2);
		cy.then(function() {
			win2.__orderBefore = win2.app.impress.partList.map(function(part) { return part.part; });
			win2.app.socket.sendMessage('moveselectedclientparts position=1');
		});
		helper.processToIdle(win2);

		// Confirm the move actually happened: the slide order changed, so the
		// test cannot pass on a silent no-op.
		cy.wrap(null).should(function() {
			var orderAfter = win2.app.impress.partList.map(function(part) { return part.part; });
			expect(orderAfter).to.not.deep.equal(win2.__orderBefore);
		});

		// Let User A finish reacting to the broadcast.
		cy.cSetActiveFrame('#iframe1');
		helper.processToIdle(win1);

		// A must have processed at least one status from the move (otherwise
		// the check below is vacuous), and every one of them must have left A
		// on its own slide. Adopting B's slide, index 1, here is the bug.
		cy.wrap(null).should(function() {
			expect(win1.__seenParts.length).to.be.greaterThan(0);
			expect(win1.__seenParts).to.not.include(1);
			win1.__seenParts.forEach(function(part) {
				expect(part).to.equal(2);
			});
		});
	});

	it('deleting an earlier slide keeps the other view\'s tiles valid', function() {
		cy.cSetActiveFrame('#iframe1');
		waitForLayout(win1);
		cy.cSetActiveFrame('#iframe2');
		waitForLayout(win2);

		// User A parks on the last slide, index 2. Deleting a slide before it
		// renumbers this slide to index 1, which is where the main view used to
		// flicker while tiles were keyed by part index.
		cy.cSetActiveFrame('#iframe1');
		clickSlideThumbnail(2);
		helper.processToIdle(win1);
		cy.wrap(null).should(function() {
			expect(win1.app.map.getCurrentPartNumber()).to.equal(2);
		});

		// Remember the slide's unique id, wait until A has drawn the slide, and
		// capture the exact tile objects rendering it. Tiles are keyed by the
		// slide's id, so the renumbering must leave these same tile objects
		// cached and drawable under the same key.
		cy.then(function() {
			win1.__viewedUniqueId = win1.app.impress.partList[2].part;
		});
		cy.wrap(null).should(function() {
			expect(readyTilesForSlide(win1, win1.__viewedUniqueId).length).to.be.greaterThan(0);
		});
		cy.then(function() {
			win1.__capturedTiles = readyTilesForSlide(win1, win1.__viewedUniqueId);
			win1.__tileChecks = [];

			// After each status, once A's slide has shifted to a new index,
			// record whether the captured tiles are still cached and drawable
			// under the slide's own id.
			var docLayer = win1.app.map._docLayer;
			var original = docLayer._onStatusMsg;
			docLayer._onStatusMsg = function(textMsg) {
				original.call(docLayer, textMsg);
				var part = win1.app.map.getCurrentPartNumber();
				if (part !== 2) {
					var kept = win1.__capturedTiles.every(function(tile) {
						return tile.coords.part === win1.__viewedUniqueId &&
							tile.isReadyToDraw();
					});
					win1.__tileChecks.push(kept);
				}
			};
		});

		// User B deletes the first slide, index 0.
		cy.cSetActiveFrame('#iframe2');
		clickSlideThumbnail(0);
		cy.wrap(null).should(function() {
			expect(win2.app.map.getCurrentPartNumber()).to.equal(0);
		});
		cy.then(function() { win2.app.map.deletePage(); });
		helper.processToIdle(win2);
		cy.wrap(null).should(function() {
			expect(win2.app.impress.partList).to.have.length(2);
		});

		// Let User A finish reacting to the broadcast.
		cy.cSetActiveFrame('#iframe1');
		helper.processToIdle(win1);

		cy.wrap(null).should(function() {
			// A stays on its own slide, now renumbered from index 2 to index 1.
			expect(win1.app.map.getCurrentPartNumber()).to.equal(1);
			expect(win1.app.impress.partList[1].part).to.equal(win1.__viewedUniqueId);

			// A processed the shift at least once, and every time the slide's
			// own tiles were still in place under its id: no stale slide drawn,
			// nothing refetched.
			expect(win1.__tileChecks.length).to.be.greaterThan(0);
			win1.__tileChecks.forEach(function(kept) {
				expect(kept).to.equal(true);
			});
		});
	});
});
