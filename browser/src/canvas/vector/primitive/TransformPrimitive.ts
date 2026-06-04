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
	/// A 2D affine transform applied to the wrapped child subtree.
	///
	/// The wire format ships the matrix as six values in column-major
	/// canvas order [a, b, c, d, e, f]:
	///
	///     | a  c  e |
	///     | b  d  f |
	///     | 0  0  1 |
	export interface TransformPrimitive extends Primitive {
		type: typeof TransformPrimitive.type;
		matrix?: number[];
	}

	export namespace TransformPrimitive {
		export const type = 'transform';
	}
}
