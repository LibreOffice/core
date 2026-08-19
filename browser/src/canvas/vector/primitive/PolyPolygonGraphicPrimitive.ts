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
	/// An image fill showing through a polygon. The image is placed in
	/// the unit square of definitionRange and clipped to the path.
	export interface PolyPolygonGraphicPrimitive extends Primitive {
		type: typeof PolyPolygonGraphicPrimitive.type;
		path?: string;
		bounds?: [number, number, number, number];
		definitionRange?: [number, number, number, number];
		fillGraphic?: FillGraphicAttribute;
		transparency?: number;
	}

	export namespace PolyPolygonGraphicPrimitive {
		export const type = 'polyPolygonGraphic';
	}
}
