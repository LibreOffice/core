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
	/// A polygon filled with one color and an optional transparency.
	/// The wire field transparency is in [0, 1] where 0 is fully opaque.
	export interface PolyPolygonRGBAPrimitive extends Primitive {
		type: typeof PolyPolygonRGBAPrimitive.type;
		path: string;
		color?: string;
		transparency?: number;
	}

	export namespace PolyPolygonRGBAPrimitive {
		export const type = 'polyPolygonRGBA';
	}
}
