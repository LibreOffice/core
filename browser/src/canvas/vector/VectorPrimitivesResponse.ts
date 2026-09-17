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
		/// The page list the page belongs to: 0 the slides, 1 the master
		/// pages, 2 the notes pages. Defaults to the slides.
		mode?: number;
		/// The id of the page, its GUID. Absent only when the request named no
		/// page.
		partId?: VectorPartGuid;
		/// The version space the versions below count in. It is drawn once per document
		/// in the engine, so versions carrying two different epochs describe two different
		/// documents and mean nothing to each other.
		epoch?: number;
		/// The version the part is at once everything in this response is applied.
		version?: number;
		/// The newest input of this view that the content reflects, when
		/// known.
		inputSequence?: number;
		/// In a delta, the version it was compared against. The delta carries the objects that
		/// changed after that version.
		from?: number;
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
