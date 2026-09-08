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
	/// Interface for the vector primitives response from core.
	export interface VectorPrimitivesResponse {
		part?: number;
		version?: number;
		/// Every painted object in paint order in a full response, the
		/// entry for the page itself first. In a delta only the changed
		/// ones, the page entry among them when its content changed.
		objects?: SlideObject[];
		/// In a delta, the ids of every live object on the part in paint
		/// order, the page entry first. Objects not listed in this array
		/// are gone. Objects listed but absent from "objects" keep their
		/// cached content.
		order?: number[];
	}
}
