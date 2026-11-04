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
	/// The children, flipped by the matrix onto the range under them, faded
	/// from top to bottom and blurred by blur twips. The reflected object is
	/// a separate primitive. Without a range or children it draws nothing.
	///
	/// A transparency is 0 for opaque and 1 for clear. It is
	/// startTransparency down to startPosition, endTransparency from
	/// endPosition on, and linear between them. A position is a fraction of
	/// the height of the range, and endPosition >= startPosition.
	export interface ReflectionPrimitive extends Primitive {
		type: typeof ReflectionPrimitive.type;
		matrix?: number[];
		range?: [number, number, number, number]; // [minX, minY, maxX, maxY] in twips
		startTransparency?: number;
		startPosition?: number;
		endTransparency?: number;
		endPosition?: number;
		blur?: number; // twips
		bounds?: [number, number, number, number]; // the range grown by the blur, in twips
	}

	export namespace ReflectionPrimitive {
		export const type = 'reflection';
	}
}
