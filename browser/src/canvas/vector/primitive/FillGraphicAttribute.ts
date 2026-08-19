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

namespace cool {
	/// An image used as a fill.
	export interface FillGraphicAttribute {
		checksum: number; // key into the bitmap cache
		graphicRange?: number[]; // [minX, minY, maxX, maxY] in unit coordinates
		tiling?: boolean; // the image repeats across the shape
		offsetX?: number; // 0 to 1 of a tile, every other row is offset by it
		offsetY?: number; // 0 to 1 of a tile, every other column is offset by it
	}
}
