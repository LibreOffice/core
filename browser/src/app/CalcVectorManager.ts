/* -*- js-indent-level: 8; fill-column: 100 -*- */
/*
 * Copyright the Collabora Online contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

/*
 * CalcVectorManager - the blocks of cells, drawing objects and cell edits of the sheets as vector
 * primitives, and the fonts and images they name.
 *
 * Content comes in the exact document geometry, which is the same at every zoom. Each block is
 * placed on the rounded cells of the sheet geometry when drawn, so it lines up with the grid and
 * cursor, and a zoom asks for nothing again. A block in view is asked for with the version held,
 * and the engine answers only when it is newer. Changes arrive as pushed deltas.
 */

class CalcVectorManager {
	private static _instance: CalcVectorManager | null = null;

	/// The most blocks kept for one sheet. Blocks out of view go first.
	private static readonly maxBlocks = 512;

	/// The most sheets kept, counting each mode of a sheet on its own.
	private static readonly maxSheets = 4;

	/// The most requests waiting for an answer at once. The engine answers one at a time, and one
	/// extra request keeps it busy.
	private static readonly maxRequestsInFlight = 2;

	/// How long a request waits for its answer, in milliseconds. After that it is dropped and its
	/// blocks can be asked for again.
	private static readonly requestTimeout = 10000;

	/// True when the document is a spreadsheet drawn from vector primitives.
	static isActive(): boolean {
		const docLayer = app.map && app.map._docLayer;
		return (
			!!docLayer &&
			docLayer._docType === 'spreadsheet' &&
			cool.VectorRenderingConfig.isEnabled()
		);
	}

	static instance(): CalcVectorManager {
		if (!CalcVectorManager._instance) {
			CalcVectorManager._instance = new CalcVectorManager();
			// The document state names the manager, for the tests.
			app.calc.vectorManager = CalcVectorManager._instance;
		}
		return CalcVectorManager._instance;
	}

	private _bitmaps = new VectorResourceTracker<number, HTMLImageElement>(
		(checksum) =>
			'commandvalues command=.uno:VectorRenderingGraphics?checksum=' +
			String(checksum),
	);

	// Loaded font faces by font id, registered as family "vecfont-<id>".
	private _fonts = new VectorResourceTracker<string, FontFace>(
		(fontId) => 'commandvalues command=.uno:VectorRenderingFont?id=' + fontId,
	);

	readonly renderer: cool.VectorPrimitiveRenderer =
		new cool.VectorPrimitiveRenderer(
			(checksum) => this._bitmaps.get(checksum),
			(fontId) => this._fonts.has(fontId),
		);

	// Font attributes of cell text by id. An id is stable within an epoch.
	private _textStyles: Map<number, cool.CalcTextStyle> = new Map();

	// What is held of each sheet, keyed by sheet and mode.
	private _sheets: Map<string, cool.CalcSheetData> = new Map();

	// The epoch the held versions count in.
	private _epoch: number | undefined = undefined;

	// Blocks a request is in flight for, keyed by sheet, mode and block.
	private _inFlight: Set<string> = new Set();

	// Sheets that were asked for at least once.
	private _sheetRequested: Set<string> = new Set();

	// For each sheet marked stale, the id of the first request sent after the mark. Only an
	// answer to that request or a later one brings the stale blocks of the sheet up to date.
	private _staleFrom: Map<string, number> = new Map();

	// Sheets marked stale whose drawing objects and settings the next request asks for again,
	// even when it asks for no block.
	private _sheetsBehind: Set<string> = new Set();

	// The blocks of each request in flight, keyed by request id.
	private _requests: Map<
		number,
		{ sheetKey: string; blocks: string[]; sent: number }
	> = new Map();
	private _nextRequestId = 1;

	// Blocks of a sheet waiting to be asked for, most wanted first. Each draw replaces them, so
	// only blocks still in view are asked for.
	private _queue: { part: number; mode: number; keys: string[] } | null = null;

