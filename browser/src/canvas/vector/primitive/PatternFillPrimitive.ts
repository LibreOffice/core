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
	/// A child subtree repeated as tiles inside a polygon. The children
	/// are drawn in the unit square, and each tile maps that square onto
	/// itself.
	export interface PatternFillPrimitive extends Primitive {
		type: typeof PatternFillPrimitive.type;
		path?: string; // the polygon, as SVG path data in twips
		bounds?: [number, number, number, number]; // its bounding box in twips
		referenceRange?: [number, number, number, number]; // one tile, in the unit square of bounds
	}

	export namespace PatternFillPrimitive {
		export const type = 'patternFill';
	}
}
