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
	/// Fills the entire drawing area with a solid color (slide background).
	export interface BackgroundColorPrimitive extends Primitive {
		type: typeof BackgroundColorPrimitive.type;
		color: string;
		transparency?: number;
	}

	export namespace BackgroundColorPrimitive {
		export const type = 'backgroundcolor';
	}
}
