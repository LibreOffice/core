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
	/// A graphic placed on the canvas. The wire ships two shapes
	/// under this primitive type.
	///
	/// When vector is true, the graphic has been decomposed into the
	/// children subtree. The children are already in slide coordinates
	/// and render through the normal primitive pipeline.
	///
	/// Otherwise the graphic is raster. The matrix maps the unit
	/// square (0,0)-(1,1) to the destination bounds. The image
	/// content is referenced by checksum, the decoded image lives in
	/// a separate cache populated from a fetch. width and height are
	/// the source pixel dimensions of the original image. SVG
	/// graphics use the same shape and are decoded natively by
	/// HTMLImageElement once the cache entry arrives.
	///
	/// The raster case also carries optional rendering modifiers
	/// (crop, alpha, rotation, mirror, drawMode). They are declared
	/// here for completeness. The current renderer ignores them.
	export interface GraphicPrimitive extends Primitive, GraphicResource {
		type: typeof GraphicPrimitive.type;
		vector?: boolean;
		matrix?: number[];
		width?: number;
		height?: number;
		crop?: {
			left?: number;
			top?: number;
			right?: number;
			bottom?: number;
		};
		alpha?: number;
		rotation?: number;
		mirror?: number;
		drawMode?: 'greys' | 'mono' | 'watermark';
	}

	export namespace GraphicPrimitive {
		export const type = 'graphic';
	}
}
