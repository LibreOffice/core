/* -*- js-indent-level: 8 -*- */
/*
 * Copyright the Collabora Online contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

describe('ViewLayout', function() {

var _ = function(text: string) { return text; };

class ResizeObserver {
	constructor(f: Function) {}

	disconnect() {}

	observe() {}

	unobserve() {}
}

function initializeJSDOM() {
	addCanvasToDom();
	addMockCanvas(window);
	global.requestAnimationFrame = window.requestAnimationFrame;
	global.cancelAnimationFrame = window.cancelAnimationFrame;
	// DocumentBase / TableMiddleware call bare `getComputedStyle(...)`.
	// In the browser it's a window global; in node we forward to jsdom's.
	global.getComputedStyle = window.getComputedStyle.bind(window);
	global.ResizeObserver = ResizeObserver;

	const canvasWidth = 1024;
	const canvasHeight = 768;
	setupCanvasContainer(canvasWidth, canvasHeight);
	app.events = new DocEvents();
}

describe('View Layout Tests', function () {
    this.beforeAll(initializeJSDOM);

	// Install the app-level stubs.
	function setupAppStubs(pixelsToTwips: number): void {
		// CSections read by MouseControl / TextSelectionSection field
		// initialisers. Values mirror browser/src/canvas/CanvasSectionProps.js.
		(app.CSections as any).MouseControl = {
			name: 'mouse-control', zIndex: 5,
			processingOrder: 72, drawingOrder: 51,
		};
		(app.CSections as any).Tiles = {
			name: 'tiles', zIndex: 5,
			processingOrder: 60, drawingOrder: 50,
		};
		(app.CSections as any).TextSelection = {
			name: 'text selection', zIndex: 5,
			processingOrder: 74, drawingOrder: 52,
		};

		// app.file is populated by docstate.ts in the browser. ViewLayoutMultiPage
		// reset() reads app.file.writer.pageRectangleList.
		(app as any).file = { writer: { pageRectangleList: [] } };

		// Use the real twips/pixel convention (1 pixel = 15 twips by default).
		app.pixelsToTwips = pixelsToTwips;
		app.twipsToPixels = 1 / pixelsToTwips;

		// app.map stub.
		app.map = {
			on: function () {},
			uiManager: null,
			_docLayer: {
				_docType: 'text',
				isWriter: function () { return true; },
			},
			getScaleZoom: function () { return 1; },
			setZoom: function () {},
			getZoom: function () { return 10; },
		} as any;

		// sendTileCombineMessage reads app.tile.size.{x,y} and calls
		// app.socket.sendMessage. Provide both.
		(app.tile as any).size = { x: 3840, y: 3840 };
		(app.socket as any).sendMessage = function () {};

		// CanvasSectionContainer.getDocumentAnchorSection returns null until a
		// section has been registered as the anchor. adjustViewZoomLevel and
		// refreshCurrentCoordList both dereference .size, so give it a minimal
		// section-like object. The size matches setupCanvasContainer(1024, 768).
		(app.sectionContainer as any).getDocumentAnchorSection = function () {
			return { size: [1024, 768], myTopLeft: [0, 0] };
		};
	}

	// Record every tile-combine request the production code sends. The return
	// value includes a `sentRequests` history array (grown in-place on every
	// call) and a `restore` hook to revert the patch.
	function instrumentTileCombineRequests(tileMgr: any): {
		sentRequests: Array<Array<TileCoordData>>;
		restore: () => void;
	} {
		const sentRequests: Array<Array<TileCoordData>> = [];
		const original: Function = tileMgr.sendTileCombineRequest;
		tileMgr.sendTileCombineRequest = function (queue: Array<TileCoordData>) {
			if (queue.length > 0) {
				// Snapshot: the caller may mutate the queue after returning.
				sentRequests.push(queue.slice());
			}
			original.call(tileMgr, queue);
		};
		return {
			sentRequests,
			restore: () => {
				tileMgr.sendTileCombineRequest = original;
			},
		};
	}

	// Observe bitmap production & reclamation. The state machine:
	// `renderedKeys` is the set of tiles currently considered to have a
	// produced image. Every visible tile ends up here once the
	// request/response round-trip completes.
	function instrumentTileRendering(tileMgr: any): {
		renderedKeys: Set<string>;
		restore: () => void;
	} {
		const renderedKeys: Set<string> = new Set();
		const origSet: Function = tileMgr.setBitmapOnTile;
		const origCreate: Function = tileMgr.createTileBitmap;
		const origReclaim: Function = tileMgr.reclaimTileBitmapMemory;

		tileMgr.setBitmapOnTile = function (tile: Tile, bitmap: ImageBitmap) {
			renderedKeys.add(tile.coords.key());
			return origSet.call(tileMgr, tile, bitmap);
		};
		tileMgr.createTileBitmap = function (
			tile: Tile,
			delta: any,
			deltas: any[],
			bitmaps: Promise<ImageBitmap>[],
		) {
			renderedKeys.add(tile.coords.key());
			return origCreate.call(tileMgr, tile, delta, deltas, bitmaps);
		};
		tileMgr.reclaimTileBitmapMemory = function (tile: Tile) {
			renderedKeys.delete(tile.coords.key());
			return origReclaim.call(tileMgr, tile);
		};

		return {
			renderedKeys,
			restore: () => {
				tileMgr.setBitmapOnTile = origSet;
				tileMgr.createTileBitmap = origCreate;
				tileMgr.reclaimTileBitmapMemory = origReclaim;
			},
		};
	}

	// Stand in for the browser receiving a rendered bitmap for `tile`: bump
	// wireId so needsFetch() returns false, then drive the real
	// setBitmapOnTile path with a stand-in bitmap so the test can move past
	// the request/response boundary without wiring up fzstd +
	// createImageBitmap. Ordering mirrors BitmapTileManager.onTileMsg.
	function simulateTileArrival(tileMgr: any, tile: Tile): void {
		if (tile.wireId === 0) tile.wireId = 1;
		const fakeBitmap = { close: function () {} } as any;
		tileMgr.setBitmapOnTile(tile, fakeBitmap);
	}

	// Reclaim every tile we previously installed a bitmap on. Returns the
	// shared state to "needs fetch" so the next scenario's request list is
	// independent of earlier scenarios'. In production, off-screen tiles
	// are dropped via garbage collection over time.
	function clearRenderedTiles(
		tileMgr: any,
		renderedKeys: Set<string>,
	): void {
		for (const key of Array.from(renderedKeys)) {
			const tile = tileMgr.tiles.get(key);
			if (tile) tileMgr.reclaimTileBitmapMemory(tile);
		}
		nodeassert.strictEqual(renderedKeys.size, 0);
	}

	// Shallow "expected subset of actual" assertion over sorted tile keys.
	// The real layout may request more tiles than our hand-computed minimum
	// (prefetch, overdraw); the contract we verify is that every hand-
	// computed tile IS in the request.
	function assertSubset(
		label: string,
		expected: string[],
		actual: string[],
	): void {
		const actualSet = new Set(actual);
		const missing = expected.filter((k) => !actualSet.has(k));
		if (missing.length > 0) {
			console.error('[' + label + '] expected tiles missing from request.');
			console.error('  missing:   ' + JSON.stringify(missing));
			console.error('  expected:  ' + JSON.stringify(expected));
			console.error('  requested: ' + JSON.stringify(actual));
		}
		nodeassert.deepStrictEqual(
			missing,
			[],
			label + ': every expected tile must appear in the request',
		);
	}

	// ========================================================================
	// Tests
	// ========================================================================

    it('Check Pan Direction', function () {
		app.map = {
			on: function() {}
		} as any

        const viewLayout = new ViewLayoutBase();

        viewLayout.viewedRectangle = new cool.SimpleRectangle(100, 100, 100, 100);
        let panDirection = viewLayout.getLastPanDirection();

        nodeassert.ok(panDirection[0] === 1 && panDirection[1] === 1);

        viewLayout.viewedRectangle = new cool.SimpleRectangle(50, 50, 100, 100);
        panDirection = viewLayout.getLastPanDirection();
        nodeassert.ok(panDirection[0] === -1 && panDirection[1] === -1);
	});

	it('MultiPage tracks requested tiles across zoom and scroll', function () {
		setupAppStubs(15);

		// Assign a real DocumentBase instance. _fileSize must cover all 3
		// pages because BitmapTileManager.isValidTile rejects tile coordinates
		// outside fileSize.
		const activeDocument = new DocumentBase();
		(activeDocument as any)._fileSize = new cool.SimplePoint(7500, 49000);
		app.activeDocument = activeDocument;

		const multiPageViewLayout = new ViewLayoutMultiPage();

		// TileManager.tileSize forces lazy singleton creation; then we reach
		// into the BitmapTileManager instance and install monkey patches for
		// the two observation points: the tile-combine request queue and the
		// bitmap production/reclamation state machine.
		void TileManager.tileSize;
		const tileMgr: any = (TileManager as any)._instance;

		const tileCombine = instrumentTileCombineRequests(tileMgr);
		const rendering = instrumentTileRendering(tileMgr);
		const sentTileCombineRequests = tileCombine.sentRequests;
		const renderedKeys = rendering.renderedKeys;

		// A Writer document with three identical pages.
		const pageRectangles = [
			[0, 0, 7500, 15000],
			[0, 17000, 7500, 15000],
			[0, 34000, 7500, 15000],
		];
		app.file.writer.pageRectangleList = pageRectangles;

		// Drive the layout step directly; the public reset() defers via
		// LayoutingService and would also invoke updateViewData which
		// expects a live socket / docLayer.
		(multiPageViewLayout as any).resetViewLayout();

		// Feed the layout's visible-tile list through checkRequestTiles;
		// every fresh tile has needsFetch() === true, so the monkey-patched
		// wrapper records a queue equal to the visible list.
		const triggerRequest = (): void => {
			(multiPageViewLayout as any).refreshCurrentCoordList();
			TileManager.checkRequestTiles(
				multiPageViewLayout.getCurrentCoordList().slice(),
			);
		};

		const lastRequestedKeys = (): string[] => {
			nodeassert.ok(
				sentTileCombineRequests.length > 0,
				'expected a new tile-combine request to be recorded',
			);
			return sentTileCombineRequests[sentTileCombineRequests.length - 1]
				.map(function (c: any) { return c.key(); })
				.sort();
		};

		const visibleCoords = (): TileCoordData[] =>
			multiPageViewLayout.getCurrentCoordList().slice();

		// Run a single scenario's invariants: every expected tile must be
		// requested, then after a simulated response every visible tile must
		// be recorded as rendered and drawable, and a second pass must not
		// emit any new request because all visible tiles are now current.
		const runScenarioInvariants = (
			label: string,
			expectedKeys: string[],
		): void => {
			triggerRequest();
			assertSubset(label, expectedKeys, lastRequestedKeys());

			for (const coords of visibleCoords()) {
				const tile = tileMgr.get(coords);
				nodeassert.ok(
					tile,
					label + ': tile ' + coords.key() + ' should exist after request',
				);
				simulateTileArrival(tileMgr, tile);
			}

			for (const coords of visibleCoords()) {
				nodeassert.ok(
					renderedKeys.has(coords.key()),
					label + ': visible tile ' + coords.key() +
						' has no bitmap produced',
				);
				nodeassert.ok(
					tileMgr.get(coords).isReadyToDraw(),
					label + ': visible tile ' + coords.key() +
						' is not ready to draw',
				);
			}

			const beforeNoOp = sentTileCombineRequests.length;
			triggerRequest();
			nodeassert.strictEqual(
				sentTileCombineRequests.length,
				beforeNoOp,
				label + ': no new tile-combine request expected once all ' +
					'visible tiles are rendered',
			);
		};

		// Expected lists below are hand-derived from the scenario inputs
		// (page rectangles, viewport, pixelsToTwips, tileSize = 256, fileSize
		// = (7500, 49000)) without calling refreshCurrentCoordList, so they
		// are an independent reference rather than a self-comparison.
		try {
			// --- Scenario 1: Zoom in -------------------------------------
			// Halve twips-per-pixel so a page grows from 500x1000 to
			// 1000x2000 px. Only page 0 fits vertically; the viewport shows
			// its top portion in core-pixel doc space (0, 0)..(1000, 748).
			// tileSize=256 enumerates x in {0,256,512,768,1024} and
			// y in {0,256,512,768}. isValidTile drops x=1024 because
			// 1024 * 7.5 = 7680 >= fileSize.x (7500). Result: a 4x4 grid.
			app.pixelsToTwips = 7.5;
			app.twipsToPixels = 1 / 7.5;
			(multiPageViewLayout as any).resetViewLayout();

			const expectedZoomIn = [
				'0:0:10:0:0', '0:256:10:0:0', '0:512:10:0:0', '0:768:10:0:0',
				'256:0:10:0:0', '256:256:10:0:0', '256:512:10:0:0', '256:768:10:0:0',
				'512:0:10:0:0', '512:256:10:0:0', '512:512:10:0:0', '512:768:10:0:0',
				'768:0:10:0:0', '768:256:10:0:0', '768:512:10:0:0', '768:768:10:0:0',
			].sort();

			runScenarioInvariants('zoom-in', expectedZoomIn);

			// --- Scenario 2: Scroll down by one viewport -----------------
			// Stay at the zoomed-in scale. Sliding viewY to 768 exposes
			// doc-y 748..1516 of page 0; page 1 starts at view-y 2040 and is
			// still off-screen. tile y in {512,768,1024,1280,1536}, tile x
			// in {0,256,512,768} (1024 dropped as above). Result: 4x5.
			//
			// Reset rendered state first so the subset assertion sees every
			// tile go through the fetch path - not just the non-overlapping
			// ones left over from zoom-in.
			clearRenderedTiles(tileMgr, renderedKeys);
			multiPageViewLayout.scrollProperties.viewY = 768;

			const expectedAfterScroll = [
				'0:512:10:0:0', '0:768:10:0:0', '0:1024:10:0:0', '0:1280:10:0:0', '0:1536:10:0:0',
				'256:512:10:0:0', '256:768:10:0:0', '256:1024:10:0:0', '256:1280:10:0:0', '256:1536:10:0:0',
				'512:512:10:0:0', '512:768:10:0:0', '512:1024:10:0:0', '512:1280:10:0:0', '512:1536:10:0:0',
				'768:512:10:0:0', '768:768:10:0:0', '768:1024:10:0:0', '768:1280:10:0:0', '768:1536:10:0:0',
			].sort();

			runScenarioInvariants('scroll-down', expectedAfterScroll);

			// --- Scenario 3: Zoom out so all three pages fit -------------
			// At pixelsToTwips = 70 each page is 107x214 px. Two pages fit
			// side-by-side on the top row, the third lives on a second row,
			// and the full stack is < 768 px tall so everything is on-screen.
			// Per-page tiles before isValidTile: page 0 {(0,0),(0,256)},
			// page 1 {(0,0),(0,256),(0,512)}, page 2 {(0,256),(0,512),(0,768)}.
			// isValidTile drops every x >= 256 (256*70 = 17920 >= 7500) and
			// y = 768 for page 2 (768*70 = 53760 >= 49000). Deduped union:
			// (0,0), (0,256), (0,512).
			clearRenderedTiles(tileMgr, renderedKeys);
			app.pixelsToTwips = 70;
			app.twipsToPixels = 1 / 70;
			multiPageViewLayout.scrollProperties.viewX = 0;
			multiPageViewLayout.scrollProperties.viewY = 0;
			(multiPageViewLayout as any).resetViewLayout();

			const expectedZoomOut = [
				'0:0:10:0:0', '0:256:10:0:0', '0:512:10:0:0',
			].sort();

			runScenarioInvariants('zoom-out', expectedZoomOut);

			// --- Scenario 4: Reclaim a few tiles, verify re-request ------
			// reclaimTileBitmapMemory drops tiles out of renderedKeys (via
			// our patch) AND nulls tile.image (via the real impl), so
			// needsFetch() returns true for them again. The next request
			// should enumerate exactly the reclaimed tiles; the still-
			// rendered one must NOT be re-fetched.
			const visibleAtZoomOut = visibleCoords();
			const toReclaim = visibleAtZoomOut.slice(0, 2);
			const toKeep = visibleAtZoomOut.slice(2);

			for (const coords of toReclaim) {
				tileMgr.reclaimTileBitmapMemory(tileMgr.get(coords));
				nodeassert.ok(
					!renderedKeys.has(coords.key()),
					'reclaim: tile ' + coords.key() +
						' should have been dropped from renderedKeys',
				);
			}

			const beforeReclaim = sentTileCombineRequests.length;
			triggerRequest();
			nodeassert.ok(
				sentTileCombineRequests.length > beforeReclaim,
				'reclaim: a new tile-combine request should have been recorded',
			);
			const reclaimRequestKeys = new Set(lastRequestedKeys());
			for (const coords of toReclaim) {
				nodeassert.ok(
					reclaimRequestKeys.has(coords.key()),
					'reclaim: expected ' + coords.key() + ' in re-request',
				);
			}
			for (const coords of toKeep) {
				nodeassert.ok(
					!reclaimRequestKeys.has(coords.key()),
					'reclaim: still-rendered tile ' + coords.key() +
						' must not be re-fetched',
				);
			}
		} finally {
			// Always restore the original methods so later tests in this
			// suite (or a future addition) see untouched production code.
			rendering.restore();
			tileCombine.restore();
		}
	});
});

});
