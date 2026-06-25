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
		slideWidth?: number;
		slideHeight?: number;
		masterPage?: SlideObject;
		objects?: SlideObject[];
		/// In a delta, the ids of every live object on the part in
		/// z-order. Objects not listed in this array are gone. Objects
		/// listed but absent from "objects" keep their cached content.
		order?: number[];
	}
}
