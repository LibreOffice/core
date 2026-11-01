// @ts-strict-ignore
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

/*
 * RenderManagerBase - Base class for the tile-managing implementations that
 * sit behind the static RenderManager facade.
 *
 * Every method has a default, a no-op or an answer that nothing is held, so a subclass overrides
 * only the parts that apply to its kind of content.
 */

type AfterVisualsReadyTask = () => void;

/*
	A callback for a change of the cached vector data. answered holds the ids of the
	objects that the engine's answer replaced: empty for a change made here, undefined
	where everything may have changed. inputSequence is the newest input of this view
	that the answer reflects, where it names one.
*/
type VectorChangeListener = (
	answered?: number[],
	inputSequence?: number,
) => void;

namespace cool {
	/// A caret update's JSON, each value optional: the newest input it reflects as a number,
	/// and the logical caret and the edited object's id as strings.
	export interface CaretUpdate {
		inputSequence?: number;
		paragraph?: string;
		index?: string;
		object?: string;
	}
}

/// What a render manager draws: the objects of slides or drawing pages, the pages of a text
/// document or the cells of a spreadsheet from vector primitives, or bitmap tiles.
type RenderManagerKind = 'slides' | 'text' | 'sheet' | 'tiles';

abstract class RenderManagerBase {
	/// What the manager draws.
	abstract kind(): RenderManagerKind;

	// Whether some of the document's content has arrived and can be painted.
	private visualsReady: boolean = false;

	// Tasks to be executed once the first visuals are ready.
	private afterVisualsReadyTasks: Array<AfterVisualsReadyTask> = [];

	// -- lifecycle / discovery --
	// eslint-disable-next-line @typescript-eslint/no-empty-function
	initialize(): void {}

	appendAfterVisualsReady(task: AfterVisualsReadyTask): void {
		// in case the visuals are already there -> do in next frame
		if (this.visualsReady)
			app.layoutingService.appendLayoutingTask(() => {
				task();
			});
		else this.afterVisualsReadyTasks.push(task);
	}

	/// Takes over the tasks that another manager still holds for its first
	/// visuals. They run once this manager has content to show.
	adoptAfterVisualsReadyTasks(previous: RenderManagerBase): void {
		for (const task of previous.afterVisualsReadyTasks)
			this.appendAfterVisualsReady(task);
		previous.afterVisualsReadyTasks = [];
	}

	/// Content to show arrived from outside the tiles. Run the waiting tasks.
	markVisualsReady(): void {
		if (!this.visualsReady) this.setVisualsReady();
	}

	protected setVisualsReady(): void {
		this.visualsReady = true;
		while (this.afterVisualsReadyTasks.length > 0) {
			const task = this.afterVisualsReadyTasks.shift();
			task();
		}
	}

	// -- rendering / drawing --
	// eslint-disable-next-line @typescript-eslint/no-empty-function
	updateOverlayMessages(): void {}

	// -- transactions (delta processing) --
	// eslint-disable-next-line @typescript-eslint/no-empty-function
	beginTransaction(): void {}
	endTransaction(callback: any = null): void {
		if (callback) callback();
	}
	// eslint-disable-next-line @typescript-eslint/no-empty-function
	sendProcessedResponse(): void {}
	// eslint-disable-next-line @typescript-eslint/no-empty-function
	onWorkerEndTransaction(_e: any): void {}
	// eslint-disable-next-line @typescript-eslint/no-empty-function
	onWorkerError(_e: any): void {}

	// -- pre-fetching --
	// eslint-disable-next-line @typescript-eslint/no-empty-function
	resetPreFetching(_resetBorder: boolean): void {}
	// eslint-disable-next-line @typescript-eslint/no-empty-function
	clearPreFetch(): void {}
	// eslint-disable-next-line @typescript-eslint/no-empty-function
	initPreFetchAdjacentTiles(): void {}