	private _changeListeners: (() => void)[] = [];
	// Callbacks that release graphics memory held for drawing the content.
	private _reclaimListeners: (() => void)[] = [];
	// Counts up each time a font or an image the content names has loaded.
	private _resourceVersion = 0;
	private _visualsReported = false;

	private _listensToGeometry = false;
	// Counts up on every change of the sheet geometry, the zoom among them.
	private _geometryCount = 0;

	// Blocks placed at the zoom of the view, keyed by sheet and block. The pixel size is in tile
	// twips.
	private _placedBlocks: Map<
		string,
		{
			source: cool.CalcBlockEntry<cool.CalcCellRect>;
			count: number;
			pixel: number;
			placed: cool.CalcBlockEntry;
		}
	> = new Map();

	/// The mode content is asked in: cell positions, which the view places at any zoom, so a
	/// column or a row that changes size moves the content after it without sending it again.
	static readonly contentMode = -1;

	onChanged(callback: () => void): void {
		this._changeListeners.push(callback);
	}

	/// Call back to release graphics memory, such as when the document is hidden.
	onReclaimGraphicsMemory(callback: () => void): void {
		this._reclaimListeners.push(callback);
	}

	offReclaimGraphicsMemory(callback: () => void): void {
		const index = this._reclaimListeners.indexOf(callback);
		if (index >= 0) this._reclaimListeners.splice(index, 1);
	}

	/// Release graphics memory. What is still needed is drawn again when shown.
	reclaimGraphicsMemory(): void {
		for (const listener of this._reclaimListeners) listener();
	}

	offChanged(callback: () => void): void {
		const index = this._changeListeners.indexOf(callback);
		if (index >= 0) this._changeListeners.splice(index, 1);
	}

	private _fireChanged(): void {
		for (const listener of this._changeListeners) listener();
	}

	/// Counts up each time a font or image that the content names has loaded.
	resourceVersion(): number {
		return this._resourceVersion;
	}

	private _listenToGeometry(): void {
		if (this._listensToGeometry) return;
		this._listensToGeometry = true;
		app.map.on('sheetgeometrychanged', () => {
			this._geometryCount++;
		});
	}

	/// The block placed at the view zoom, in tile twips from its first cell. Undefined while the
	/// geometry of its sheet is not known.
	placedBlock(
		sheet: cool.CalcSheetData,
		block: cool.CalcBlockEntry<cool.CalcCellRect>,
	): cool.CalcBlockEntry | undefined {
		const docLayer = app.map && app.map._docLayer;
		const geometry: cool.SheetGeometry | undefined = docLayer
			? docLayer.sheetGeometry
			: undefined;
		if (!geometry || geometry.getPart() !== sheet.part || !app.tile.size)
			return undefined;
		this._listenToGeometry();

		const key =
			CalcVectorManager._sheetKey(sheet.part, sheet.mode) +
			'/' +
			cool.calcBlockKey(block.blockRow, block.blockColumn);
		const pixel = app.tile.size.x / RenderManager.tileSize;
		const held = this._placedBlocks.get(key);
		if (
			held &&
			held.source === block &&
			held.count === this._geometryCount &&
			held.pixel === pixel
		)
			return held.placed;
		const placed = cool.calcPlacedBlock(block, geometry, pixel);
		if (!placed) return undefined;
		this._placedBlocks.set(key, {
			source: block,
			count: this._geometryCount,
			pixel,
			placed,
		});
		return placed;
	}

	// Forget the placed blocks of a sheet, or of one block of it.
	private _forgetPlaced(sheetKey: string, blockKey?: string): void {
		if (blockKey !== undefined) {
			this._placedBlocks.delete(sheetKey + '/' + blockKey);
			return;
		}
		for (const key of Array.from(this._placedBlocks.keys()))
			if (key.startsWith(sheetKey + '/')) this._placedBlocks.delete(key);
	}

