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
	/// Mixin for primitives that reference an image by checksum.
	/// The decoded image lives in the bitmap cache and is fetched
	/// separately.
	export interface GraphicResource {
		checksum: number;
	}

	export namespace GraphicResource {
		/// Type guard for primitives that carry a numeric checksum.
		export function is(
			primitive: Primitive,
		): primitive is Primitive & GraphicResource {
			return (
				typeof (primitive as Partial<GraphicResource>).checksum === 'number'
			);
		}
	}
}
