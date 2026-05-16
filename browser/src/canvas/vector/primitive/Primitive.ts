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
	/// A node in the JSON primitive tree.
	/// Concrete types extend this with per-type fields (color, path, text, ...).
	export interface Primitive {
		type?: string;
		children?: Primitive[];
	}
}