	private static _sheetKey(part: number, mode: number): string {
		return String(part) + ':' + String(mode);
	}

	// The owner name of the fonts and images of an entry, released with it.
	private static _resourceOwner(sheetKey: string, entryKey: string): string {
		return 'calc:' + sheetKey + ':' + entryKey;
	}

	getSheet(part: number, mode: number): cool.CalcSheetData | undefined {
		return this._sheets.get(CalcVectorManager._sheetKey(part, mode));
	}

	private _ensureSheet(part: number, mode: number): cool.CalcSheetData {
		const key = CalcVectorManager._sheetKey(part, mode);
		let sheet = this._sheets.get(key);
		if (!sheet) {
			sheet = {
				part: part,
				mode: mode,
				version: 0,
				complete: false,
				blockRows: 64,
				blockColumns: 16,
				sheet: undefined,
				blocks: new Map(),
				staleBlocks: new Set(),
				objects: new Map(),
				drawOrder: [],
				objectBlocks: [],
				textEdits: [],
			};
			this._sheets.set(key, sheet);
		}
		return sheet;
	}

	/// Ask for the blocks that are not held or may be behind, most wanted first, replacing the
	/// earlier queue. Each missing block gets a request of its own, so the first one shows as soon
	/// as the engine has built it.
	requestBlocks(
		part: number,
		mode: number,
		blocks: Array<[number, number]>,
	): void {
		const sheet = this._ensureSheet(part, mode);
		const sheetKey = CalcVectorManager._sheetKey(part, mode);
		const keys: string[] = [];
		for (const [blockRow, blockColumn] of blocks) {
			const key = cool.calcBlockKey(blockRow, blockColumn);
			if (keys.indexOf(key) < 0 && this._isWanted(sheet, sheetKey, key))
				keys.push(key);
		}
		this._queue = { part, mode, keys };

		// The first request brings the drawing objects and settings of the sheet, even with no
		// block.
		if (
			keys.length === 0 &&
			(!this._sheetRequested.has(sheetKey) || this._sheetsBehind.has(sheetKey))
		)
			this._sendRequest(part, mode, []);
		this._sendQueued();
	}

	/// Whether a block is missing or stale and not already asked for.
	private _isWanted(
		sheet: cool.CalcSheetData,
		sheetKey: string,
		key: string,
	): boolean {
		if (this._inFlight.has(sheetKey + '/' + key)) return false;
		return !sheet.blocks.has(key) || sheet.staleBlocks.has(key);
	}

	/// Ask for the queued blocks, one per request, up to the limit in flight. A request for another
	/// sheet gives its place to the sheet shown, and its answer is still taken when it comes.
	private _sendQueued(): void {
		const queue = this._queue;
		if (!queue) return;
		const sheetKey = CalcVectorManager._sheetKey(queue.part, queue.mode);
		const sheet = this._sheets.get(sheetKey);
		if (!sheet) {
			this._queue = null;
			return;
		}
		this._expireRequests();
		while (queue.keys.length > 0) {
			const key = queue.keys[0];
			// The block may have arrived in a push since it was queued.
			if (!this._isWanted(sheet, sheetKey, key)) {
				queue.keys.shift();
				continue;
			}
			if (
				this._requests.size >= CalcVectorManager.maxRequestsInFlight &&
				!this._forgetRequestOfOtherSheet(sheetKey)
			)
				break;
			queue.keys.shift();
			const keys = [key];
			// A held block is drawn already and asked again only to confirm its version, so all
			// held blocks queued go in one request. A missing block gets its own, to show as soon
			// as it is built.
			if (sheet.blocks.has(key)) {
				queue.keys = queue.keys.filter((other) => {
					if (!sheet.blocks.has(other)) return true;
					if (this._isWanted(sheet, sheetKey, other)) keys.push(other);
					return false;
				});
			}
			this._sendRequest(queue.part, queue.mode, keys);
		}
	}