	// -- memory / cache --
	// eslint-disable-next-line @typescript-eslint/no-empty-function
	pruneTiles(): void {}
	// eslint-disable-next-line @typescript-eslint/no-empty-function
	reclaimGraphicsMemory(): void {}
	// eslint-disable-next-line @typescript-eslint/no-empty-function
	discardAllCache(): void {}
	/// The tiles held.
	tileCount(): number {
		return 0;
	}
	// eslint-disable-next-line @typescript-eslint/no-empty-function
	markThumbnailStale(): void {}

	// -- debug --
	// Tile state change subscribers. Static, so a subscriber survives the
	// instance being replaced when the document type becomes known. Nothing
	// subscribes unless the preload map debug overlay is on, so notifying
	// costs a length check.
	private static tileStateListeners: Array<() => void> = [];

	onTileStateChanged(callback: () => void): void {
		RenderManagerBase.tileStateListeners.push(callback);
	}

	offTileStateChanged(callback: () => void): void {
		const index = RenderManagerBase.tileStateListeners.indexOf(callback);
		if (index >= 0) RenderManagerBase.tileStateListeners.splice(index, 1);
	}

	/// Tell subscribers that some tile's state changed: created, destroyed,
	/// invalidated, requested, or given new content.
	protected notifyTileStateChanged(): void {
		for (const callback of RenderManagerBase.tileStateListeners) callback();
	}

	// eslint-disable-next-line @typescript-eslint/no-empty-function
	setDebugDeltas(_state: boolean): void {}
	// eslint-disable-next-line @typescript-eslint/no-empty-function
	refreshTilesInBackground(): void {}

	// -- tile lookup --
	get(_coords: TileCoordData): Tile {
		return null;
	}
	getTiles(): Map<string, Tile> {
		return new Map();
	}
	isValidTile(_coords: TileCoordData): boolean {
		return false;
	}

	// -- geometry --
	pxBoundsToTileRange(_bounds: any): cool.Bounds {
		// An empty range, with max below min, so that a range loop that
		// counts up from min to max runs zero times.
		return new cool.Bounds(new cool.Point(0, 0), new cool.Point(-1, -1));
	}
	expandTileRange(range: cool.Bounds): cool.Bounds {
		return range;
	}
	predictTilesToSlurp(): number {
		return 0;
	}

	// -- invalidation --
	overlapInvalidatedRectangleWithView(
		_part: PartNumber,
		_mode: number,
		_wireId: number,
		_invalidatedRectangle: cool.SimpleRectangle,
		_textMsg: string,
		// eslint-disable-next-line @typescript-eslint/no-empty-function
	): void {}
	// eslint-disable-next-line @typescript-eslint/no-empty-function
	invalidateTile(_key: string, _wireId: number): void {}

	// -- update / fetch --
	// eslint-disable-next-line @typescript-eslint/no-empty-function
	update(_center: any = null, _zoom: number = null): void {}
	// eslint-disable-next-line @typescript-eslint/no-empty-function
	updateOnChangePart(): void {}
	updateFileBasedView(
		_checkOnly: boolean = false,
		_zoomFrameBounds: any = null,
		_forZoom: any = null,
	): TileCoordData[] {
		return [];
	}
	checkRequestTiles(
		_currentCoordList: TileCoordData[],
		_sendTileCombine: boolean = true,
	): TileCoordData[] {
		return [];
	}
	// eslint-disable-next-line @typescript-eslint/no-empty-function
	requestVisibleTiles(_currentCoordList: TileCoordData[]): void {}

	// -- message handling --
	// eslint-disable-next-line @typescript-eslint/no-empty-function
	onTileMsg(_textMsg: string, _img: any): void {}
	// eslint-disable-next-line @typescript-eslint/no-empty-function
	onTileGoneMsg(_textMsg: string): void {}

	// -- expiry / touch --
	getExpiryFactor(_tile: Tile): number {
		return 0;
	}
	// eslint-disable-next-line @typescript-eslint/no-empty-function
	touchImage(_tile: Tile): void {}

	// -- vector rendering --
	/// Whether the view draws the objects of slides or drawing pages from vector primitives.
	isVectorRendering(): boolean {
		return false;
	}

