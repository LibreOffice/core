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
	/// A child subtree whose edge fades out over radius twips inwards.
	export interface SoftEdgePrimitive extends Primitive {
		type: typeof SoftEdgePrimitive.type;
		radius?: number; // twips
		bounds?: [number, number, number, number]; // what the effect covers, in twips
	}

	export namespace SoftEdgePrimitive {
		export const type = 'softEdge';
		/// Draws the object itself, not its text.
		export const isObjectDrawing = true;
	}
}
