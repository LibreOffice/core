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
	/// A gradient filling a polygon. The ramp is laid out across
	/// definitionRange and shows through the path.
	export interface PolyPolygonGradientPrimitive extends Primitive {
		type: typeof PolyPolygonGradientPrimitive.type;
		path?: string;
		bounds?: number[]; // [minX, minY, maxX, maxY] in twips
		definitionRange?: number[]; // [minX, minY, maxX, maxY] in twips
		gradient?: GradientAttribute;
		alphaGradient?: GradientAttribute;
		transparency?: number;
	}

	export namespace PolyPolygonGradientPrimitive {
		export const type = 'polyPolygonGradient';
	}
}
