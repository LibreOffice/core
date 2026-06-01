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
	/// A colour transformation applied to the wrapped subtree.
	///
	/// modifier names the kind of transformation. Known values
	/// include "gray", "luminance_to_alpha", "invert" and "replace".
	/// color is only set for the "replace" transformation, which
	/// carries the substitute colour.
	export interface ModifiedColorPrimitive extends Primitive {
		type: typeof ModifiedColorPrimitive.type;
		modifier?: string;
		color?: string;
	}

	export namespace ModifiedColorPrimitive {
		export const type = 'modifiedColor';
	}
}
