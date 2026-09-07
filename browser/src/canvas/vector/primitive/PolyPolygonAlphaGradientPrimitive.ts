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
	/// A polygon in one color whose transparency follows a gradient.
	export interface PolyPolygonAlphaGradientPrimitive extends Primitive {
		type: typeof PolyPolygonAlphaGradientPrimitive.type;
		color?: string;
		path?: string;
		bounds?: number[]; // [minX, minY, maxX, maxY] in twips
		alphaGradient?: GradientAttribute;
	}

	export namespace PolyPolygonAlphaGradientPrimitive {
		export const type = 'polyPolygonAlphaGradient';
	}
}
