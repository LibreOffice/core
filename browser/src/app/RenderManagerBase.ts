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
 * The tile methods have a safe no-op default, so a subclass overrides only the
 * parts that apply to it. The vector rendering methods are abstract, and each
 * subclass says what they do for its kind of content.
 */

type AfterVisualsReadyTask = () => void;

abstract class RenderManagerBase {
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
	abstract isVectorRendering(): boolean;

	abstract requestThumbnail(
		id: cool.PreviewId,
		part: number,
		mode: number,
		maxWidth: number,
		maxHeight: number,
	): void;

	abstract requestPart(
		part: number,
		mode: number,
	): cool.VectorPrimitivesData | undefined;

	abstract partIdAt(
		part: number,
		mode: number,
	): cool.VectorPartGuid | undefined;

	abstract renderInto(
		context: CanvasRenderingContext2D,
		data: cool.VectorPrimitivesData,
		options?: cool.VectorRenderOptions,
	): void;

	abstract renderObjectsWith(
		context: CanvasRenderingContext2D,
		data: cool.VectorPrimitivesData,
		objectIds: number[],
		matrix: cool.Matrix2D,
	): void;

	abstract renderPrimitives(
		context: CanvasRenderingContext2D,
		data: cool.VectorPrimitivesData,
		primitives: cool.Primitive[],
	): void;

	abstract renderPlaceholderAids(
		context: CanvasRenderingContext2D,
		data: cool.VectorPrimitivesData,
	): void;

	abstract setHiddenLayers(layers: unknown): void;

	abstract drawnElsewhere(objectIds: number[]): void;

	abstract drawnElsewhereVersion(): number;

	abstract isPartDrawable(part: number, mode: number): boolean;

	abstract requestPartById(
		partId: cool.VectorPartGuid,
		mode: number,
	): cool.VectorPrimitivesData | undefined;

	abstract isPartDrawableById(
		partId: cool.VectorPartGuid,
		mode: number,
	): boolean;

	abstract isLayerVisible(layer: number): boolean;

	abstract onVectorChanged(callback: (answered?: number[]) => void): void;

	abstract offVectorChanged(callback: () => void): void;

	abstract changedHere(): void;

	abstract handleVectorPrimitivesResponse(
		values: cool.VectorPrimitivesResponse,
	): void;

	abstract handleVectorRenderingGraphicsResponse(
		values: cool.VectorRenderingGraphicsResponse,
	): void;

	abstract handleVectorRenderingFontResponse(
		values: cool.VectorRenderingFontResponse,
	): void;

	abstract handleVectorPrimitivesDelta(
		values: cool.VectorPrimitivesResponse,
	): void;

	abstract partListChanged(): void;

	abstract revalidateCachedParts(): void;

	abstract forgetRequestsInFlight(): void;

	abstract clearCachedPart(partId: cool.VectorPartGuid): void;

	abstract clearAllParts(): void;

	// -- redraw --
	redraw(): RenderManagerBase {
		return this;
	}
}
