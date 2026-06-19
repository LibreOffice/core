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
 * Every method has a safe no-op default, so a subclass overrides only the
 * parts that apply to it.
 */

class RenderManagerBase {
	// -- lifecycle / discovery --
	// eslint-disable-next-line @typescript-eslint/no-empty-function
	initialize(): void {}

	// eslint-disable-next-line @typescript-eslint/no-empty-function
	appendAfterFirstTileTask(_task: () => void): void {}

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

	// -- memory / cache --
	// eslint-disable-next-line @typescript-eslint/no-empty-function
	pruneTiles(): void {}
	// eslint-disable-next-line @typescript-eslint/no-empty-function
	reclaimGraphicsMemory(): void {}
	// eslint-disable-next-line @typescript-eslint/no-empty-function
	discardAllCache(): void {}

	// -- debug --
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
		_part: number,
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

	// -- message handling --
	// eslint-disable-next-line @typescript-eslint/no-empty-function
	onTileMsg(_textMsg: string, _img: any): void {}

	// -- expiry / touch --
	getExpiryFactor(_tile: Tile): number {
		return 0;
	}
	// eslint-disable-next-line @typescript-eslint/no-empty-function
	touchImage(_tile: Tile): void {}

	// -- vector rendering --
	isVectorRendering(): boolean {
		return false;
	}
	requestThumbnail(
		_id: cool.PreviewId,
		_part: number,
		_maxWidth: number,
		_maxHeight: number,
		// eslint-disable-next-line @typescript-eslint/no-empty-function
	): void {}
	requestPart(_part: number): cool.VectorTileData | undefined {
		return undefined;
	}
	renderInto(
		_context: CanvasRenderingContext2D,
		_data: cool.VectorTileData,
		// eslint-disable-next-line @typescript-eslint/no-empty-function
	): void {}
	// eslint-disable-next-line @typescript-eslint/no-empty-function
	onVectorChanged(_callback: () => void): void {}
	// eslint-disable-next-line @typescript-eslint/no-empty-function
	handleVectorTileResponse(_values: cool.VectorTileResponse): void {}
	handleVectorRenderingGraphicsResponse(
		_values: cool.VectorRenderingGraphicsResponse,
		// eslint-disable-next-line @typescript-eslint/no-empty-function
	): void {}
	// eslint-disable-next-line @typescript-eslint/no-empty-function
	clearCachedPart(_part: number): void {}
	// eslint-disable-next-line @typescript-eslint/no-empty-function
	clearAllParts(): void {}

	// -- redraw --
	redraw(): RenderManagerBase {
		return this;
	}
}
