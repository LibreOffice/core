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

describe('ViewLayout', function () {
	var _ = function (text: string) {
		return text;
	};

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
				name: 'mouse-control',
				zIndex: 5,
				processingOrder: 72,
				drawingOrder: 51,
			};
			(app.CSections as any).Tiles = {
				name: 'tiles',
				zIndex: 5,
				processingOrder: 60,
				drawingOrder: 50,
			};
			(app.CSections as any).TextSelection = {
				name: 'text selection',
				zIndex: 5,
				processingOrder: 74,
				drawingOrder: 52,
			};
			// DocumentBase.addSections() now instantiates ZoomControl, whose field
			// initialisers read app.CSections.ZoomControl.
			(app.CSections as any).ZoomControl = {
				name: 'zoom control',
				zIndex: 13,
				processingOrder: 2,
				drawingOrder: 2,
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
				off: function () {},
				// applyZoom fires 'zoomend'/'zoomlevelschange' at the end; the doc
				// type's listeners do the post-zoom work in the browser. Nothing is
				// wired up here, so fire() is a no-op (the layouts call reset()
				// directly in their constructors).
				fire: function () {},
				uiManager: null,
				_docLayer: {
					_docType: 'text',
					isWriter: function () {
						return true;
					},
					_sendClientZoom: function () {},
					// A text document has a single part, part 0.
					getSelectedPart: function () {
						return '0';
					},
					// Text and spreadsheet documents name a part by its index
					// written in decimal, so the identifier parses back to the
					// index.
					getIndexFromPart: function (part: string) {
						return parseInt(part);
					},
					// applyZoom calls this to recompute app.twipsToPixels for the new
					// zoom. The tests pin twipsToPixels to a fixed scale (see above),
					// so keep it a no-op to preserve that scale.
					_updateTileTwips: function () {},
				},
				getScaleZoom: function () {
					return 1;
				},
				setZoom: function () {},
				getZoom: function () {
					return 10;
				},
				// applyZoom still routes the target through map._limitZoom while the
				// map is being removed from the zoom path. The test zoom levels are
				// already valid integral zooms, so clamp is a no-op here.
				_limitZoom: function (zoom: number) {
					return zoom;
				},
			} as any;

			// sendTileCombineMessage reads app.tile.size.{x,y} and calls
			// app.socket.sendMessage. Provide both.
			(app.tile as any).size = { x: 3840, y: 3840 };
			(app.socket as any).sendMessage = function () {};

			// The tile grid is enumerated in steps of window.tileSize pixels. The
			// browser sets this from the server; tests fix it at 256 pixels, which
			// matches a 3840 twip tile at the default 15 twips per pixel.
			window.tileSize = 256;

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
		function instrumentTileCombineRequests(tileManager: any): {
			sentRequests: Array<Array<TileCoordData>>;
			restore: () => void;
		} {
			const sentRequests: Array<Array<TileCoordData>> = [];
			const original: Function = tileManager.sendTileCombineRequest;
			tileManager.sendTileCombineRequest = function (
				queue: Array<TileCoordData>,
			) {
				if (queue.length > 0) {
					// Snapshot: the caller may mutate the queue after returning.
					sentRequests.push(queue.slice());
				}
				original.call(tileManager, queue);
			};
			return {
				sentRequests,
				restore: () => {
					tileManager.sendTileCombineRequest = original;
				},
			};
		}

		// Observe bitmap production & reclamation. The state machine:
		// `renderedKeys` is the set of tiles currently considered to have a
		// produced image. Every visible tile ends up here once the
		// request/response round-trip completes.
		function instrumentTileRendering(tileManager: any): {
			renderedKeys: Set<string>;
			restore: () => void;
		} {
			const renderedKeys: Set<string> = new Set();
			const origSet: Function = tileManager.setBitmapOnTile;
			const origCreate: Function = tileManager.createTileBitmap;
			const origReclaim: Function = tileManager.reclaimTileBitmapMemory;

			tileManager.setBitmapOnTile = function (tile: Tile, bitmap: ImageBitmap) {
				renderedKeys.add(tile.coords.key());
				return origSet.call(tileManager, tile, bitmap);
			};
			tileManager.createTileBitmap = function (
				tile: Tile,
				delta: any,
				deltas: any[],
				bitmaps: Promise<ImageBitmap>[],
			) {
				renderedKeys.add(tile.coords.key());
				return origCreate.call(tileManager, tile, delta, deltas, bitmaps);
			};
			tileManager.reclaimTileBitmapMemory = function (tile: Tile) {
				renderedKeys.delete(tile.coords.key());
				return origReclaim.call(tileManager, tile);
			};

			return {
				renderedKeys,
				restore: () => {
					tileManager.setBitmapOnTile = origSet;
					tileManager.createTileBitmap = origCreate;
					tileManager.reclaimTileBitmapMemory = origReclaim;
				},
			};
		}

		// Stand in for the browser receiving a rendered bitmap for `tile`: bump
		// wireId so needsFetch() returns false, then drive the real
		// setBitmapOnTile path with a stand-in bitmap so the test can move past
		// the request/response boundary without wiring up fzstd +
		// createImageBitmap. Ordering mirrors BitmapTileManager.onTileMsg.
		function simulateTileArrival(tileManager: any, tile: Tile): void {
			if (tile.wireId === 0) tile.wireId = 1;
			const fakeBitmap = { close: function () {} } as any;
			tileManager.setBitmapOnTile(tile, fakeBitmap);
		}

		// Reclaim every tile we previously installed a bitmap on. Returns the
		// shared state to "needs fetch" so the next scenario's request list is
		// independent of earlier scenarios'. In production, off-screen tiles
		// are dropped via garbage collection over time.
		function clearRenderedTiles(
			tileManager: any,
			renderedKeys: Set<string>,
		): void {
			for (const key of Array.from(renderedKeys)) {
				const tile = tileManager.tiles.get(key);
				if (tile) tileManager.reclaimTileBitmapMemory(tile);
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

		// Install a fresh BitmapTileManager as the RenderManager singleton so each
		// scenario starts from clean state instead of inheriting tiles, caches and
		// queues left behind by earlier tests.
		function resetRenderManagerState(): any {
			const tileManager: any = new BitmapTileManager();
			(RenderManager as any)._instance = tileManager;
			return tileManager;
		}

		// ========================================================================
		// Shared test context and scenario assertions
		// ========================================================================

		// Create a test context: reset the RenderManager singleton, install
		// monkey-patches, and return everything needed for lifecycle
		// assertions. Caller must call ctx.restore() in a finally block.
		function createTestContext(layout: any): {
			layout: any;
			tileManager: any;
			sentRequests: Array<Array<TileCoordData>>;
			renderedKeys: Set<string>;
			restore: () => void;
		} {
			const tileManager: any = resetRenderManagerState();
			const tileCombine = instrumentTileCombineRequests(tileManager);
			const rendering = instrumentTileRendering(tileManager);
			return {
				layout,
				tileManager,
				sentRequests: tileCombine.sentRequests,
				renderedKeys: rendering.renderedKeys,
				restore: () => {
					rendering.restore();
					tileCombine.restore();
				},
			};
		}

		// Request all visible tiles and simulate their arrival so the
		// tile map is fully populated. Returns the visible coordinate
		// list.
		function requestAndRenderAll(ctx: any): TileCoordData[] {
			(ctx.layout as any).refreshCurrentCoordList();
			RenderManager.checkRequestTiles(ctx.layout.getCurrentCoordList().slice());
			const visibleCoords: TileCoordData[] = ctx.layout
				.getCurrentCoordList()
				.slice();
			for (const coords of visibleCoords) {
				simulateTileArrival(ctx.tileManager, ctx.tileManager.get(coords));
			}
			return visibleCoords;
		}

		// After all visible tiles are rendered, invalidate a subset and
		// verify only those tiles are re-requested.
		function assertInvalidationCycle(ctx: any): void {
			const visibleCoords = requestAndRenderAll(ctx);

			const before = ctx.sentRequests.length;
			(ctx.layout as any).refreshCurrentCoordList();
			RenderManager.checkRequestTiles(ctx.layout.getCurrentCoordList().slice());
			nodeassert.strictEqual(
				ctx.sentRequests.length,
				before,
				'no request expected before invalidation',
			);

			const toInvalidate = visibleCoords.slice(0, 3);
			const toKeep = visibleCoords.slice(3);

			for (const coords of toInvalidate) {
				ctx.tileManager.get(coords).invalidFrom =
					ctx.tileManager.get(coords).wireId;
			}

			(ctx.layout as any).refreshCurrentCoordList();
			RenderManager.checkRequestTiles(ctx.layout.getCurrentCoordList().slice());

			nodeassert.ok(
				ctx.sentRequests.length > before,
				'invalidation should trigger a new tile-combine request',
			);

			const requestKeys = new Set(
				ctx.sentRequests[ctx.sentRequests.length - 1].map(function (c: any) {
					return c.key();
				}),
			);

			for (const coords of toInvalidate) {
				nodeassert.ok(
					requestKeys.has(coords.key()),
					'invalidated tile ' + coords.key() + ' must be re-requested',
				);
			}

			for (const coords of toKeep) {
				nodeassert.ok(
					!requestKeys.has(coords.key()),
					'non-invalidated tile ' + coords.key() + ' must not be re-requested',
				);
			}
		}

		// After all visible tiles are rendered, forceKeyframe on a subset
		// and verify only those are re-requested.
		function assertForceKeyframeCycle(ctx: any): void {
			const visibleCoords = requestAndRenderAll(ctx);

			const toForce = visibleCoords.slice(0, 2);
			const toKeep = visibleCoords.slice(2);

			for (const coords of toForce) {
				ctx.tileManager.get(coords).forceKeyframe();
			}

			const before = ctx.sentRequests.length;
			(ctx.layout as any).refreshCurrentCoordList();
			RenderManager.checkRequestTiles(ctx.layout.getCurrentCoordList().slice());

			nodeassert.ok(
				ctx.sentRequests.length > before,
				'forceKeyframe should trigger a new tile-combine request',
			);

			const requestKeys = new Set(
				ctx.sentRequests[ctx.sentRequests.length - 1].map(function (c: any) {
					return c.key();
				}),
			);

			for (const coords of toForce) {
				nodeassert.ok(
					requestKeys.has(coords.key()),
					'force-keyframed tile ' + coords.key() + ' must be re-requested',
				);
			}

			for (const coords of toKeep) {
				nodeassert.ok(
					!requestKeys.has(coords.key()),
					'untouched tile ' + coords.key() + ' must not be re-requested',
				);
			}
		}

		// Verify that an invalidation arriving while a tile request is
		// in flight survives the stale response.
		function assertRaceProtection(ctx: any): void {
			(ctx.layout as any).refreshCurrentCoordList();
			RenderManager.checkRequestTiles(ctx.layout.getCurrentCoordList().slice());

			const visibleCoords: TileCoordData[] = ctx.layout
				.getCurrentCoordList()
				.slice();

			const raceTile = ctx.tileManager.get(visibleCoords[0]);
			raceTile.invalidFrom = 5;

			simulateTileArrival(ctx.tileManager, raceTile);

			nodeassert.ok(
				raceTile.needsFetch(),
				'tile must still need fetch after stale arrival ' + '(race protection)',
			);

			for (let i = 1; i < visibleCoords.length; i++) {
				simulateTileArrival(
					ctx.tileManager,
					ctx.tileManager.get(visibleCoords[i]),
				);
			}

			const before = ctx.sentRequests.length;
			(ctx.layout as any).refreshCurrentCoordList();
			RenderManager.checkRequestTiles(ctx.layout.getCurrentCoordList().slice());

			nodeassert.ok(
				ctx.sentRequests.length > before,
				'race-affected tile should trigger a new request',
			);

			const raceKeys = new Set(
				ctx.sentRequests[ctx.sentRequests.length - 1].map(function (c: any) {
					return c.key();
				}),
			);

			nodeassert.ok(
				raceKeys.has(visibleCoords[0].key()),
				'race-affected tile ' +
					visibleCoords[0].key() +
					' must be re-requested',
			);

			for (let i = 1; i < visibleCoords.length; i++) {
				nodeassert.ok(
					!raceKeys.has(visibleCoords[i].key()),
					'normally-rendered tile ' +
						visibleCoords[i].key() +
						' must not be re-requested',
				);
			}
		}

		// ========================================================================
		// Layout setup helpers
		// ========================================================================

		// Three-page Writer document at pixelsToTwips=7.5 (zoomed in).
		// pageRectangleList is set AFTER the constructor so that reset()
		// returns early (empty list) and does not queue an async task
		// that would interfere with test state.
		function setupMultiPageForTest(): ViewLayoutMultiPage {
			setupAppStubs(7.5);

			const activeDocument = new DocumentBase();
			(activeDocument as any)._fileSize = new cool.SimplePoint(7500, 49000);
			app.activeDocument = activeDocument;

			const layout = new ViewLayoutMultiPage();

			app.file.writer.pageRectangleList = [
				[0, 0, 7500, 15000],
				[0, 17000, 7500, 15000],
				[0, 34000, 7500, 15000],
			];

			layout.scrollProperties.viewX = 0;
			layout.scrollProperties.viewY = 0;
			(layout as any).resetViewLayout();
			return layout;
		}

		// Side-by-side compare view at pixelsToTwips=7.5. Sets
		// _viewedRectangle and _viewSize directly to avoid calling
		// updateViewData which needs a live socket / docLayer.
		function setupCompareChangesForTest(): ViewLayoutCompareChanges {
			setupAppStubs(7.5);

			const activeDocument = new DocumentBase();
			(activeDocument as any)._fileSize = new cool.SimplePoint(7500, 49000);
			app.activeDocument = activeDocument;

			const layout = new ViewLayoutCompareChanges();

			// The constructor queues an async task that would call
			// updateViewData and overwrite the state we set below.
			// Drain the queue so it never fires.
			(app.layoutingService as any)._layoutTasks.length = 0;

			// Reproduce refreshVisibleAreaRectangle + updateViewData
			// inline.  The margin, yStart and viewGap values match the
			// class defaults when app.dpiScale === 1.
			const margin = 15;
			const yStart = 20;
			const viewGap = 20;
			(layout as any)._viewedRectangle = cool.SimpleRectangle.fromCorePixels([
				-margin,
				-yStart,
				app.activeDocument.fileSize.pX + 2 * margin,
				768,
			]);
			(layout as any)._viewSize = cool.SimplePoint.fromCorePixels([
				Math.max(1024, 2 * app.activeDocument.fileSize.pX + 2 * viewGap),
				Math.max(768, app.activeDocument.fileSize.pY + yStart),
			]);
			return layout;
		}

		// The document-anchor stub that setupAppStubs installs is a fixed literal.
		// This one comes back by reference, so a test can change the frame size and
		// see the layout react to it.
		function installResizableDocumentAnchor(
			width: number,
			height: number,
		): { size: number[]; myTopLeft: number[] } {
			const anchor = { size: [width, height], myTopLeft: [0, 0] };
			(app.sectionContainer as any).getDocumentAnchorSection = function () {
				return anchor;
			};
			return anchor;
		}

		// The plain single-window layout at pixelsToTwips=15, with a 1024x768 frame
		// and a 5000x5000 core-pixel scrollable extent, so both axes have room to
		// scroll. This is the base that carries the shared scroll and visible-area
		// bookkeeping.
		function setupBaseLayoutForTest(): ViewLayoutBase {
			setupAppStubs(15);
			installResizableDocumentAnchor(1024, 768);
			resetRenderManagerState();

			const activeDocument = new DocumentBase();
			(activeDocument as any)._fileSize = new cool.SimplePoint(75000, 75000);
			app.activeDocument = activeDocument;

			const layout = new ViewLayoutBase();
			app.activeDocument.activeLayout = layout;
			layout.viewSize = cool.SimplePoint.fromCorePixels([5000, 5000]);
			return layout;
		}

		// A spreadsheet layout with the same frame and extent. Calc takes the sheet
		// direction, the frozen panes and the selected sheet from the doc layer, and
		// looks the two header sections up by name after a scroll; no header section
		// is registered here, so the names alone are enough. The two spacer entries
		// are there because a spreadsheet document builds those sections.
		function setupCalcForTest(): {
			layout: ViewLayoutCalc;
			anchor: { size: number[]; myTopLeft: number[] };
		} {
			setupAppStubs(15);
			const anchor = installResizableDocumentAnchor(1024, 768);
			resetRenderManagerState();

			const docLayer: any = app.map._docLayer;
			docLayer._docType = 'spreadsheet';
			docLayer.isWriter = function () {
				return false;
			};
			docLayer.isCalc = function () {
				return true;
			};
			docLayer.isCalcRTL = function () {
				return false;
			};
			docLayer.getSplitPanesContext = function () {
				return null;
			};
			(app.map as any).getSplitPanesContext = function () {
				return null;
			};

			(app.CSections as any).RowHeader = { name: 'row header' };
			(app.CSections as any).ColumnHeader = { name: 'column header' };
			(app.CSections as any).RightSpacer = {
				name: 'right spacer',
				zIndex: 5,
				processingOrder: 56,
				drawingOrder: 45,
			};
			(app.CSections as any).BottomSpacer = {
				name: 'bottom spacer',
				zIndex: 5,
				processingOrder: 57,
				drawingOrder: 46,
			};

			const activeDocument = new DocumentBase();
			(activeDocument as any)._fileSize = new cool.SimplePoint(75000, 75000);
			app.activeDocument = activeDocument;

			const layout = activeDocument.activeLayout as ViewLayoutCalc;
			layout.viewSize = cool.SimplePoint.fromCorePixels([5000, 5000]);
			return { layout, anchor };
		}

		// ========================================================================
		// Tests
		// ========================================================================

		it('Check Pan Direction', function () {
			app.map = {
				on: function () {},
				off: function () {},
			} as any;

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

			// Reset to a fresh BitmapTileManager, then install monkey patches on
			// the two observation points: the tile-combine request queue and the
			// bitmap production and reclamation state machine.
			const tileManager: any = resetRenderManagerState();

			const tileCombine = instrumentTileCombineRequests(tileManager);
			const rendering = instrumentTileRendering(tileManager);
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
				RenderManager.checkRequestTiles(
					multiPageViewLayout.getCurrentCoordList().slice(),
				);
			};

			const lastRequestedKeys = (): string[] => {
				nodeassert.ok(
					sentTileCombineRequests.length > 0,
					'expected a new tile-combine request to be recorded',
				);
				return sentTileCombineRequests[sentTileCombineRequests.length - 1]
					.map(function (c: any) {
						return c.key();
					})
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
					const tile = tileManager.get(coords);
					nodeassert.ok(
						tile,
						label + ': tile ' + coords.key() + ' should exist after request',
					);
					simulateTileArrival(tileManager, tile);
				}

				for (const coords of visibleCoords()) {
					nodeassert.ok(
						renderedKeys.has(coords.key()),
						label +
							': visible tile ' +
							coords.key() +
							' has no bitmap produced',
					);
					nodeassert.ok(
						tileManager.get(coords).isReadyToDraw(),
						label + ': visible tile ' + coords.key() + ' is not ready to draw',
					);
				}

				const beforeNoOp = sentTileCombineRequests.length;
				triggerRequest();
				nodeassert.strictEqual(
					sentTileCombineRequests.length,
					beforeNoOp,
					label +
						': no new tile-combine request expected once all ' +
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
					'0:0:10:0:0',
					'0:256:10:0:0',
					'0:512:10:0:0',
					'0:768:10:0:0',
					'256:0:10:0:0',
					'256:256:10:0:0',
					'256:512:10:0:0',
					'256:768:10:0:0',
					'512:0:10:0:0',
					'512:256:10:0:0',
					'512:512:10:0:0',
					'512:768:10:0:0',
					'768:0:10:0:0',
					'768:256:10:0:0',
					'768:512:10:0:0',
					'768:768:10:0:0',
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
				clearRenderedTiles(tileManager, renderedKeys);
				multiPageViewLayout.scrollProperties.viewY = 768;

				const expectedAfterScroll = [
					'0:512:10:0:0',
					'0:768:10:0:0',
					'0:1024:10:0:0',
					'0:1280:10:0:0',
					'0:1536:10:0:0',
					'256:512:10:0:0',
					'256:768:10:0:0',
					'256:1024:10:0:0',
					'256:1280:10:0:0',
					'256:1536:10:0:0',
					'512:512:10:0:0',
					'512:768:10:0:0',
					'512:1024:10:0:0',
					'512:1280:10:0:0',
					'512:1536:10:0:0',
					'768:512:10:0:0',
					'768:768:10:0:0',
					'768:1024:10:0:0',
					'768:1280:10:0:0',
					'768:1536:10:0:0',
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
				clearRenderedTiles(tileManager, renderedKeys);
				app.pixelsToTwips = 70;
				app.twipsToPixels = 1 / 70;
				multiPageViewLayout.scrollProperties.viewX = 0;
				multiPageViewLayout.scrollProperties.viewY = 0;
				(multiPageViewLayout as any).resetViewLayout();

				const expectedZoomOut = [
					'0:0:10:0:0',
					'0:256:10:0:0',
					'0:512:10:0:0',
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
					tileManager.reclaimTileBitmapMemory(tileManager.get(coords));
					nodeassert.ok(
						!renderedKeys.has(coords.key()),
						'reclaim: tile ' +
							coords.key() +
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
						'reclaim: still-rendered tile ' +
							coords.key() +
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

		it('MultiPage invalidation triggers re-request for affected tiles only', function () {
			const ctx = createTestContext(setupMultiPageForTest());
			try {
				assertInvalidationCycle(ctx);
			} finally {
				ctx.restore();
			}
		});

		it('MultiPage forceKeyframe causes tile re-request', function () {
			const ctx = createTestContext(setupMultiPageForTest());
			try {
				assertForceKeyframeCycle(ctx);
			} finally {
				ctx.restore();
			}
		});

		// ----------------------------------------------------------------
		// When the viewport intersects two pages, tiles from BOTH pages
		// must appear in the request. The page-1-only tiles (y=2304) are
		// the canary: if the page intersection check in
		// refreshCurrentCoordList misses the second page, those tiles
		// disappear and the bottom sliver of the viewport goes blank.
		//
		// At pixelsToTwips=7.5 with viewport 1024x768:
		//   page 0 view rect: (12, 20) .. (1012, 2020)
		//   page 1 view rect: (12, 2040) .. (1012, 4040)
		//   viewY=1280 -> viewport y 1280..2048, overlapping both pages.
		//
		//   page 0 doc visible: y 1260..2000 -> tiles y {1024..2048}
		//   page 1 doc visible: y 2267..2275 -> tiles y {2048, 2304}
		//   After dedup: y=2048 shared, y=2304 is page-1-only.
		// ----------------------------------------------------------------
		it('MultiPage scroll to page boundary includes tiles from both pages', function () {
			setupAppStubs(7.5);

			const activeDocument = new DocumentBase();
			(activeDocument as any)._fileSize = new cool.SimplePoint(7500, 49000);
			app.activeDocument = activeDocument;

			const layout = new ViewLayoutMultiPage();

			const tileManager: any = resetRenderManagerState();

			const tileCombine = instrumentTileCombineRequests(tileManager);
			const rendering = instrumentTileRendering(tileManager);

			app.file.writer.pageRectangleList = [
				[0, 0, 7500, 15000],
				[0, 17000, 7500, 15000],
				[0, 34000, 7500, 15000],
			];

			layout.scrollProperties.viewX = 0;
			layout.scrollProperties.viewY = 0;
			(layout as any).resetViewLayout();

			try {
				layout.scrollProperties.viewY = 1280;

				// These tiles come exclusively from page 1's document
				// region and would be missing if page intersection
				// detection failed.
				const expectedPage1Only = [
					'0:2304:10:0:0',
					'256:2304:10:0:0',
					'512:2304:10:0:0',
					'768:2304:10:0:0',
				];

				const expectedAll = [
					'0:1024:10:0:0',
					'0:1280:10:0:0',
					'0:1536:10:0:0',
					'0:1792:10:0:0',
					'0:2048:10:0:0',
					'0:2304:10:0:0',
					'256:1024:10:0:0',
					'256:1280:10:0:0',
					'256:1536:10:0:0',
					'256:1792:10:0:0',
					'256:2048:10:0:0',
					'256:2304:10:0:0',
					'512:1024:10:0:0',
					'512:1280:10:0:0',
					'512:1536:10:0:0',
					'512:1792:10:0:0',
					'512:2048:10:0:0',
					'512:2304:10:0:0',
					'768:1024:10:0:0',
					'768:1280:10:0:0',
					'768:1536:10:0:0',
					'768:1792:10:0:0',
					'768:2048:10:0:0',
					'768:2304:10:0:0',
				].sort();

				(layout as any).refreshCurrentCoordList();
				RenderManager.checkRequestTiles(layout.getCurrentCoordList().slice());

				const requestedKeys = tileCombine.sentRequests[
					tileCombine.sentRequests.length - 1
				]
					.map(function (c: any) {
						return c.key();
					})
					.sort();

				assertSubset('page-boundary', expectedAll, requestedKeys);

				// Verify page-1-only tiles explicitly.
				const requestedSet = new Set(requestedKeys);
				for (const key of expectedPage1Only) {
					nodeassert.ok(
						requestedSet.has(key),
						'page-1-only tile ' + key + ' must be present at page boundary',
					);
				}

				// Full round-trip: render and verify all visible tiles
				// are drawable.
				for (const coords of layout.getCurrentCoordList().slice()) {
					simulateTileArrival(tileManager, tileManager.get(coords));
				}

				for (const coords of layout.getCurrentCoordList().slice()) {
					nodeassert.ok(
						tileManager.get(coords).isReadyToDraw(),
						'boundary tile ' +
							coords.key() +
							' must be ready to draw after arrival',
					);
				}
			} finally {
				rendering.restore();
				tileCombine.restore();
			}
		});

		it('MultiPage invalidation during pending response is not lost', function () {
			const ctx = createTestContext(setupMultiPageForTest());
			try {
				assertRaceProtection(ctx);
			} finally {
				ctx.restore();
			}
		});

		// ================================================================
		// ViewLayoutCompareChanges tests
		// ================================================================

		// CompareChanges duplicates every base tile with LeftSide and
		// RightSide modes. Verify that both sets are present with
		// identical base coordinates, and that the hand-derived tile
		// grid matches the production output.
		//
		// At pixelsToTwips=7.5 with _viewedRectangle from
		// fromCorePixels([-15, -20, 1030, 768]):
		//   startX = floor(-15/256)*256 = -256
		//   startY = floor(-20/256)*256 = -256
		//   columnCount = ceil(1030/256) = 5, rowCount = ceil(768/256) = 3
		//   j=0..3 -> y = {-256, 0, 256, 512}; y=-256 invalid
		//   isValidTile drops x<0, y<0, x>=1024 (1024*7.5=7680>=7500)
		//   Valid base: x in {0,256,512,768}, y in {0,256,512} = 12
		//   After mode duplication: 12 LeftSide + 12 RightSide = 24
		it('CompareChanges produces left and right mode tiles', function () {
			const ctx = createTestContext(setupCompareChangesForTest());
			try {
				(ctx.layout as any).refreshCurrentCoordList();
				const coordList: TileCoordData[] = ctx.layout
					.getCurrentCoordList()
					.slice();

				const leftKeys = new Set<string>();
				const rightKeys = new Set<string>();

				for (const coords of coordList) {
					const base =
						coords.x + ':' + coords.y + ':' + coords.z + ':' + coords.part;
					if (coords.mode === TileMode.LeftSide) {
						leftKeys.add(base);
					} else if (coords.mode === TileMode.RightSide) {
						rightKeys.add(base);
					}
				}

				nodeassert.ok(leftKeys.size > 0, 'should have LeftSide tiles');
				nodeassert.ok(rightKeys.size > 0, 'should have RightSide tiles');
				nodeassert.deepStrictEqual(
					Array.from(leftKeys).sort(),
					Array.from(rightKeys).sort(),
					'LeftSide and RightSide must cover identical coordinates',
				);
				nodeassert.strictEqual(
					coordList.length,
					leftKeys.size + rightKeys.size,
					'total tiles must equal left + right (no extra modes)',
				);

				// Verify against hand-derived expected base coordinates.
				const expectedBases = [
					'0:0:10:0',
					'0:256:10:0',
					'0:512:10:0',
					'256:0:10:0',
					'256:256:10:0',
					'256:512:10:0',
					'512:0:10:0',
					'512:256:10:0',
					'512:512:10:0',
					'768:0:10:0',
					'768:256:10:0',
					'768:512:10:0',
				].sort();

				nodeassert.deepStrictEqual(
					Array.from(leftKeys).sort(),
					expectedBases,
					'LeftSide base coordinates must match hand-derived grid',
				);
			} finally {
				ctx.restore();
			}
		});

		it('CompareChanges invalidation triggers re-request for affected tiles only', function () {
			const ctx = createTestContext(setupCompareChangesForTest());
			try {
				assertInvalidationCycle(ctx);
			} finally {
				ctx.restore();
			}
		});

		it('CompareChanges forceKeyframe causes tile re-request', function () {
			const ctx = createTestContext(setupCompareChangesForTest());
			try {
				assertForceKeyframeCycle(ctx);
			} finally {
				ctx.restore();
			}
		});

		it('CompareChanges invalidation during pending response is not lost', function () {
			const ctx = createTestContext(setupCompareChangesForTest());
			try {
				assertRaceProtection(ctx);
			} finally {
				ctx.restore();
			}
		});

		// ================================================================
		// View-changes transitions (docdispatcher)
		// ================================================================

		// The tile render mode must follow the active layout as soon as the
		// user switches view, without waiting for the next server status
		// message. The normal view renders in the standard mode (0); the
		// side-by-side compare view renders in the two redline modes (1 and
		// 2). If the mode lagged behind the layout, tiles would be requested
		// in the wrong mode or dropped as belonging to an inactive mode, so
		// the view would stay blank until a later status message caught up.
		it('View-changes transitions set the tile render mode to match the layout', function () {
			setupAppStubs(7.5);

			// The view-changes actions reach for a few more map hooks than the
			// pure layout tests need. Provide no-op stand-ins so the actions run
			// without a live map.
			const map: any = app.map;
			map.fire = function () {};
			map.sendUnoCommand = function () {};
			map._docLayer._docType = 'text';
			map._docLayer._fitWidthZoom = function () {};
			const itemValues: Record<string, string> = {};
			map.stateChangeHandler = {
				setItemValue: function (key: string, value: string) {
					itemValues[key] = value;
				},
				getItemValue: function (key: string) {
					return itemValues[key];
				},
			};
			(app.sectionContainer as any).requestReDraw = function () {};

			const originalWindowMode = (window as any).mode;
			(window as any).mode = {
				isSmallScreenDevice: function () {
					return false;
				},
			};

			const activeDocument = new DocumentBase();
			(activeDocument as any)._fileSize = new cool.SimplePoint(7500, 49000);
			app.activeDocument = activeDocument;
			app.activeDocument.activeLayout = new ViewLayoutWriter();

			// Keep the mode bookkeeping isolated from the tile pipeline: neutralise
			// the redraw that the switch to the normal layout runs, and drop the
			// deferred task the compare layout queues while being constructed.
			const tileManager: any = resetRenderManagerState();
			tileManager.redraw = function () {};
			const layoutingService: any = app.layoutingService;
			const originalAppendLayoutingTask = layoutingService.appendLayoutingTask;
			layoutingService.appendLayoutingTask = function () {};

			const dispatcher: any = new (app.definitions as any)['dispatcher'](
				'text',
			);

			try {
				// Radio menu: entering side-by-side activates both redline modes.
				dispatcher.actionsMap['viewchanges-sidebyside']();
				nodeassert.strictEqual(
					app.activeDocument.activeLayout.type,
					'ViewLayoutCompareChanges',
					'side-by-side must switch to the compare layout',
				);
				nodeassert.deepStrictEqual(
					app.activeDocument.activeModes,
					[1, 2],
					'side-by-side must activate the two redline modes',
				);

				// Radio menu: the inline (normal) view returns to the standard mode.
				dispatcher.actionsMap['viewchanges-inline']();
				nodeassert.strictEqual(
					app.activeDocument.activeLayout.type,
					'ViewLayoutWriter',
					'inline must switch back to the normal layout',
				);
				nodeassert.deepStrictEqual(
					app.activeDocument.activeModes,
					[0],
					'inline view must activate the standard mode',
				);

				// Toggle button: normal to side-by-side.
				dispatcher.actionsMap['comparechanges']();
				nodeassert.deepStrictEqual(
					app.activeDocument.activeModes,
					[1, 2],
					'compare toggle into side-by-side must activate the redline modes',
				);

				// Toggle button: side-by-side back to normal.
				dispatcher.actionsMap['comparechanges']();
				nodeassert.deepStrictEqual(
					app.activeDocument.activeModes,
					[0],
					'compare toggle back to normal must activate the standard mode',
				);
			} finally {
				layoutingService.appendLayoutingTask = originalAppendLayoutingTask;
				(window as any).mode = originalWindowMode;
			}
		});

		// ================================================================
		// Scroll position (scrollProperties.viewX / viewY)
		// ================================================================

		// The viewport bounds say which part of the document is on screen, in
		// document core pixels, so they have to move as the view scrolls.
		it('Calc: the viewport follows the scroll position', function () {
			const { layout } = setupCalcForTest();

			layout.scrollTo(240, 300);

			const bounds = layout.getViewportCorePixelBounds();
			nodeassert.strictEqual(bounds.min.x, 240, 'viewport left edge');
			nodeassert.strictEqual(bounds.min.y, 300, 'viewport top edge');
			nodeassert.strictEqual(bounds.max.x, 240 + 1024, 'viewport right edge');
			nodeassert.strictEqual(bounds.max.y, 300 + 768, 'viewport bottom edge');
		});

		it('Calc: the viewed rectangle is the frame placed at the scroll position', function () {
			const { layout } = setupCalcForTest();

			layout.scrollTo(240, 300);

			nodeassert.deepStrictEqual(
				layout.viewedRectangle.pToArray(),
				[240, 300, 1024, 768],
			);
		});

		it('Calc: a frame with no size leaves the viewed rectangle where it is', function () {
			const { layout, anchor } = setupCalcForTest();

			layout.scrollTo(0, 300);
			const rectangleBefore = layout.viewedRectangle.toArray();

			// The document container is hidden, or has not been laid out yet.
			anchor.size = [0, 0];
			layout.rebuildViewedRectangle();

			nodeassert.deepStrictEqual(
				layout.viewedRectangle.toArray(),
				rectangleBefore,
			);
		});

		it('Calc: the document end stays at the view edge when the frame grows', function () {
			const { layout, anchor } = setupCalcForTest();

			// Scroll past the end of the sheet, which lands on the last position the
			// document spans.
			layout.scrollTo(0, 100000);
			nodeassert.strictEqual(
				layout.scrollProperties.viewY + 768,
				5000,
				'the sheet ends at the bottom of the view',
			);

			// A bigger window, or a panel that closes, leaves less document below the
			// current position than the frame now shows.
			anchor.size = [1024, 1200];
			layout.rebuildViewedRectangle();

			nodeassert.strictEqual(
				layout.scrollProperties.viewY + 1200,
				5000,
				'the sheet still ends at the bottom of the view',
			);
			nodeassert.strictEqual(layout.viewedRectangle.pY1, 3800);
		});

		it('Calc: the scroll position stays inside the document', function () {
			const { layout } = setupCalcForTest();

			layout.scrollTo(0, 4000);

			// The extent shrinks under the current position, as it does when the
			// sheet's used area gets smaller.
			layout.viewSize = cool.SimplePoint.fromCorePixels([5000, 3000]);
			layout.rebuildViewedRectangle();
			nodeassert.strictEqual(layout.scrollProperties.viewY, 3000 - 768);

			// The first row is the top of the scrollable range.
			layout.scrollProperties.viewY = -50;
			layout.rebuildViewedRectangle();
			nodeassert.strictEqual(layout.scrollProperties.viewY, 0);
		});

		it('Calc: the scroll bar thumb follows the scroll position', function () {
			const { layout } = setupCalcForTest();
			const scrollProps = layout.scrollProperties;

			layout.refreshScrollProperties();
			const thumbAtTop = scrollProps.startY;

			layout.scrollTo(0, 2000);
			layout.refreshScrollProperties();
			nodeassert.ok(
				scrollProps.startY > thumbAtTop,
				'the thumb moved down with the view',
			);

			layout.scrollTo(0, 100000);
			layout.refreshScrollProperties();
			const thumbEnd = scrollProps.startY + scrollProps.verticalScrollSize;
			const trackEnd = scrollProps.yOffset + scrollProps.verticalScrollLength;
			nodeassert.ok(
				Math.abs(thumbEnd - trackEnd) <= 1,
				'at the end of the sheet the thumb sits at the end of its track: ' +
					thumbEnd +
					' against ' +
					trackEnd,
			);
		});

		it('Calc: a zoom keeps the anchor point in the middle of the view', function () {
			const { layout } = setupCalcForTest();
			const docLayer: any = app.map._docLayer;

			// The stubs pin the twips-per-pixel factor, so the rebuild that a zoom
			// runs is where the new scale is set. This one halves the scale.
			docLayer._updateTileTwips = function () {
				app.pixelsToTwips = 30;
				app.twipsToPixels = 1 / 30;
			};

			const anchorTwips = new cool.SimplePoint(40000, 40000);
			layout.applyZoom(9, anchorTwips);

			// One core pixel is 30 twips at the new scale, and the position is kept
			// in whole pixels.
			const center = layout.viewedRectangle.center;
			nodeassert.ok(
				Math.abs(center[0] - anchorTwips.x) <= 30,
				'anchor x against view center: ' + anchorTwips.x + ', ' + center[0],
			);
			nodeassert.ok(
				Math.abs(center[1] - anchorTwips.y) <= 30,
				'anchor y against view center: ' + anchorTwips.y + ', ' + center[1],
			);

			// The zoom moved the scroll position as well, so the viewport starts
			// where the viewed rectangle does.
			const bounds = layout.getViewportCorePixelBounds();
			nodeassert.ok(bounds.min.y > 0, 'the view is away from the first row');
			nodeassert.strictEqual(bounds.min.x, layout.viewedRectangle.pX1);
			nodeassert.strictEqual(bounds.min.y, layout.viewedRectangle.pY1);
		});

		// A page-stack layout builds its viewed rectangle from the pages that are on
		// screen, so a zoom leaves the scroll position alone.
		it('Stacked-page layouts keep their scroll position across a zoom', function () {
			const layout = setupMultiPageForTest();

			layout.scrollProperties.viewY = 768;
			const rectangleBefore = layout.viewedRectangle.toArray();

			layout.setViewRectangleFromPointAndScale(
				new cool.SimplePoint(1000, 1000),
				app.twipsToPixels,
			);

			nodeassert.strictEqual(layout.scrollProperties.viewY, 768);
			nodeassert.deepStrictEqual(
				layout.viewedRectangle.toArray(),
				rectangleBefore,
			);
		});

		// ================================================================
		// Visible-area bookkeeping
		// ================================================================

		it('The pan direction follows the direction of the last scroll', function () {
			const layout = setupBaseLayoutForTest();

			layout.scroll(0, 400);
			nodeassert.strictEqual(
				layout.getLastPanDirection()[1],
				1,
				'scrolling down',
			);

			layout.scroll(0, -200);
			nodeassert.strictEqual(
				layout.getLastPanDirection()[1],
				-1,
				'scrolling up',
			);
		});

		// Tiles are fetched a little beyond the visible area, further on the side the
		// view is heading for.
		it('The pre-fetch range grows towards the side the view is moving to', function () {
			const layout = setupBaseLayoutForTest();

			// Scroll down first, so that the scroll up afterwards has room.
			layout.scroll(0, 2000);
			layout.scroll(0, -1000);

			const range = new cool.Bounds(
				new cool.Point(0, 10),
				new cool.Point(4, 14),
			);
			const expanded = RenderManager.expandTileRange(range);

			const grewAbove = range.min.y - expanded.min.y;
			const grewBelow = expanded.max.y - range.max.y;
			nodeassert.ok(
				grewAbove > grewBelow,
				'more rows of tiles above the view than below: ' +
					grewAbove +
					' against ' +
					grewBelow,
			);
		});

		it('The visible area is sent again after a reset, changed or not', function () {
			const layout = setupBaseLayoutForTest();

			const sentMessages: string[] = [];
			(app.socket as any).sendMessage = function (message: string) {
				sentMessages.push(message);
			};
			(app.socket as any).connected = function () {
				return true;
			};

			layout.viewedRectangle = cool.SimpleRectangle.fromCorePixels([
				0, 400, 1024, 768,
			]);
			(app.map as any)._docLoaded = true;

			layout.sendClientVisibleArea();
			nodeassert.strictEqual(sentMessages.length, 1, 'a new area is sent');

			layout.sendClientVisibleArea();
			nodeassert.strictEqual(
				sentMessages.length,
				1,
				'an area the server already has is not sent again',
			);

			// A reconnection or a user interface mode change resets the area, and the
			// server has to hear it again even though it did not change.
			layout.resetClientVisibleArea();
			layout.sendClientVisibleArea();
			nodeassert.strictEqual(sentMessages.length, 2, 'the area is sent again');
		});

		it('CompareChanges tracks the previous visible area and asks for a redraw', function () {
			const layout = setupCompareChangesForTest();
			const container: any = app.sectionContainer;
			const originalRequestReDraw = container.requestReDraw;

			let redrawRequests = 0;
			container.requestReDraw = function () {
				redrawRequests++;
			};

			try {
				layout.scrollProperties.viewY = 500;
				(layout as any).refreshVisibleAreaRectangle();
				nodeassert.strictEqual(
					layout.getLastPanDirection()[1],
					1,
					'scrolling down',
				);

				layout.scrollProperties.viewY = 300;
				(layout as any).refreshVisibleAreaRectangle();
				nodeassert.strictEqual(
					layout.getLastPanDirection()[1],
					-1,
					'scrolling up',
				);

				nodeassert.strictEqual(
					redrawRequests,
					2,
					'each new visible area asks for a redraw',
				);
			} finally {
				container.requestReDraw = originalRequestReDraw;
			}
		});
	});
});
