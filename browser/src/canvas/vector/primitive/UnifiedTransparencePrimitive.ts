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
	/// A single uniform alpha applied to the wrapped subtree.
	///
	/// transparence is a value between 0 and 1, where 0 means fully
	/// opaque and 1 means fully transparent.
	export interface UnifiedTransparencePrimitive extends Primitive {
		type: typeof UnifiedTransparencePrimitive.type;
		transparence?: number;
	}

	export namespace UnifiedTransparencePrimitive {
		export const type = 'unifiedTransparence';
	}
}