	// Forget the oldest request for another sheet. True when there was one.
	private _forgetRequestOfOtherSheet(sheetKey: string): boolean {
		for (const [requestId, request] of this._requests) {
			if (request.sheetKey === sheetKey) continue;
			this._forgetRequest(requestId, request);
			return true;
		}
		return false;
	}

	/// Drop timed out requests so their blocks can be asked for again. A late answer is still
	/// taken. True when a request was dropped.
	private _expireRequests(): boolean {
		const now = performance.now();
		let expired = false;
		for (const [requestId, request] of Array.from(this._requests.entries())) {
			if (now - request.sent < CalcVectorManager.requestTimeout) continue;
			this._forgetRequest(requestId, request);
			expired = true;
		}
		return expired;
	}

	private _forgetRequest(
		requestId: number,
		request: { sheetKey: string; blocks: string[] },
	): void {
		this._requests.delete(requestId);
		for (const key of request.blocks)
			this._inFlight.delete(request.sheetKey + '/' + key);
	}

	// The timer that runs when the oldest request waiting reaches its time.
	private _expiryTimer: ReturnType<typeof setTimeout> | undefined = undefined;

	/// Arm the timer for the oldest waiting request, so the next blocks are asked for even when the
	/// view is not drawn meanwhile.
	private _armExpiry(): void {
		if (this._expiryTimer !== undefined || this._requests.size === 0) return;
		let oldest = Infinity;
		for (const request of this._requests.values())
			oldest = Math.min(oldest, request.sent);
		const wait = Math.max(
			0,
			oldest + CalcVectorManager.requestTimeout - performance.now(),
		);
		this._expiryTimer = setTimeout(() => this._retireExpired(), wait + 1);
	}

	private _disarmExpiry(): void {
		if (this._expiryTimer === undefined) return;
		clearTimeout(this._expiryTimer);
		this._expiryTimer = undefined;
	}

	// Drawing again asks anew for the blocks of the timed out requests.
	private _retireExpired(): void {
		this._expiryTimer = undefined;
		if (this._expireRequests()) this._fireChanged();
		this._sendQueued();
		this._armExpiry();
	}

	/// Ask for blocks of a sheet, and for changes to its objects and settings.
	private _sendRequest(part: number, mode: number, keys: string[]): void {
		const sheet = this._ensureSheet(part, mode);
		const sheetKey = CalcVectorManager._sheetKey(part, mode);
		const names: string[] = [];
		for (const key of keys) {
			this._inFlight.add(sheetKey + '/' + key);
			const held = sheet.blocks.get(key);
			// A held block is answered with the parts changed after its version.
			names.push(held ? key + ':' + String(held.current) : key);
		}
		this._sheetRequested.add(sheetKey);
		this._sheetsBehind.delete(sheetKey);

		const requestId = this._nextRequestId++;
		this._requests.set(requestId, {
			sheetKey: sheetKey,
			blocks: keys,
			sent: performance.now(),
		});
		this._armExpiry();

		app.socket.sendMessage(
			cool.vectorPrimitivesRequest({
				partId: String(part),
				mode: mode,
				requestId: requestId,
				blocks: names,
				epoch: this._epoch,
				// Only a complete sheet asks for changes since its version. A sheet
				// filled only by pushes asks for its drawing objects whole.
				since: sheet.complete ? sheet.version : undefined,
			}),
		);
	}

