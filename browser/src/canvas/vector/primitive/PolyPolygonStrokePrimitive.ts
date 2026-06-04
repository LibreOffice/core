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
	/// Multiple polygons sharing one stroke. The path field is an
	/// SVG-D string that may describe more than one disjoint subpath.
	export interface PolyPolygonStrokePrimitive extends Omit<
		PolygonStrokePrimitive,
		'type'
	> {
		type: typeof PolyPolygonStrokePrimitive.type;
	}

	export namespace PolyPolygonStrokePrimitive {
		export const type = 'polyPolygonStroke';
	}
}
