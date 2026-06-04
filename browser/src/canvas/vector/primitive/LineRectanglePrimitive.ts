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
	/// Axis-aligned rectangle outlined with a one-pixel line.
	/// bounds is [minX, minY, maxX, maxY].
	export interface LineRectanglePrimitive extends Primitive {
		type: typeof LineRectanglePrimitive.type;
		color?: string;
		bounds?: [number, number, number, number];
	}

	export namespace LineRectanglePrimitive {
		export const type = 'lineRectangle';
	}
}
