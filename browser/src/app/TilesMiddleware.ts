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

/// Static facade — preserves the static TileManager API used by all call sites.
class TileManager {
	private static _instance: BitmapTileManager;

	private static ensureInstance(): BitmapTileManager {
		if (!TileManager._instance) TileManager._instance = new BitmapTileManager();
		return TileManager._instance;
	}

	static get tileSize(): number {
		return TileManager.ensureInstance().tileSize;
	}

	static initialize(): void {
		TileManager.ensureInstance().initialize();
	}

	static appendAfterFirstTileTask(task: AfterFirstTileTask): void {
		TileManager.ensureInstance().appendAfterFirstTileTask(task);
	}

	static updateOverlayMessages(): void {
		TileManager.ensureInstance().updateOverlayMessages();
	}

	static getExpiryFactor(tile: Tile): number {
		return TileManager.ensureInstance().getExpiryFactor(tile);
	}

	static beginTransaction(): void {
		TileManager.ensureInstance().beginTransaction();
	}

	static endTransaction(callback: any = null): void {
		TileManager.ensureInstance().endTransaction(callback);
	}

	static refreshTilesInBackground(): void {
		TileManager.ensureInstance().refreshTilesInBackground();
	}

	static setDebugDeltas(state: boolean): void {
		TileManager.ensureInstance().setDebugDeltas(state);
	}

	static get(coords: TileCoordData): Tile {
		return TileManager.ensureInstance().get(coords);
	}

	static getTiles(): Map<string, Tile> {
		return TileManager.ensureInstance().getTiles();
	}

	static overlapInvalidatedRectangleWithView(
		part: number,
		mode: number,
		wireId: number,
		invalidatedRectangle: cool.SimpleRectangle,
		textMsg: string,
	): void {
		TileManager.ensureInstance().overlapInvalidatedRectangleWithView(
			part,
			mode,
			wireId,
			invalidatedRectangle,
			textMsg,
		);
	}

	static resetPreFetching(resetBorder: boolean): void {
		TileManager.ensureInstance().resetPreFetching(resetBorder);
	}

	static clearPreFetch(): void {
		TileManager.ensureInstance().clearPreFetch();
	}

	static sendProcessedResponse(): void {
		TileManager.ensureInstance().sendProcessedResponse();
	}

	static onTileMsg(textMsg: string, img: any): void {
		TileManager.ensureInstance().onTileMsg(textMsg, img);
	}

	static predictTilesToSlurp(): number {
		return TileManager.ensureInstance().predictTilesToSlurp();
	}

	static pruneTiles(): void {
		TileManager.ensureInstance().pruneTiles();
	}

	static reclaimGraphicsMemory(): void {
		TileManager.ensureInstance().reclaimGraphicsMemory();
	}

	static discardAllCache(): void {
		TileManager.ensureInstance().discardAllCache();
	}

	static isValidTile(coords: TileCoordData): boolean {
		return TileManager.ensureInstance().isValidTile(coords);
	}

	static redraw(): BitmapTileManager {
		return TileManager.ensureInstance().redraw();
	}

	static update(center: any = null, zoom: number = null): void {
		TileManager.ensureInstance().update(center, zoom);
	}

	static onWorkerEndTransaction(e: any): void {
		TileManager.ensureInstance().onWorkerEndTransaction(e);
	}

	static onWorkerError(e: any): void {
		TileManager.ensureInstance().onWorkerError(e);
	}

	static updateOnChangePart(): void {
		TileManager.ensureInstance().updateOnChangePart();
	}

	static expandTileRange(range: cool.Bounds): cool.Bounds {
		return TileManager.ensureInstance().expandTileRange(range);
	}

	static pxBoundsToTileRange(bounds: any): cool.Bounds {
		return TileManager.ensureInstance().pxBoundsToTileRange(bounds);
	}

	static checkRequestTiles(
		currentCoordList: TileCoordData[],
		sendTileCombine: boolean = true,
	): TileCoordData[] {
		return TileManager.ensureInstance().checkRequestTiles(
			currentCoordList,
			sendTileCombine,
		);
	}

	static updateFileBasedView(
		checkOnly: boolean = false,
		zoomFrameBounds: any = null,
		forZoom: any = null,
	): TileCoordData[] {
		return TileManager.ensureInstance().updateFileBasedView(
			checkOnly,
			zoomFrameBounds,
			forZoom,
		);
	}

	static invalidateTile(key: string, wireId: number): void {
		TileManager.ensureInstance().invalidateTile(key, wireId);
	}

	static touchImage(tile: Tile): void {
		TileManager.ensureInstance().touchImage(tile);
	}
}

(window as any).TileManager = TileManager;
