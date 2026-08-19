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
	/// Parallel lines drawn distance apart at angle, in one, two or
	/// three crossing sets.
	export interface HatchAttribute {
		style?: 'single' | 'double' | 'triple';
		distance?: number; // twips
		angle?: number; // radians
		color?: string;
		fillBackground?: boolean; // the background shows under the lines
	}
}