	/// Whether the client holds the drawing objects and selects, hit tests
	/// and moves them itself.
	drawsFromObjects(): boolean {
		return this.isVectorRendering();
	}

	/// The message with object positions in the engine's placement.
	toEngine(message: string): string {
		return message;
	}

	/// True when the view draws the pages of the document from vector primitives. Unlike
	/// isVectorRendering, it is also true for a text document, whose objects the server handles.
	/// A vector-drawn spreadsheet draws its cells in a section of its own and is false here.
	drawsFromPrimitives(): boolean {
		return false;
	}

	// Shows typed text before the engine answers. True when it shows.
	predictTextInsert(_text: string): boolean {
		return false;
	}

	// Removes characters before and after the caret before the engine answers. True when it
	// shows.
	predictTextDelete(_before: number, _after: number): boolean {
		return false;
	}

	// True when the local caret is ahead of the update's input.
	keepPredictedCaret(_update: cool.CaretUpdate): boolean {
		return false;
	}

	// Returns true when a keystroke at the caret would be predicted.
	canPredictAtCaret(): boolean {
		return false;
	}

	// Moves the caret one character without the server, and with extend selects text to it.
	// True when it moved.
	moveCaretByCharacter(_step: -1 | 1, _extend?: boolean): boolean {
		return false;
	}

	// Moves the caret one word without the server, and with extend selects text to it. True
	// when it moved.
	moveCaretByWord(_step: -1 | 1, _extend?: boolean): boolean {
		return false;
	}

	// Moves the caret one line up or down without the server, and with extend selects text to
	// it. True when it moved.
	moveCaretByLine(_step: -1 | 1, _extend?: boolean): boolean {
		return false;
	}

	// Starts a drag that selects text without the server. True when the press is in the edited
	// text.
	startTextDrag(_x: number, _y: number, _hitId: number | undefined): boolean {
		return false;
	}

	// Selects the text from where the drag started to the mouse.
	// eslint-disable-next-line @typescript-eslint/no-empty-function
	dragTextTo(_x: number, _y: number): void {}

	// eslint-disable-next-line @typescript-eslint/no-empty-function
	endTextDrag(): void {}

	/// True when (x, y), in document twips, lies outside the text where the running drag that
	/// selects text started. The selection made here stays inside that text.
	textDragLeavesItsText(_x: number, _y: number): boolean {
		return false;
	}

	/// The text frame at (x, y), in document twips, on a text document page drawn from vector
	/// primitives, or undefined.
	textFrameAt(_x: number, _y: number): number | undefined {
		return undefined;
	}

	// Puts the caret where a click lands in the edited text, without the server. True when it
	// moved.
	placeCaretAt(_x: number, _y: number, _hitId: number | undefined): boolean {
		return false;
	}

	// Selects the word under a double click in the edited text, without the server. True when
	// the word is selected.
	selectWordAt(_x: number, _y: number, _hitId: number | undefined): boolean {
		return false;
	}

	// Selects the paragraph under a triple click in the edited text, without the server. True
	// when the paragraph is selected.
	selectParagraphAt(
		_x: number,
		_y: number,
		_hitId: number | undefined,
	): boolean {
		return false;
	}

	requestThumbnail(
		_id: cool.PreviewId,
		_part: number,
		_mode: number,
		_maxWidth: number,
		_maxHeight: number,
		// eslint-disable-next-line @typescript-eslint/no-empty-function
	): void {}

	/// The vector content of a part, or undefined when none is held.
	requestPart(
		_part: number,
		_mode: number,
	): cool.VectorPrimitivesData | undefined {
		return undefined;
	}

	partIdAt(_part: number, _mode: number): cool.VectorPartGuid | undefined {
		return undefined;
	}

	renderInto(
		_context: CanvasRenderingContext2D,
		_data: cool.VectorPrimitivesData,
		_options?: cool.VectorRenderOptions,
		// eslint-disable-next-line @typescript-eslint/no-empty-function
	): void {}

	renderObjectsWith(
		_context: CanvasRenderingContext2D,
		_data: cool.VectorPrimitivesData,
		_objectIds: number[],
		_matrix: cool.Matrix2D,
		// eslint-disable-next-line @typescript-eslint/no-empty-function
	): void {}