	/// Drop blocks out of view once a sheet holds too many. Once a block in view is held, also drop
	/// the other modes of the sheet and the sheets shown longest ago beyond maxSheets.
	evict(part: number, mode: number, keep: Set<string>): void {
		const sheetKey = CalcVectorManager._sheetKey(part, mode);
		const sheet = this._sheets.get(sheetKey);
		if (!sheet) return;

		let holdsVisible = false;
		for (const key of keep) {
			if (sheet.blocks.has(key)) {
				holdsVisible = true;
				break;
			}
		}
		if (holdsVisible) {
			// The sheets are kept in the order last shown, this one last.
			this._sheets.delete(sheetKey);
			this._sheets.set(sheetKey, sheet);
			for (const [key, other] of Array.from(this._sheets.entries())) {
				if (key === sheetKey) continue;
				if (
					other.part === part ||
					this._sheets.size > CalcVectorManager.maxSheets
				)
					this._dropSheet(key, other);
			}
		}

		if (sheet.blocks.size <= CalcVectorManager.maxBlocks) return;
		for (const [key, block] of Array.from(sheet.blocks.entries())) {
			if (keep.has(key)) continue;
			sheet.blocks.delete(key);
			this._forgetPlaced(sheetKey, key);
			sheet.staleBlocks.delete(key);
			this._releaseOwner(CalcVectorManager._resourceOwner(sheetKey, key));
			// The objects of the block go with it, and come back when it is asked for.
			for (const id of Array.from(block.objects))
				this._dropObject(sheet, sheetKey, id);
			if (sheet.blocks.size <= CalcVectorManager.maxBlocks / 2) break;
		}
	}

	private _dropSheet(sheetKey: string, sheet: cool.CalcSheetData): void {
		for (const key of sheet.blocks.keys())
			this._releaseOwner(CalcVectorManager._resourceOwner(sheetKey, key));
		for (const id of sheet.objects.keys())
			this._releaseOwner(CalcVectorManager._resourceOwner(sheetKey, 'o' + id));
		this._releaseOwner(CalcVectorManager._resourceOwner(sheetKey, 'edits'));
		this._sheets.delete(sheetKey);
		this._forgetPlaced(sheetKey);
		this._sheetRequested.delete(sheetKey);
		this._staleFrom.delete(sheetKey);
		this._sheetsBehind.delete(sheetKey);
		for (const flightKey of Array.from(this._inFlight))
			if (flightKey.startsWith(sheetKey + '/'))
				this._inFlight.delete(flightKey);
		// A later answer for the sheet finds no request and is dropped.
		for (const [requestId, request] of Array.from(this._requests.entries()))
			if (request.sheetKey === sheetKey) this._requests.delete(requestId);
		if (
			this._queue &&
			CalcVectorManager._sheetKey(this._queue.part, this._queue.mode) ===
				sheetKey
		)
			this._queue = null;
	}

	/// Mark everything held as possibly behind, after the client was away. It is still drawn, and
	/// asked for again with the versions held.
	markAllStale(): void {
		for (const [sheetKey, sheet] of this._sheets)
			this._markSheetStale(sheetKey, sheet);
		this._inFlight.clear();
		this._requests.clear();
		this._disarmExpiry();
		this._queue = null;
		this._sheetRequested.clear();
		this._fireChanged();
	}

	/// Mark one sheet as possibly behind, to ask again with the versions held.
	markPartStale(part: number): void {
		for (const [sheetKey, sheet] of this._sheets) {
			if (sheet.part !== part) continue;
			this._markSheetStale(sheetKey, sheet);
			// The next request asks again for the drawing objects and settings of the sheet.
			this._sheetsBehind.add(sheetKey);
		}
		this._fireChanged();
	}

	// Mark the blocks held and the blocks asked for as stale. The answers to the requests
	// already sent leave the marks in place.
	private _markSheetStale(sheetKey: string, sheet: cool.CalcSheetData): void {
		for (const key of sheet.blocks.keys()) sheet.staleBlocks.add(key);
		for (const flightKey of this._inFlight)
			if (flightKey.startsWith(sheetKey + '/'))
				sheet.staleBlocks.add(flightKey.substring(sheetKey.length + 1));
		this._staleFrom.set(sheetKey, this._nextRequestId);
	}

