/* -*- js-indent-level: 8 -*- */
/*
 * Copyright the Collabora Office contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

namespace cool {
	/// The children in the shadow color, moved by the matrix and blurred by
	/// blur twips. The casting object is a separate primitive.
	export interface ShadowPrimitive extends Primitive {
		type: typeof ShadowPrimitive.type;
		color?: string;
		blur?: number; // twips
		matrix?: number[];
		bounds?: [number, number, number, number]; // what the moved effect covers, in twips
	}

	export namespace ShadowPrimitive {
		export const type = 'shadow';
	}
}
