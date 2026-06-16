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

/// Static facade exposing the RenderManager API as static methods.
class RenderManager {
	private static _instance: RenderManagerBase;

	/// True when the document renders from vector primitives: an Impress or
	/// Draw document with the vector rendering option enabled.
	private static wantsVector(docType?: string): boolean {
		return (
			(docType === 'presentation' || docType === 'drawing') &&
			cool.VectorRenderingConfig.isEnabled()
		);
	}

	/// Pick the manager for the document type, replacing the instance when
	/// an earlier one is of the wrong kind.
	private static makeInstance(docType?: string): RenderManagerBase {
		const wantVector = RenderManager.wantsVector(docType);
		const haveVector = RenderManager._instance instanceof VectorManager;
		if (!RenderManager._instance || wantVector !== haveVector) {
			RenderManager._instance = wantVector
				? new VectorManager()
				: new BitmapTileManager();
		}
		return RenderManager._instance;
	}

	private static ensureInstance(): RenderManagerBase {
		// Provisional default before the document type is known.
		if (!RenderManager._instance)
			RenderManager._instance = new BitmapTileManager();
		return RenderManager._instance;
	}

	static get tileSize(): number {
		return window.tileSize;
	}

	static initialize(docType?: string): void {
		RenderManager.makeInstance(docType).initialize();
	}

	static appendAfterFirstTileTask(task: AfterFirstTileTask): void {
		RenderManager.ensureInstance().appendAfterFirstTileTask(task);
	}

	static updateOverlayMessages(): void {
		RenderManager.ensureInstance().updateOverlayMessages();
	}

	static getExpiryFactor(tile: Tile): number {
		return RenderManager.ensureInstance().getExpiryFactor(tile);
	}

	static beginTransaction(): void {
		RenderManager.ensureInstance().beginTransaction();
	}

	static endTransaction(callback: any = null): void {
		RenderManager.ensureInstance().endTransaction(callback);
	}

	static refreshTilesInBackground(): void {
		RenderManager.ensureInstance().refreshTilesInBackground();
	}

	static setDebugDeltas(state: boolean): void {
		RenderManager.ensureInstance().setDebugDeltas(state);
	}

	static get(coords: TileCoordData): Tile {
		return RenderManager.ensureInstance().get(coords);
	}

	static getTiles(): Map<string, Tile> {
		return RenderManager.ensureInstance().getTiles();
	}

	static overlapInvalidatedRectangleWithView(
		part: number,
		mode: number,
		wireId: number,
		invalidatedRectangle: cool.SimpleRectangle,
		textMsg: string,
	): void {
		RenderManager.ensureInstance().overlapInvalidatedRectangleWithView(
			part,
			mode,
			wireId,
			invalidatedRectangle,
			textMsg,
		);
	}

	static resetPreFetching(resetBorder: boolean): void {
		RenderManager.ensureInstance().resetPreFetching(resetBorder);
	}

	static clearPreFetch(): void {
		RenderManager.ensureInstance().clearPreFetch();
	}

	static sendProcessedResponse(): void {
		RenderManager.ensureInstance().sendProcessedResponse();
	}

	static onTileMsg(textMsg: string, img: any): void {
		RenderManager.ensureInstance().onTileMsg(textMsg, img);
	}

	static predictTilesToSlurp(): number {
		return RenderManager.ensureInstance().predictTilesToSlurp();
	}

	static pruneTiles(): void {
		RenderManager.ensureInstance().pruneTiles();
	}

	static reclaimGraphicsMemory(): void {
		RenderManager.ensureInstance().reclaimGraphicsMemory();
	}

	static discardAllCache(): void {
		RenderManager.ensureInstance().discardAllCache();
	}

	static isValidTile(coords: TileCoordData): boolean {
		return RenderManager.ensureInstance().isValidTile(coords);
	}

	static redraw(): RenderManagerBase {
		return RenderManager.ensureInstance().redraw();
	}

	static update(center: any = null, zoom: number = null): void {
		RenderManager.ensureInstance().update(center, zoom);
	}

	static onWorkerEndTransaction(e: any): void {
		RenderManager.ensureInstance().onWorkerEndTransaction(e);
	}

	static onWorkerError(e: any): void {
		RenderManager.ensureInstance().onWorkerError(e);
	}

	static updateOnChangePart(): void {
		RenderManager.ensureInstance().updateOnChangePart();
	}

	static expandTileRange(range: cool.Bounds): cool.Bounds {
		return RenderManager.ensureInstance().expandTileRange(range);
	}

	static pxBoundsToTileRange(bounds: any): cool.Bounds {
		return RenderManager.ensureInstance().pxBoundsToTileRange(bounds);
	}

	static checkRequestTiles(
		currentCoordList: TileCoordData[],
		sendTileCombine: boolean = true,
	): TileCoordData[] {
		return RenderManager.ensureInstance().checkRequestTiles(
			currentCoordList,
			sendTileCombine,
		);
	}

	static updateFileBasedView(
		checkOnly: boolean = false,
		zoomFrameBounds: any = null,
		forZoom: any = null,
	): TileCoordData[] {
		return RenderManager.ensureInstance().updateFileBasedView(
			checkOnly,
			zoomFrameBounds,
			forZoom,
		);
	}

	static invalidateTile(key: string, wireId: number): void {
		RenderManager.ensureInstance().invalidateTile(key, wireId);
	}

	static touchImage(tile: Tile): void {
		RenderManager.ensureInstance().touchImage(tile);
	}

	// vector rendering specific

	static isVectorRendering(): boolean {
		return RenderManager.ensureInstance().isVectorRendering();
	}

	static requestThumbnail(
		id: cool.PreviewId,
		part: number,
		maxWidth: number,
		maxHeight: number,
	): void {
		RenderManager.ensureInstance().requestThumbnail(
			id,
			part,
			maxWidth,
			maxHeight,
		);
	}

	static requestPart(part: number): cool.VectorTileData | undefined {
		return RenderManager.ensureInstance().requestPart(part);
	}

	static renderInto(
		context: CanvasRenderingContext2D,
		data: cool.VectorTileData,
	): void {
		RenderManager.ensureInstance().renderInto(context, data);
	}

	static onVectorChanged(callback: () => void): void {
		RenderManager.ensureInstance().onVectorChanged(callback);
	}

	static handleVectorTileResponse(values: cool.VectorTileResponse): void {
		RenderManager.ensureInstance().handleVectorTileResponse(values);
	}

	static handleVectorRenderingGraphicsResponse(
		values: cool.VectorRenderingGraphicsResponse,
	): void {
		RenderManager.ensureInstance().handleVectorRenderingGraphicsResponse(
			values,
		);
	}

	static clearCachedPart(part: number): void {
		RenderManager.ensureInstance().clearCachedPart(part);
	}
}

(window as any).RenderManager = RenderManager;