	/// Forget all content, keeping the fonts and images that are loaded.
	clearAll(): void {
		this._textStyles.clear();
		this._sheets.clear();
		this._placedBlocks.clear();
		this._inFlight.clear();
		this._requests.clear();
		this._disarmExpiry();
		this._queue = null;
		this._sheetRequested.clear();
		this._staleFrom.clear();
		this._sheetsBehind.clear();
		this._owners.clear();
		this._bitmaps.forgetAllParts();
		this._fonts.forgetAllParts();
		this._fireChanged();
	}

	/// Handle an answer to .uno:VectorPrimitives or a pushed delta.
	handleVectorPrimitives(values: cool.CalcVectorResponse): void {
		const part = parseInt(values.partId);
		if (isNaN(part)) return;
		const mode = values.mode ?? 0;

		const sheetKey = CalcVectorManager._sheetKey(part, mode);
		const requestId = values.requestId;
		const request =
			requestId !== undefined ? this._requests.get(requestId) : undefined;
		if (request && requestId !== undefined)
			this._forgetRequest(requestId, request);

		// A new epoch makes the held versions invalid. The answer that brings it was built in the
		// new epoch, so it is kept and the rest is asked for again.
		if (this._epoch !== values.epoch) {
			const hadEpoch = this._epoch !== undefined;
			this._epoch = values.epoch;
			if (hadEpoch) this.clearAll();
		}

		// Pushes go to every client holding the sheet in any mode, and an answer can arrive after
		// its request was dropped. Keep both only for a sheet held and asked for in this mode.
		const pushed = requestId === undefined;
		if (
			!request &&
			(!this._sheets.has(sheetKey) || !this._sheetRequested.has(sheetKey))
		)
			return;

		const sheet = this._ensureSheet(part, mode);
		if (request) this._sheetRequested.add(sheetKey);
		if (values.type === 'vectorprimitives') sheet.complete = true;
		// The answer names the blocks asked for and carries those that changed, so all of them are
		// current, even when the request was forgotten before the answer came.
		const asked = values.blocks ?? request?.blocks ?? [];
		// An answer to a request sent before the sheet was marked stale leaves the marks.
		const clearsStale =
			requestId === undefined ||
			requestId >= (this._staleFrom.get(sheetKey) ?? 0);
		if (clearsStale) for (const key of asked) sheet.staleBlocks.delete(key);

		if (values.blockRows) sheet.blockRows = values.blockRows;
		if (values.blockColumns) sheet.blockColumns = values.blockColumns;
		if (values.sheet) sheet.sheet = values.sheet;
		if (values.version !== undefined)
			sheet.version = Math.max(sheet.version, values.version);

		for (const [id, style] of Object.entries(values.textStyles ?? {}))
			this._textStyles.set(Number(id), style);

		const checksums = new Set<number>();
		const fontIds = new Set<string>();
		// The version of the sheet that the answer brings what is held up to.
		const current = values.version ?? 0;
		// The blocks whose update builds on parts that are not held.
		const behind = new Set<string>();
		for (const entry of values.objects ?? []) {
			if (entry.kind !== 'cells') continue;
			const key = cool.calcBlockKey(entry.blockRow, entry.blockColumn);
			const held = sheet.blocks.get(key);
			// A push carries changed blocks for all clients. Keep only held ones.
			if (pushed && !held) continue;
			if (held && held.current >= entry.version) continue;
			for (const part of [entry.borders, ...(entry.rows ?? [])]) {
				const layers = part?.content.layers ?? {};
				for (const layer of Object.values(layers))
					cool.calcApplyTextStyles(layer, this._textStyles);
			}
			const block = cool.calcUpdatedBlock(held, entry, current);
			if (!block) {
				// The update builds on missing parts, so ask for the block again.
				sheet.staleBlocks.add(key);
				behind.add(key);
				continue;
			}
			sheet.blocks.set(key, block);
			if (clearsStale) sheet.staleBlocks.delete(key);
			this._collectResources(
				CalcVectorManager._resourceOwner(sheetKey, key),
				(walker) => {
					const layers = block.content.layers;
					for (const layer of Object.values(layers))
						if (layer) walker.walkPrimitives(layer as cool.Primitive[]);
				},
				checksums,
				fontIds,
			);

			// The objects travel with the block holding their anchor cell. A whole block names all
			// of its objects, so a held one it leaves out has left.
			if (entry.from === 0) {
				const named = new Set((entry.objects ?? []).map((object) => object.id));
				for (const id of Array.from(block.objects))
					if (!named.has(id)) this._leaveBlock(block, id);
			}
			for (const object of entry.objects ?? []) {
				block.objects.add(object.id);
				const heldObject = sheet.objects.get(object.id);
				if (heldObject && heldObject.version >= object.version) continue;
				sheet.objects.set(object.id, object);
				this._collectResources(
					CalcVectorManager._resourceOwner(sheetKey, 'o' + object.id),
					(walker) => walker.walkPrimitives(object.content.primitives),
					checksums,
					fontIds,
				);
			}
			for (const id of entry.removed ?? []) this._leaveBlock(block, id);
		}

		// A block the answer leaves out is unchanged up to the answer version. A push carries every
		// block changed since its base, and a request names each held version. A block older than
		// the push base may miss a change.
		for (const [key, block] of sheet.blocks) {
			if (behind.has(key)) continue;
			if (pushed) {
				if (block.current >= (values.from ?? 0))
					block.current = Math.max(block.current, current);
				else sheet.staleBlocks.add(key);
			} else if (asked.indexOf(key) >= 0)
				block.current = Math.max(block.current, current);
		}

		if (values.objectBlocks) {
			const before = new Map(
				sheet.objectBlocks.map((block) => [
					cool.calcBlockKey(block.blockRow, block.blockColumn),
					block,
				]),
			);
			sheet.objectBlocks = values.objectBlocks;
			// An object that left its block for one not held keeps drawing as it was until that
			// block answers. The blocks whose objects changed are asked for first, so the answer
			// comes with the next round trip.
			if (this._hasObjectWithoutBlock(sheet)) {
				const moved: string[] = [];
				for (const block of values.objectBlocks) {
					const key = cool.calcBlockKey(block.blockRow, block.blockColumn);
					const was = before.get(key);
					if (
						sheet.blocks.has(key) ||
						(was &&
							was.startColumn === block.startColumn &&
							was.startRow === block.startRow &&
							was.endColumn === block.endColumn &&
							was.endRow === block.endRow)
					)
						continue;
					moved.push(key);
				}
				if (moved.length > 0) {
					const queued =
						this._queue &&
						this._queue.part === part &&
						this._queue.mode === mode
							? this._queue.keys.filter((key) => moved.indexOf(key) < 0)
							: [];
					this._queue = { part, mode, keys: moved.concat(queued) };
					this._sendQueued();
				}
			}
		}
		if (values.drawOrder) {
			sheet.drawOrder = values.drawOrder;
			// An object the order no longer names is gone.
			const named = new Set(values.drawOrder);
			for (const id of Array.from(sheet.objects.keys()))
				if (!named.has(id)) this._dropObject(sheet, sheetKey, id);
		}

		const textEdits = values.textEdits;
		if (textEdits) {
			sheet.textEdits = textEdits;
			this._collectResources(
				CalcVectorManager._resourceOwner(sheetKey, 'edits'),
				(walker) => {
					for (const edit of textEdits) walker.walkPrimitives(edit.primitives);
				},
				checksums,
				fontIds,
			);
		}

		this._bitmaps.requestMissing(checksums);
		this._fonts.requestMissing(fontIds);

		if (!this._visualsReported && sheet.blocks.size > 0) {
			this._visualsReported = true;
			RenderManager.markVisualsReady();
		}
		this._fireChanged();

		// An answer frees a place for the next block waiting.
		if (request) this._sendQueued();
	}

