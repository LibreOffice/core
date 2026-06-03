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
	/// A list of single-pixel points painted in one colour.
	export interface PointArrayPrimitive extends Primitive {
		type: typeof PointArrayPrimitive.type;
		color?: string;
		points?: { x: number; y: number }[];
	}

	export namespace PointArrayPrimitive {
		export const type = 'pointArray';
	}
}
