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

class Tile {
	coords: TileCoordData;
	distanceFromView: number = 0; // distance to the center of the nearest visible area (0 = visible)
	image: ImageBitmap | null = null; // ImageBitmap ready to render
	rawDeltas: cool.RawDelta[] = []; // deltas ready to decompress
	deltaCount: number = 0; // how many deltas on top of the keyframe
	updateCount: number = 0; // how many updates did we have
	loadCount: number = 0; // how many times did we get a new keyframe
	gcErrors: number = 0; // count freed keyframe in JS, but kept in wsd.
	missingContent: number = 0; // how many times rendered without content
	invalidateCount: number = 0; // how many invalidations touched this tile
	viewId: number = 0; // canonical view id
	wireId: number = 0; // monotonic timestamp for optimizing fetch
	invalidFrom: number = 0; // a wireId - for avoiding races on invalidation
	deltaId: number = 0; // monotonic id for delta updates
	lastPendingId: number = 0; // the id of the last delta requested to be decompressed
	decompressedId: number = 0; // the id of the last decompressed delta chunk in imgDataCache
	lastRendered: number = performance.timeOrigin;
	private lastRequestTime: Date = undefined; // when did we last do a tilecombine request.
	/// How long a tile that has just been asked for waits before it is asked for again.
	static readonly requestWaitMs = 5000;

	constructor(coords: TileCoordData) {
		this.coords = coords;
	}

	hasContent(): boolean {
		return !!this.image || this.hasKeyframe();
	}

	needsFetch() {
		return this.invalidFrom >= this.wireId || !this.hasContent();
	}

	needsRehydration(): boolean {
		if (this.rawDeltas.length === 0) return false;
		const lastId = this.rawDeltas[this.rawDeltas.length - 1].id;
		return this.lastPendingId !== lastId;
	}

	hasKeyframe(): boolean {
		return !!this.rawDeltas.length;
	}

	isReady(): boolean {
		return this.decompressedId === this.lastPendingId;
	}

	/// Demand a whole tile back to the keyframe from coolwsd.
	forceKeyframe(wireId: number = 0) {
		this.wireId = wireId;
		this.invalidFrom = wireId;
		this.allowFastRequest();
	}

	/// Avoid continually re-requesting tiles for eg. preloading
	requestingTooFast(now: Date): boolean {
		const tooFast: boolean =
			this.lastRequestTime &&
			now.getTime() - this.lastRequestTime.getTime() < Tile.requestWaitMs;
		return tooFast;
	}

	updateLastRequest(now: Date) {
		this.lastRequestTime = now;
	}

	/// Allow faster requests
	allowFastRequest() {
		this.updateLastRequest(undefined);
	}

	isReadyToDraw(): boolean {
		return !!this.image;
	}
}
