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
	/// The tail of a caption: its line with the line ends, or its hidden geometry. It keeps its
	/// end while the rest of the caption is transformed, and is laid out anew from it.
	export interface CaptionTailPrimitive extends Primitive {
		type: typeof CaptionTailPrimitive.type;
	}

	export namespace CaptionTailPrimitive {
		export const type = 'captionTail';
	}
}
