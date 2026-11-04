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
	/// extra request keeps it busy. A refresh of stale blocks around the view takes one at most,
	/// so a block the view comes to show is not held up behind it.
	private static readonly maxRequestsInFlight = 2;

	/// The most held blocks asked for in one request. A held block is asked for again when it may
	/// have changed, and the engine builds it again for the answer, so a few at a time keep each
	/// answer short and let other messages in between. A view that shows many blocks, as at a
	/// low zoom, takes twice as many.
	private static readonly maxHeldBlocksPerRequest = 4;

	/// The most missing blocks asked for in one request. A missing block shows as soon as it is
	/// built, so at a high zoom each gets a request of its own, and a view that shows many
	/// blocks asks for a few at a time.
	private static readonly maxMissingBlocksPerRequest = 4;

	/// How far around the blocks asked for, in blocks, held stale blocks are asked for again while
	/// the view has all it needs, so they are up to date before they are scrolled to. The limit
	/// keeps the engine from building every stale block of a long sheet. A view that spans more
	/// blocks reaches twice as many block rows as it spans, and as many block columns.
	private static readonly refreshBlockRows = 3;
	private static readonly refreshBlockColumns = 1;

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

	// The images and fonts the content names, each owned by the blocks, drawing objects and
	// text edits that name it.
	private _resources = new VectorResourceLoader(() => {
		this._resourceVersion++;
		this._fireChanged([]);
	});

	// The typing of this view in its cell edit, measured like the renderer.
	private _typing = new cool.CalcTyping(
		new cool.TextMeasurer(
			cool.canvasTextMeasure((fontId) => this._resources.fonts.has(fontId)),
		),
	);

	readonly renderer: cool.VectorPrimitiveRenderer =
		new cool.VectorPrimitiveRenderer(
			(checksum) => this._resources.image(checksum),
			(fontId) => this._resources.fonts.has(fontId),
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

	// The block rows and columns the view spans, from the last draw. They set how many blocks go
	// in one request and how far around the view stale blocks are refreshed.
	private _view: { rows: number; columns: number } = { rows: 1, columns: 1 };

	// Blocks of a sheet waiting to be asked for, most wanted first. Each draw replaces them, so
	// only blocks still in view are asked for. A refresh holds stale blocks around the view, none
	// of which the view shows.
	private _queue: {
		part: number;
		mode: number;
		keys: string[];
		refresh?: boolean;
	} | null = null;

	private _changeListeners = new cool.VectorChangeListeners();
	// Callbacks that release graphics memory held for drawing the content.
	private _reclaimListeners: (() => void)[] = [];

	// Objects drawn outside the sheet, such as the selection during a drag.
	private _drawnElsewhere = new cool.DrawnElsewhere();
	// Counts up each time a font or an image the content names has loaded.
	private _resourceVersion = 0;
	private _visualsReported = false;

	// The drawing objects of each sheet in tile twips, keyed by part.
	private _objectViews: Map<number, cool.CalcObjectView> = new Map();
	// Counts changes of content and geometry. Older object views get rebuilt.
	private _changeCount = 0;
	private _viewCounts: Map<number, number> = new Map();
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
			/// The versions of the shifts the block was placed by.
			shifted: string;
			/// The block placed whole, and as shown without the cells that views edit.
			whole: cool.CalcBlockEntry;
			edited: string;
			placed: cool.CalcBlockEntry;
		}
	> = new Map();

	/// The mode content is asked in: cell positions, which the view places at any zoom, so a
	/// column or a row that changes size moves the content after it without sending it again.
	static readonly contentMode = -1;

	/// Call back on every change of what is held, with the ids of the drawing objects replaced or
	/// removed. Undefined means anything may have changed.
	onChanged(callback: (answered?: number[]) => void): void {
		this._changeListeners.add(callback);
	}

	/// Call back to release graphics memory, such as when the document is hidden.
	onReclaimGraphicsMemory(callback: () => void): void {
		this._reclaimListeners.push(callback);
	}

	offReclaimGraphicsMemory(callback: () => void): void {
		const index = this._reclaimListeners.indexOf(callback);
		if (index >= 0) this._reclaimListeners.splice(index, 1);
	}

	/// Release graphics memory: the decoded images and what the renderer keeps between draws.
	/// What is still needed is asked for and drawn again when shown. The fonts stay registered,
	/// since they are few and small.
	reclaimGraphicsMemory(): void {
		for (const listener of this._reclaimListeners) listener();
		this._resources.dropImages();
		this.renderer.releaseScratchCanvases();
		this.renderer.releaseColorChangedImages();
	}

	offChanged(callback: (answered?: number[]) => void): void {
		this._changeListeners.remove(callback);
	}

	private _fireChanged(answered?: number[]): void {
		this._changeCount++;
		this._changeListeners.fire(answered);
	}

	/// Draw again the objects the client changed, such as after a drag.
	changedHere(): void {
		this._fireChanged([]);
	}

	/// Leave out of the sheet the objects that something else draws.
	drawnElsewhere(objectIds: number[]): void {
		this._drawnElsewhere.set(objectIds);
	}

	drawnElsewhereVersion(): number {
		return this._drawnElsewhere.version();
	}

	/// Counts up each time a font or image that the content names has loaded.
	resourceVersion(): number {
		return this._resourceVersion;
	}

	isDrawnElsewhere(objectId: number): boolean {
		return this._drawnElsewhere.has(objectId);
	}

	/// The text edits of the sheet, with the unanswered typing of this view.
	textEditsOf(sheet: cool.CalcSheetData): cool.CalcTextEdit[] {
		return this._typing.editsOf(sheet.part, sheet.textEdits);
	}

	/// True when a keystroke at the caret can show before the engine answers.
	canPredictAtCaret(): boolean {
		const context = this._typingContext();
		if (!context) return false;
		return this._typing.canTypeAt(
			context.edits,
			context.viewId,
			context.origin,
			context.caret,
		);
	}

	/// Show typed text in the cell edit before the engine answers, replacing a selection in the
	/// paragraph of the caret. True when it shows.
	predictTextInsert(text: string): boolean {
		return this._type((selected) => cool.typedChange(text, selected));
	}

	/// Delete before characters left of the caret and after characters right of it in the cell edit
	/// of this view. True when it shows.
	predictTextDelete(before: number, after: number): boolean {
		if (before <= 0 && after <= 0) return false;
		return this._type((selected) =>
			cool.deletedChange(before, after, selected),
		);
	}

	/// After an input that ends the cell edit or moves its caret, drop the typing and predict
	/// nothing until the engine reports the caret again.
	noteInput(message: string): void {
		if (!cool.calcInputMovesEdit(message)) return;
		const wasTyping = this._typing.isTyping();
		this._typing.holdUntilCaret();
		if (wasTyping) this._fireChanged([]);
	}

	/// True when the caret update is older than the newest typing. A caret replayed for a geometry
	/// change predates the typing, so the typed caret is placed again with the current geometry.
	keepPredictedCaret(update: cool.CaretUpdate): boolean {
		const docLayer = app.map && app.map._docLayer;
		if (docLayer?._isReplayingMessages && this._typing.isTyping()) {
			const context = this._typingContext();
			const typed = context
				? this._typing.caretAfterTyping(context.viewId, context.origin)
				: null;
			if (typed) {
				this._showTypedCaret(typed);
				return true;
			}
		}
		const wasTyping = this._typing.isTyping();
		const kept = this._typing.keepsCaret(update);
		// A dropped typing leaves the edits of the engine to draw.
		if (wasTyping && !this._typing.isTyping()) this._fireChanged([]);
		return kept;
	}

	private _type(
		build: (selected: cool.SelectedAround | null) => cool.PredictedChange,
	): boolean {
		const context = this._typingContext();
		if (!context) return false;
		const caret = context.caret;
		const typed = this._typing.type(
			context.part,
			context.edits,
			context.viewId,
			context.origin,
			caret,
			// The ticket takes the number of the input message sent next.
			cool.InputSequence.ticketForTheNextMessage(),
			build,
		);
		if (!typed) return false;
		if (caret.selected || caret.anchor) {
			// The selection is gone with the text it covered.
			app.file.textCursor.anchor = null;
			if (caret.selected)
				(
					app.map._docLayer as unknown as cool.CanvasTileLayerInterface
				).showTextSelection([]);
		}
		this._showTypedCaret(typed);
		this._fireChanged([]);
		return true;
	}

	private _showTypedCaret(typed: cool.CalcTypedCaret): void {
		app.file.textCursor.logical = typed.logical;
		cool.showCaret(typed.geometry);
	}

	/// What typing in a cell edit needs, or null when no cell edit caret shows.
	private _typingContext(): {
		part: number;
		edits: cool.CalcTextEdit[];
		viewId: number;
		origin: cool.CalcCellOrigin;
		caret: cool.CalcCaret;
	} | null {
		const docLayer = app.map && app.map._docLayer;
		const cursor = app.file.textCursor;
		if (!docLayer || !cursor || !cursor.visible || !cursor.rectangle)
			return null;
		// Typing in the formula bar shows in the bar.
		if (app.map.calcInputBarHasFocus()) return null;
		const part: number = docLayer._selectedPart;
		const viewId: number | undefined = docLayer._viewId;
		const geometry: cool.SheetGeometry | undefined = docLayer.sheetGeometry;
		if (viewId === undefined || !geometry || geometry.getPart() !== part)
			return null;
		const sheet = this.getSheet(part, CalcVectorManager.contentMode);
		if (!sheet) return null;
		return {
			part,
			edits: sheet.textEdits,
			viewId,
			origin: (column: number, row: number) => {
				const x = geometry
					.getColumnsGeometry()
					.getElementDataAny(column, 'tiletwips');
				const y = geometry
					.getRowsGeometry()
					.getElementDataAny(row, 'tiletwips');
				return x && y ? new cool.Point(x.startpos, y.startpos) : undefined;
			},
			caret: {
				rectangle: cursor.rectangle,
				logical: cursor.logical ?? null,
				anchor: cursor.anchor ?? null,
				selected: TextSelections.isActive(),
			},
		};
	}

	/// Draw the given objects of a view of objects through the matrix.
	renderObjectsWith(
		context: CanvasRenderingContext2D,
		data: cool.VectorPrimitivesData,
		objectIds: number[],
		matrix: cool.Matrix2D,
	): void {
		this.renderer.setEditViewContentVisible(true);
		this.renderer.renderObjectsMoved(context, data.objects, objectIds, matrix);
	}

	renderPrimitives(
		context: CanvasRenderingContext2D,
		primitives: cool.Primitive[],
	): void {
		this.renderer.setEditViewContentVisible(true);
		for (const primitive of primitives)
			this.renderer.renderPrimitive(context, primitive);
	}

	/// A message that changes drawing objects, with its positions converted from tile twips to
	/// print twips. Other messages come back unchanged.
	messageInPrintTwips(message: string): string {
		if (!cool.calcMessageHasPositions(message)) return message;
		const docLayer = app.map && app.map._docLayer;
		if (!docLayer) return message;
		const part: number = docLayer._selectedPart;
		const view = this._objectView(part);
		const geometry: cool.SheetGeometry | undefined = docLayer.sheetGeometry;
		if (!view || !geometry) return message;
		const objects = view.data.objects;
		const outermostOf = (id: number): number => {
			const seen = new Set<number>([id]);
			let top = id;
			for (;;) {
				const parent = objects.get(top)?.parent;
				if (!parent || seen.has(parent) || !objects.has(parent)) return top;
				seen.add(parent);
				top = parent;
			}
		};
		// A group holds other objects and draws nothing of its own.
		const groups = new Set<number>();
		for (const object of objects.values())
			if (object.parent) groups.add(object.parent);
		return cool.calcMessageInPrintTwips(message, {
			placingOf: (id: number) => view.placingOf(id),
			outermostOf,
			membersOf: (outermost: number) =>
				Array.from(objects.keys()).filter(
					(id) => !groups.has(id) && outermostOf(id) === outermost,
				),
			placingAt: (id: number, point: cool.Point) => {
				const print = geometry.getPrintTwipsPointFromTile(point);
				const scale = view.placingOf(id) ?? cool.calcIdentityPlacing;
				return {
					scaleX: scale.scaleX,
					scaleY: scale.scaleY,
					moveX: point.x - scale.scaleX * print.x,
					moveY: point.y - scale.scaleY * print.y,
				};
			},
		});
	}

	/// The drawing objects of a sheet in tile twips, placed by the shown geometry. Undefined until
	/// they arrive, or while the geometry is still that of the previous sheet.
	objectsOf(part: number): cool.VectorPrimitivesData | undefined {
		const view = this._objectView(part);
		return view ? view.data : undefined;
	}

	/// The placing of the object from print twips of the sheet to tile twips, if held.
	objectPlacingOf(
		part: number,
		id: number,
	): cool.CalcObjectPlacing | undefined {
		const view = this._objectView(part);
		return view ? view.placingOf(id) : undefined;
	}

	private _objectView(part: number): cool.CalcObjectView | undefined {
		const docLayer = app.map && app.map._docLayer;
		const geometry: cool.SheetGeometry | undefined = docLayer
			? docLayer.sheetGeometry
			: undefined;
		if (!geometry || geometry.getPart() !== part) return undefined;
		const sheet = this._sheetWithObjects(part);
		if (!sheet) return undefined;

		this._listenToGeometry();

		let view = this._objectViews.get(part);
		if (!view) {
			view = new cool.CalcObjectView(part);
			this._objectViews.set(part, view);
		}
		if (this._viewCounts.get(part) !== this._changeCount) {
			view.update(sheet, geometry);
			this._viewCounts.set(part, this._changeCount);
		}
		return view;
	}

	private _listenToGeometry(): void {
		if (this._listensToGeometry) return;
		this._listensToGeometry = true;
		app.map.on('sheetgeometrychanged', () => {
			this._changeCount++;
			this._geometryCount++;
		});
	}

	/// The sheet whose drawing objects are shown, once it arrived whole.
	private _sheetWithObjects(part: number): cool.CalcSheetData | undefined {
		const sheet = this.getSheet(part, CalcVectorManager.contentMode);
		return sheet && sheet.complete ? sheet : undefined;
	}

	/// The block placed at the view zoom, in tile twips from its first cell, without the stored
	/// text of the cells that views edit. Undefined while the geometry of its sheet is not known.
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
		const editedCells = this._editedCellsIn(sheet, block);
		const edited = editedCells
			.map((cell) => String(cell.column) + ':' + String(cell.row))
			.join(',');
		const shifts = this.blockShifts(sheet, block);
		const shifted = shifts.map((shift) => String(shift.version)).join(',');
		const held = this._placedBlocks.get(key);
		const placedAsHeld =
			held &&
			held.source === block &&
			held.count === this._geometryCount &&
			held.pixel === pixel &&
			held.shifted === shifted;
		if (held && placedAsHeld && held.edited === edited) return held.placed;
		const whole =
			held && placedAsHeld
				? held.whole
				: cool.calcPlacedBlock(block, geometry, pixel, shifts);
		if (!whole) return undefined;
		// A new block is drawn again, text rasters too, so only a block with an edit changes.
		const placed = editedCells.length
			? cool.calcBlockWithoutCellText(whole, editedCells)
			: whole;
		this._placedBlocks.set(key, {
			source: block,
			count: this._geometryCount,
			pixel,
			shifted,
			whole,
			edited,
			placed,
		});
		return placed;
	}

	/// The shifts of the sheet that a held block predates and that moved all of its cells, so
	/// the block is placed as they moved until it is built again. A shift across a part of the
	/// block leaves it in place.
	blockShifts(
		sheet: cool.CalcSheetData,
		block: cool.CalcBlockEntry<cool.CalcCellRect>,
	): Array<cool.CalcShift & { version: number }> {
		return sheet.shifts.filter((shift) => {
			if (shift.version <= block.current) return false;
			const [first, last] =
				shift.axis === 'rows'
					? [block.startColumn, block.endColumn]
					: [block.startRow, block.endRow];
			return shift.across[0] <= first && last <= shift.across[1];
		});
	}

	/// The cells of the block that views edit, whose edits are drawn over them.
	private _editedCellsIn(
		sheet: cool.CalcSheetData,
		block: cool.CalcBlockEntry<cool.CalcCellRect>,
	): Array<{ column: number; row: number }> {
		const cells: Array<{ column: number; row: number }> = [];
		for (const edit of this.textEditsOf(sheet)) {
			if (
				edit.parent !== undefined ||
				edit.column === undefined ||
				edit.row === undefined
			)
				continue;
			if (
				edit.column >= block.startColumn &&
				edit.column <= block.endColumn &&
				edit.row >= block.startRow &&
				edit.row <= block.endRow
			)
				cells.push({ column: edit.column, row: edit.row });
		}
		return cells;
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
				shifts: [],
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
	/// earlier queue. A missing block gets a request of its own, so the first one shows as soon
	/// as the engine has built it, unless the view spans many blocks. direction is the sign of
	/// the last scroll in x and y, which the stale blocks around the view are refreshed towards
	/// first, and view the block rows and columns the view spans.
	requestBlocks(
		part: number,
		mode: number,
		blocks: Array<[number, number]>,
		direction: [number, number] = [0, 0],
		view?: { rows: number; columns: number },
	): void {
		if (view) this._view = view;
		const sheet = this._ensureSheet(part, mode);
		const sheetKey = CalcVectorManager._sheetKey(part, mode);
		const keys: string[] = [];
		for (const [blockRow, blockColumn] of blocks) {
			const key = cool.calcBlockKey(blockRow, blockColumn);
			if (keys.indexOf(key) < 0 && this._isWanted(sheet, sheetKey, key))
				keys.push(key);
		}

		// The first request brings the drawing objects and settings of the sheet, even with no
		// block. A view that has all it needs refreshes the stale blocks around it.
		const needsSheet =
			!this._sheetRequested.has(sheetKey) || this._sheetsBehind.has(sheetKey);
		let refresh = false;
		if (keys.length === 0 && !needsSheet) {
			keys.push(...this._staleAround(sheet, sheetKey, blocks, direction));
			refresh = keys.length > 0;
		}
		this._queue = { part, mode, keys, refresh };
		if (keys.length === 0 && needsSheet) this._sendRequest(part, mode, []);
		this._sendQueued();
	}

	/// The held stale blocks near the given blocks that are not asked for yet. As the tiles are
	/// fetched ahead, the side the view scrolls towards reaches twice as far and comes first, and
	/// nearer blocks come before farther ones.
	private _staleAround(
		sheet: cool.CalcSheetData,
		sheetKey: string,
		blocks: Array<[number, number]>,
		direction: [number, number],
	): string[] {
		if (!blocks.length || !sheet.staleBlocks.size) return [];
		const [towardsX, towardsY] = direction;
		const reach = (sign: number, towards: number, distance: number): number =>
			sign === towards ? 2 * distance : distance;
		const rows = blocks.map(([blockRow]) => blockRow);
		const columns = blocks.map(([, blockColumn]) => blockColumn);
		const rowReach = Math.max(
			CalcVectorManager.refreshBlockRows,
			2 * this._view.rows,
		);
		const columnReach = Math.max(
			CalcVectorManager.refreshBlockColumns,
			this._view.columns,
		);
		const top = Math.min(...rows) - reach(-1, towardsY, rowReach);
		const bottom = Math.max(...rows) + reach(1, towardsY, rowReach);
		const left = Math.min(...columns) - reach(-1, towardsX, columnReach);
		const right = Math.max(...columns) + reach(1, towardsX, columnReach);
		const middleRow = (Math.min(...rows) + Math.max(...rows)) / 2;
		const middleColumn = (Math.min(...columns) + Math.max(...columns)) / 2;

		const near: Array<{ key: string; ahead: boolean; distance: number }> = [];
		for (const key of sheet.staleBlocks) {
			const block = sheet.blocks.get(key);
			if (!block || !this._isWanted(sheet, sheetKey, key)) continue;
			if (
				block.blockRow < top ||
				block.blockRow > bottom ||
				block.blockColumn < left ||
				block.blockColumn > right
			)
				continue;
			near.push({
				key,
				ahead:
					(towardsY !== 0 &&
						Math.sign(block.blockRow - middleRow) === towardsY) ||
					(towardsX !== 0 &&
						Math.sign(block.blockColumn - middleColumn) === towardsX),
				distance: Math.hypot(
					block.blockRow - middleRow,
					block.blockColumn - middleColumn,
				),
			});
		}
		near.sort(
			(a, b) => Number(b.ahead) - Number(a.ahead) || a.distance - b.distance,
		);
		return near.map((entry) => entry.key);
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
		// A view that shows many blocks, as at a low zoom, asks for more blocks in one request.
		const shown = this._view.rows * this._view.columns;
		const missingPerRequest = Math.min(
			CalcVectorManager.maxMissingBlocksPerRequest,
			Math.max(1, Math.floor(shown / 8)),
		);
		const heldPerRequest =
			shown > 16
				? 2 * CalcVectorManager.maxHeldBlocksPerRequest
				: CalcVectorManager.maxHeldBlocksPerRequest;
		while (queue.keys.length > 0) {
			const key = queue.keys[0];
			// The block may have arrived in a push since it was queued.
			if (!this._isWanted(sheet, sheetKey, key)) {
				queue.keys.shift();
				continue;
			}
			// A refresh leaves one request free for a block the view comes to show.
			const limit = queue.refresh
				? CalcVectorManager.maxRequestsInFlight - 1
				: CalcVectorManager.maxRequestsInFlight;
			if (
				this._requests.size >= limit &&
				!this._forgetRequestOfOtherSheet(sheetKey)
			)
				break;
			queue.keys.shift();
			const keys = [key];
			// Blocks of one kind go together. A held block is drawn already, so a few held blocks
			// share a request. A missing block shows as soon as it is built, so missing blocks
			// share one only when the view shows many of them.
			const held = sheet.blocks.has(key);
			const perRequest = held ? heldPerRequest : missingPerRequest;
			queue.keys = queue.keys.filter((other) => {
				if (sheet.blocks.has(other) !== held) return true;
				if (keys.length >= perRequest) return true;
				if (this._isWanted(sheet, sheetKey, other)) keys.push(other);
				return false;
			});
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
		if (this._expireRequests()) this._fireChanged([]);
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
		const dropped: number[] = [];
		for (const [key, block] of Array.from(sheet.blocks.entries())) {
			if (keep.has(key)) continue;
			sheet.blocks.delete(key);
			this._forgetPlaced(sheetKey, key);
			sheet.staleBlocks.delete(key);
			this._resources.release(CalcVectorManager._resourceOwner(sheetKey, key));
			// The objects of the block go with it, and come back when it is asked for.
			for (const id of Array.from(block.objects))
				this._dropObject(sheet, sheetKey, id, dropped);
			if (sheet.blocks.size <= CalcVectorManager.maxBlocks / 2) break;
		}
		if (dropped.length) this._fireChanged(dropped);
	}

	private _dropSheet(sheetKey: string, sheet: cool.CalcSheetData): void {
		for (const key of sheet.blocks.keys())
			this._resources.release(CalcVectorManager._resourceOwner(sheetKey, key));
		for (const id of sheet.objects.keys())
			this._resources.release(
				CalcVectorManager._resourceOwner(sheetKey, 'o' + id),
			);
		this._resources.release(
			CalcVectorManager._resourceOwner(sheetKey, 'edits'),
		);
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
		this._fireChanged([]);
	}

	/// Mark one sheet as possibly behind, to ask again with the versions held.
	markPartStale(part: number): void {
		for (const [sheetKey, sheet] of this._sheets) {
			if (sheet.part !== part) continue;
			this._markSheetStale(sheetKey, sheet);
			// The next request asks again for the drawing objects and settings of the sheet.
			this._sheetsBehind.add(sheetKey);
		}
		this._fireChanged([]);
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

	/// Forget all content and the fonts and images it named, and take the fonts off the
	/// document.
	discardAll(): void {
		this.clearAll();
		this._resources.discard();
	}

	/// Forget all content, keeping the fonts and images that are loaded.
	clearAll(): void {
		this._typing.forget();
		this._textStyles.clear();
		this._sheets.clear();
		this._placedBlocks.clear();
		this._objectViews.clear();
		this._viewCounts.clear();
		this._inFlight.clear();
		this._requests.clear();
		this._disarmExpiry();
		this._queue = null;
		this._sheetRequested.clear();
		this._staleFrom.clear();
		this._sheetsBehind.clear();
		this._resources.forgetOwners();
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

		// The drawing objects that the answer replaced or removed.
		const answered: number[] = [];
		// The version of the sheet that the answer brings what is held up to.
		const current = values.version ?? 0;
		// A push tells how rows and columns moved, which places a block older than the push as its
		// cells moved.
		if (pushed)
			for (const shift of values.shifts ?? [])
				sheet.shifts.push({ ...shift, version: current });
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
			this._resources.replace(
				CalcVectorManager._resourceOwner(sheetKey, key),
				(walker) => {
					const layers = block.content.layers;
					for (const layer of Object.values(layers))
						if (layer) walker.walkPrimitives(layer as cool.Primitive[]);
				},
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
				answered.push(object.id);
				this._resources.replace(
					CalcVectorManager._resourceOwner(sheetKey, 'o' + object.id),
					(walker) => walker.walkPrimitives(object.content.primitives),
				);
			}
			for (const id of entry.removed ?? []) this._leaveBlock(block, id);
		}

		// A block the answer leaves out is unchanged up to the answer version. A push carries every
		// block changed since its base, and a request names each held version. A block older than
		// the push base may miss a change, and so may a block the answer names stale: it is asked
		// for again once it is shown.
		const stale = new Set(values.stale ?? []);
		for (const [key, block] of sheet.blocks) {
			if (behind.has(key)) continue;
			// A block still stale from an earlier push is not current up to this one either.
			if (stale.has(key) || sheet.staleBlocks.has(key)) {
				sheet.staleBlocks.add(key);
				continue;
			}
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
				if (!named.has(id)) this._dropObject(sheet, sheetKey, id, answered);
		}

		const textEdits = values.textEdits;
		if (textEdits) {
			sheet.textEdits = textEdits;
			this._answerTyping(part, textEdits, values.inputSequence);
			this._resources.replace(
				CalcVectorManager._resourceOwner(sheetKey, 'edits'),
				(walker) => {
					for (const edit of textEdits) walker.walkPrimitives(edit.primitives);
				},
			);
		}

		if (!this._visualsReported && sheet.blocks.size > 0) {
			this._visualsReported = true;
			RenderManager.markVisualsReady();
		}
		// A shift that no held block predates any more has been built into every block.
		if (sheet.shifts.length) {
			let oldest = Infinity;
			for (const block of sheet.blocks.values())
				oldest = Math.min(oldest, block.current);
			sheet.shifts = sheet.shifts.filter((shift) => shift.version > oldest);
		}
		this._fireChanged(answered);

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

	/// Forget an object, and the block it was held from. An object that is gone is answered, so the
	/// views drawing it hear of it.
	private _dropObject(
		sheet: cool.CalcSheetData,
		sheetKey: string,
		id: number,
		answered: number[],
	): void {
		for (const block of sheet.blocks.values()) block.objects.delete(id);
		if (sheet.objects.delete(id)) answered.push(id);
		this._resources.release(
			CalcVectorManager._resourceOwner(sheetKey, 'o' + id),
		);
	}

	// Apply again the typing that the engine edits do not include yet, and place the caret where
	// the typing leaves it.
	private _answerTyping(
		part: number,
		edits: cool.CalcTextEdit[],
		inputSequence: number | undefined,
	): void {
		if (!this._typing.isTyping(part)) return;
		const context = this._typingContext();
		// Typing without a caret in the sheet does not show.
		if (!context || context.part !== part) {
			this._typing.forget();
			return;
		}
		const typed = this._typing.answered(
			part,
			edits,
			inputSequence,
			context.viewId,
			context.origin,
		);
		if (typed) this._showTypedCaret(typed);
	}

	/// Cache a fetched image and draw again, or mark it unavailable.
	handleVectorRenderingGraphics(
		values: cool.VectorRenderingGraphicsResponse,
	): void {
		this._resources.handleImage(values);
	}

	/// Register a fetched font as a FontFace and draw again once it loads.
	handleVectorRenderingFont(values: cool.VectorRenderingFontResponse): void {
		this._resources.handleFont(values);
	}
}
