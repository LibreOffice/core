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
	/// The children masked by the transparence subtree, read as a gray image:
	/// black opaque, white hidden.
	export interface TransparencePrimitive extends Primitive {
		type: typeof TransparencePrimitive.type;
		transparence?: Primitive[];
		bounds?: [number, number, number, number]; // what the effect covers, in twips
	}

	export namespace TransparencePrimitive {
		export const type = 'transparence';
	}
}
