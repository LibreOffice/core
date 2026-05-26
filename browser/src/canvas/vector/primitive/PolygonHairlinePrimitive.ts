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
	/// A polygon outlined with a one-pixel line. Carries no line
	/// attributes: hairlines are one pixel wide by definition.
	export interface PolygonHairlinePrimitive extends Primitive {
		type: typeof PolygonHairlinePrimitive.type;
		color?: string;
		path: string;
	}

	export namespace PolygonHairlinePrimitive {
		export const type = 'polygonHairline';
	}
}
