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
	/// A halo around the children, fading out over radius twips. Transparency
	/// is 0 opaque to 1 invisible. The glowing object is a separate primitive.
	export interface GlowPrimitive extends Primitive {
		type: typeof GlowPrimitive.type;
		color?: string;
		transparency?: number;
		radius?: number; // twips
		bounds?: [number, number, number, number]; // what the effect covers, in twips
	}

	export namespace GlowPrimitive {
		export const type = 'glow';
		/// Draws the object itself, not its text.
		export const isObjectDrawing = true;
	}
}
