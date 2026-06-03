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
	/// A clip path paired with a child subtree. clip is an SVG-D path.
	export interface MaskPrimitive extends Primitive {
		type: typeof MaskPrimitive.type;
		clip?: string;
	}

	export namespace MaskPrimitive {
		export const type = 'mask';
	}
}
