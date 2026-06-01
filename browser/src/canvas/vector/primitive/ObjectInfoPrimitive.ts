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
	/// Wrapper that tags a subtree with metadata: a name, a title
	/// and a description.
	export interface ObjectInfoPrimitive extends Primitive {
		type: typeof ObjectInfoPrimitive.type;
		name?: string;
		title?: string;
		desc?: string;
	}

	export namespace ObjectInfoPrimitive {
		export const type = 'objectInfo';
	}
}
