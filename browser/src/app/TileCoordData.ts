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

class TileCoordData {
	x: number;
	y: number;
	z: number;
	part: PartNumber;
	mode: number;

	/*
		No need to calculate the scale every time. We have the current scale, reachable via app.getScale().
		We can compare these two when we need check if a tile is in the current scale. Assigned on creation.
	*/
	scale: number;

	constructor(
		left: number,
		top: number,
		zoom: number = null,
		part: PartNumber = null,
		mode: number = null,
	) {
		this.x = left;
		this.y = top;
		this.z = zoom !== null ? zoom : app.map.getZoom();
		this.part = part !== null ? part : app.map._docLayer.getSelectedPart();
		this.mode = mode !== null ? mode : 0;

		this.scale = Math.pow(1.2, this.z - 10);
	}

	getPos() {
		return new cool.Point(this.x, this.y);
	}

	// Returns SimplePoint. To replace getPos in the short term.
	getPosSimplePoint() {
		return cool.SimplePoint.fromCorePixels(
			[this.x, this.y],
			this.part,
			this.mode,
		);
	}

	key(): string {
		return (
			this.x +
			':' +
			this.y +
			':' +
			this.z +
			':' +
			this.part +
			':' +
			(this.mode !== undefined ? this.mode : 0)
		);
	}

	toString(): string {
		return (
			'{ left : ' +
			this.x +
			', top : ' +
			this.y +
			', z : ' +
			this.z +
			', part : ' +
			this.part +
			', mode : ' +
			this.mode +
			' }'
		);
	}

	public static parseKey(key: string): TileCoordData {
		window.app.console.assert(
			typeof key === 'string',
			'key should be a string',
		);
		const k = key.split(':');
		const mode = k.length === 4 ? +k[4] : 0;
		window.app.console.assert(k.length >= 5, 'invalid key format');
		// The key was built from a part number, so the parsed field is one.
		return new TileCoordData(+k[0], +k[1], +k[2], +k[3] as PartNumber, mode);
	}

	public static keyToTileCoords(key: string): TileCoordData {
		return TileCoordData.parseKey(key);
	}
}
