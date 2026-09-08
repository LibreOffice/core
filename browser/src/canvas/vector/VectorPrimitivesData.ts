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
	/// Vector representation of a slide.
	export interface VectorPrimitivesData {
		/// Content version the engine reported for this part. It counts
		/// up each time the part changes, so two trees with the same
		/// version describe the same content.
		version?: number;
		/// The page rectangle in twips, taken from the entry that stands
		/// for the page.
		slideWidth: number;
		slideHeight: number;
		/// Every painted object in paint order, the page entry first.
		objects: SlideObject[];
	}
}
