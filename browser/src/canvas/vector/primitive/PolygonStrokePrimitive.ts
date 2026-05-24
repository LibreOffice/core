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
	/// A polygon with a stroke (outline) and no fill.
	export interface PolygonStrokePrimitive extends Primitive {
		type: typeof PolygonStrokePrimitive.type;
		path: string;
		line?: {
			color?: string;
			width?: number;
			linejoin?: 'miter' | 'bevel' | 'round' | 'none' | 'unknown';
			linecap?: 'butt' | 'round' | 'square' | 'unknown';
		};
		stroke?: {
			dotDashArray?: number[];
			fullDotDashLen?: number;
		};
	}

	export namespace PolygonStrokePrimitive {
		export const type = 'polygonStroke';
	}
}