	/// Take an object out of a block. The object stays held as it was: it moved to another block,
	/// and the draw order drops it once it is gone.
	private _leaveBlock(
		block: cool.CalcBlockEntry<cool.CalcCellRect>,
		id: number,
	): void {
		block.objects.delete(id);
	}

	/// Whether an object held is named by no held block, so it waits for the block it moved to.
	private _hasObjectWithoutBlock(sheet: cool.CalcSheetData): boolean {
		const named = new Set<number>();
		for (const block of sheet.blocks.values())
			for (const id of block.objects) named.add(id);
		for (const id of sheet.objects.keys()) if (!named.has(id)) return true;
		return false;
	}

	/// Forget an object, and the block it was held from.
	private _dropObject(
		sheet: cool.CalcSheetData,
		sheetKey: string,
		id: number,
	): void {
		for (const block of sheet.blocks.values()) block.objects.delete(id);
		sheet.objects.delete(id);
		this._releaseOwner(CalcVectorManager._resourceOwner(sheetKey, 'o' + id));
	}

	// The owner name carries a counter, so new content takes its images
	// before the old content releases them, and a shared image stays.
	private _owners: Map<string, string> = new Map();
	private _nextOwner = 1;

	private _releaseOwner(base: string): void {
		const owner = this._owners.get(base);
		if (owner === undefined) return;
		this._owners.delete(base);
		this._bitmaps.releasePart(owner);
		this._fonts.forgetPart(owner);
	}

