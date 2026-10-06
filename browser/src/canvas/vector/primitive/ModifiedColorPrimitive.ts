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
	/// A color change applied to the wrapped subtree.
	export interface ModifiedColorPrimitive
		extends Primitive, ColorModifierAttribute {
		type: typeof ModifiedColorPrimitive.type;
		bounds?: [number, number, number, number]; // what the subtree covers, in twips
	}

	export namespace ModifiedColorPrimitive {
		export const type = 'modifiedColor';
	}
}
