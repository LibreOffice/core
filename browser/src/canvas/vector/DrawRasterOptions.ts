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
	/// Modifiers a raster image can carry alongside the base matrix
	/// and checksum. Every field is optional, and an absent field
	/// means the plain image: uncropped, upright, unmirrored, in its
	/// own colours and fully opaque.
	export interface DrawRasterOptions {
		/// Present when the image is cropped. The whole image maps
		/// onto this rectangle of the unit square, so the parts of
		/// it that fall outside the square are the cropped away
		/// ones.
		imageRect?: GraphicPrimitive['imageRect'];
		/// Rotation in tenths of a degree around the centre of the
		/// unit square, which the matrix maps to the centre of the
		/// image in slide space.
		rotation?: number;
		/// Alpha in [0, 1] where 1 is opaque and 0 is fully
		/// see-through.
		alpha?: number;
		/// Bitfield of the flipped axes. Bit 0 flips horizontally
		/// and bit 1 flips vertically.
		mirror?: number;
		/// A recolour of the image: greys is desaturated, mono is
		/// one-bit black and white, and watermark is a faded grey.
		drawMode?: GraphicPrimitive['drawMode'];
	}
}
