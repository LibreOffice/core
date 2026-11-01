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

describe('BitmapTileManager tile memory', function () {
	const assert = require('assert');

	let tileManager: any;
	let posted: any[];
	let savedGetTaskWorkers: any;
	let savedSectionContainer: any;
	let savedTimerRegistry: any;

	beforeEach(function () {
		tileManager = new BitmapTileManager();
		// Distances are set by each test.
		tileManager.updateAllTileDistances = function () {};

		posted = [];
		savedGetTaskWorkers = app.socket.getTaskWorkers;
		app.socket.getTaskWorkers = () => [
			{ postMessage: (message: any) => posted.push(message) } as any,
		];

		// The end of a transaction resumes drawing.
		savedSectionContainer = app.sectionContainer;
		(app as any).sectionContainer = {
			deferDrawing: function () {},
			resumeDrawing: function () {},
			requestReDraw: function () {},
		};

		savedTimerRegistry = (app as any).timerRegistry;
		(app as any).timerRegistry = new TimerRegistry();
	});

	afterEach(function () {
		app.socket.getTaskWorkers = savedGetTaskWorkers;
		(app as any).sectionContainer = savedSectionContainer;
		tileManager.stopIdleTrim();
		(app as any).timerRegistry = savedTimerRegistry;
	});

	// A tile decoded by a worker: a bitmap on the main thread, its key listed in
	// tileImageCache, and the compressed keyframe it was decoded from.
	function decodedTile(x: number, distanceFromView: number): any {
		const tile = tileManager.createTile(
			new TileCoordData(x, 0, 10, '0' as PartNumber, 0),
		);
		tile.distanceFromView = distanceFromView;
		tile.wireId = 1;
		tile.rawDeltas.push(new cool.RawDelta(new Uint8Array(8), 1, true));
		tile.lastPendingId = 1;
		tile.decompressedId = 1;
		const bitmap = { closed: false, close: () => (bitmap.closed = true) };
		tileManager.setBitmapOnTile(tile, bitmap);
		tileManager.tileImageCache.set(tile.coords.toString(), null);
		return { tile, bitmap };
	}

	// Reclaims tell the workers in a microtask, once per task.
	function endOfTask(): Promise<void> {
		return Promise.resolve();
	}

	it('tells the workers once when the document loses focus', async function () {
		const one = decodedTile(0, 5000);
		const two = decodedTile(256, 5000);

		tileManager.reclaimGraphicsMemory();
		await endOfTask();

		assert.strictEqual(posted.length, 1);
		assert.strictEqual(posted[0].message, 'trimImageCache');
		assert.ok(!posted[0].cachedTiles.has(one.tile.coords.toString()));
		assert.ok(!posted[0].cachedTiles.has(two.tile.coords.toString()));
	});

	it('tells the workers when a tile is removed', async function () {
		const kept = decodedTile(0, 0);
		const removed = decodedTile(256, 5000);

		tileManager.removeTile(removed.tile.coords.key());
		await endOfTask();

		assert.strictEqual(posted.length, 1);
		assert.ok(posted[0].cachedTiles.has(kept.tile.coords.toString()));
		assert.ok(!posted[0].cachedTiles.has(removed.tile.coords.toString()));
	});

	it('tells the workers once the decodes in flight are done', async function () {
		decodedTile(256, 5000);
		tileManager.nPendingWorkerTasks = 1;

		tileManager.reclaimGraphicsMemory();
		await endOfTask();
		assert.strictEqual(posted.length, 0);

		tileManager.onWorkerEndTransaction({ data: { deltas: [] } });
		assert.strictEqual(posted.length, 1);
		assert.strictEqual(posted[0].message, 'trimImageCache');
	});

	it('sends the workers nothing when no image was released', async function () {
		tileManager.nPendingWorkerTasks = 1;
		tileManager.reclaimGraphicsMemory();
		tileManager.onWorkerEndTransaction({ data: { deltas: [] } });
		await endOfTask();

		assert.strictEqual(posted.length, 0);
	});

	it('releases the images of the tiles outside the current area', function () {
		const visible = decodedTile(0, 0);
		const offscreen = decodedTile(256, 5000);

		tileManager.trimOffscreenTiles();

		assert.strictEqual(offscreen.tile.image, null);
		assert.ok(offscreen.bitmap.closed);
		assert.strictEqual(visible.tile.image, visible.bitmap);
		assert.ok(!visible.bitmap.closed);
		assert.deepStrictEqual(tileManager.tileBitmapList, [visible.tile]);
	});

	it('keeps the margin that the last view update made current', function () {
		// A tile next to the view: current for the last update, outside the
		// viewport by the strict test.
		const margin = decodedTile(256, 0);
		tileManager.updateAllTileDistances = function () {
			margin.tile.distanceFromView = 5000;
		};

		tileManager.trimOffscreenTiles();

		assert.strictEqual(margin.tile.image, margin.bitmap);
	});

	it('keeps the compressed data, so a trimmed tile can be decoded again', function () {
		const offscreen = decodedTile(256, 5000);

		tileManager.trimOffscreenTiles();

		assert.strictEqual(offscreen.tile.rawDeltas.length, 1);
		assert.ok(offscreen.tile.hasContent());
		assert.ok(!offscreen.tile.needsFetch());
		assert.ok(offscreen.tile.needsRehydration());
	});

	it('releases the images in one pass', async function () {
		decodedTile(0, 5000);
		decodedTile(256, 5000);
		let notifications = 0;
		tileManager.notifyTileStateChanged = () => ++notifications;

		tileManager.trimOffscreenTiles();
		await endOfTask();

		assert.strictEqual(notifications, 1);
		assert.strictEqual(posted.length, 1);
		assert.strictEqual(posted[0].cachedTiles.size, 0);
	});

	describe('timer', function () {
		let trims: number;
		let savedDateNow: any;

		beforeEach(function () {
			installFakeClock();
			savedDateNow = Date.now;
			Date.now = () => clockNow;
			trims = 0;
			tileManager.trimOffscreenTiles = () => ++trims;
		});

		afterEach(function () {
			tileManager.stopIdleTrim();
			Date.now = savedDateNow;
			restoreClock();
		});

		it('trims once the view has been idle for the delay', function () {
			tileManager.noteTileActivity();

			advanceClock(tileManager.idleTrimDelayMs - 1);
			assert.strictEqual(trims, 0);
			advanceClock(1);
			assert.strictEqual(trims, 1);
		});

		it('starts the delay again on each activity', function () {
			tileManager.noteTileActivity();
			advanceClock(tileManager.idleTrimDelayMs - 1);
			tileManager.noteTileActivity();

			advanceClock(tileManager.idleTrimDelayMs - 1);
			assert.strictEqual(trims, 0);
			advanceClock(1);
			assert.strictEqual(trims, 1);
		});

		it('waits for the decodes in flight to finish', function () {
			tileManager.nPendingWorkerTasks = 1;
			tileManager.noteTileActivity();
			advanceClock(tileManager.idleTrimDelayMs);
			assert.strictEqual(trims, 0);

			tileManager.nPendingWorkerTasks = 0;
			tileManager.nPendingBitmaps = 1;
			advanceClock(tileManager.idleTrimDelayMs);
			assert.strictEqual(trims, 0);

			tileManager.nPendingBitmaps = 0;
			advanceClock(tileManager.idleTrimDelayMs);
			assert.strictEqual(trims, 1);
		});

		it('stops when the cache is discarded', function () {
			tileManager.noteTileActivity();

			tileManager.discardAllCache();
			advanceClock(tileManager.idleTrimDelayMs);

			assert.strictEqual(trims, 0);
		});

		it('does not start before a document is loaded', function () {
			const savedMap = app.map;
			(app as any).map = null;
			try {
				tileManager.update();
			} finally {
				(app as any).map = savedMap;
			}
			advanceClock(tileManager.idleTrimDelayMs);

			assert.strictEqual(trims, 0);
		});
	});
});
