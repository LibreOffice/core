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
	/// A raster image with an optional uniform transparency. The
	/// matrix maps the unit square (0,0)-(1,1) to the image's
	/// destination bounds. transparency is in [0, 1] where 0 is
	/// opaque and 1 is fully see-through. width and height are the
	/// source pixel dimensions of the original image.
	export interface BitmapAlphaPrimitive extends Primitive, GraphicResource {
		type: typeof BitmapAlphaPrimitive.type;
		matrix?: number[];
		width?: number;
		height?: number;
		transparency?: number;
	}

	export namespace BitmapAlphaPrimitive {
		export const type = 'bitmapAlpha';
	}
}