	private _collectResources(
		base: string,
		walk: (walker: cool.VectorResourceWalker) => void,
		checksums: Set<number>,
		fontIds: Set<string>,
	): void {
		const ownChecksums = new Set<number>();
		const ownFontIds = new Set<string>();
		walk(new cool.VectorResourceWalker(ownChecksums, ownFontIds));
		const owner = base + '@' + String(this._nextOwner++);
		this._bitmaps.indexForPart(owner, ownChecksums);
		this._fonts.indexForPart(owner, ownFontIds);
		this._releaseOwner(base);
		this._owners.set(base, owner);
		for (const checksum of ownChecksums) checksums.add(checksum);
		for (const fontId of ownFontIds) fontIds.add(fontId);
	}

	/// Cache a fetched image and draw again, or mark it unavailable.
	handleVectorRenderingGraphics(
		values: cool.VectorRenderingGraphicsResponse,
	): void {
		if (this._bitmaps.has(values.checksum)) {
			this._bitmaps.clearInFlight(values.checksum);
			return;
		}
		if (!values.data) {
			this._bitmaps.setUnavailable(values.checksum);
			return;
		}
		const image = new Image();
		image.onload = () => {
			this._bitmaps.setLoaded(values.checksum, image);
			this._resourceVersion++;
			this._fireChanged();
		};
		image.onerror = () => {
			this._bitmaps.setUnavailable(values.checksum);
		};
		image.src = values.data;
	}

	/// Register a fetched font as a FontFace and draw again once it loads.
	handleVectorRenderingFont(values: cool.VectorRenderingFontResponse): void {
		const fontId = values.fontId;
		if (this._fonts.has(fontId)) {
			this._fonts.clearInFlight(fontId);
			return;
		}
		if (!values.data) {
			this._fonts.setUnavailable(fontId);
			return;
		}
		const binary = window.atob(values.data);
		const bytes = new Uint8Array(binary.length);
		for (let i = 0; i < binary.length; i++) bytes[i] = binary.charCodeAt(i);
		const face = new FontFace('vecfont-' + fontId, bytes.buffer);
		face
			.load()
			.then(() => {
				(document as unknown as { fonts: Set<FontFace> }).fonts.add(face);
				this._fonts.setLoaded(fontId, face);
				this._resourceVersion++;
				this._fireChanged();
			})
			.catch(() => {
				this._fonts.setUnavailable(fontId);
			});
	}
}
