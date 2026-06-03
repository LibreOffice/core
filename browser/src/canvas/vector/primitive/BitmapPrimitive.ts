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
	/// An embedded raster image placed on the canvas through a 2D
	/// affine transform. The matrix maps the unit square (0,0)-(1,1)
	/// to the image's destination bounds in user units.
	///
	/// The image content is referenced by checksum. The decoded
	/// image lives in a separate cache, populated from a fetch.
	/// width and height are the source pixel dimensions of the
	/// original image.
	export interface BitmapPrimitive extends Primitive {
		type: typeof BitmapPrimitive.type;
		matrix?: number[];
		width?: number;
		height?: number;
		checksum?: number;
	}

	export namespace BitmapPrimitive {
		export const type = 'bitmap';
	}
}