	renderPrimitives(
		_context: CanvasRenderingContext2D,
		_data: cool.VectorPrimitivesData,
		_primitives: cool.Primitive[],
		// eslint-disable-next-line @typescript-eslint/no-empty-function
	): void {}

	/// Draw one paragraph of an object's text alone, in the colors and text look given. The
	/// context transform maps the part's twips to the target pixels.
	renderObjectParagraph(
		_context: CanvasRenderingContext2D,
		_data: cool.VectorPrimitivesData,
		_objectId: number,
		_paragraph: number,
		_options?: cool.ParagraphRenderOptions,
		// eslint-disable-next-line @typescript-eslint/no-empty-function
	): void {}

	renderPlaceholderAids(
		_context: CanvasRenderingContext2D,
		_data: cool.VectorPrimitivesData,
		// eslint-disable-next-line @typescript-eslint/no-empty-function
	): void {}

	/// The color the text run is drawn in, with no animation color in its place.
	textColorOf(
		_data: cool.VectorPrimitivesData,
		primitive:
			cool.TextSimplePortionPrimitive | cool.TextDecoratedPortionPrimitive,
	): string {
		return primitive.fontcolor;
	}

	// eslint-disable-next-line @typescript-eslint/no-empty-function
	setHiddenLayers(_layers: unknown): void {}

	// eslint-disable-next-line @typescript-eslint/no-empty-function
	drawnElsewhere(_objectIds: number[]): void {}

	drawnElsewhereVersion(): number {
		return 0;
	}

	isPartDrawable(_part: number, _mode: number): boolean {
		return false;
	}

	requestPartById(
		_partId: cool.VectorPartGuid,
		_mode: number,
	): cool.VectorPrimitivesData | undefined {
		return undefined;
	}

	isPartDrawableById(_partId: cool.VectorPartGuid, _mode: number): boolean {
		return false;
	}

	isLayerVisible(_layer: number): boolean {
		return true;
	}

	// eslint-disable-next-line @typescript-eslint/no-empty-function
	onVectorChanged(_callback: VectorChangeListener): void {}

	// eslint-disable-next-line @typescript-eslint/no-empty-function
	offVectorChanged(_callback: () => void): void {}

	// eslint-disable-next-line @typescript-eslint/no-empty-function
	changedHere(): void {}

	handleVectorPrimitivesResponse(
		_values: cool.VectorPrimitivesResponse,
		// eslint-disable-next-line @typescript-eslint/no-empty-function
	): void {}

	handleVectorRenderingGraphicsResponse(
		_values: cool.VectorRenderingGraphicsResponse,
		// eslint-disable-next-line @typescript-eslint/no-empty-function
	): void {}

	handleVectorRenderingFontResponse(
		_values: cool.VectorRenderingFontResponse,
		// eslint-disable-next-line @typescript-eslint/no-empty-function
	): void {}

	handleVectorPrimitivesDelta(
		_values: cool.VectorPrimitivesResponse,
		// eslint-disable-next-line @typescript-eslint/no-empty-function
	): void {}

	// eslint-disable-next-line @typescript-eslint/no-empty-function
	partListChanged(): void {}

	/// Changes may have been missed while away, so ask again for what is held.
	// eslint-disable-next-line @typescript-eslint/no-empty-function
	revalidateCachedParts(): void {}

	/// Answers to requests sent before a reconnect never come.
	// eslint-disable-next-line @typescript-eslint/no-empty-function
	forgetRequestsInFlight(): void {}

	// eslint-disable-next-line @typescript-eslint/no-empty-function
	clearCachedPart(_partId: cool.VectorPartGuid): void {}

	// eslint-disable-next-line @typescript-eslint/no-empty-function
	clearAllParts(): void {}

	// eslint-disable-next-line @typescript-eslint/no-empty-function
	refreshTheme(): void {}

	// -- redraw --
	redraw(): RenderManagerBase {
		return this;
	}
}
