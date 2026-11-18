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
	/// Where the primitives of the entries of a part are placed. In 'page' they are in the
	/// twips of the page. In 'entry' the primitives of each entry start at the top left of the
	/// box of that entry.
	export type VectorCoordinates = 'page' | 'entry';

	/// Vector representation of a slide.
	export interface VectorPrimitivesData {
		/// The id of the page this is the content of.
		partId: VectorPartGuid;
		/// The page list the page belongs to: 0 the slides, 1 the master
		/// pages, 2 the notes pages.
		mode: number;
		/// The version space the version counts in, as the engine named it.
		/// Versions with two different epochs come from two different models
		/// and say nothing about each other.
		epoch?: number;
		/// Content version the engine reported for this part. It counts
		/// up each time the part changes, so two trees with the same
		/// version describe the same content.
		version?: number;
		/// Where the primitives of the entries are placed. Without a value they are in the
		/// twips of the page.
		coordinates?: VectorCoordinates;
		/// The page rectangle in twips, taken from the entry that stands
		/// for the page.
		slideWidth: number;
		slideHeight: number;
		/// Every painted object of the part, keyed by its id, so an update
		/// can reach one object without touching the rest.
		objects: Map<number, SlideObject>;
		/// The ids in paint order, the page entry first and the members of
		/// a group right after it.
		order: number[];
		/// Id of the master part the page draws under itself, or undefined
		/// when the page carries its master content inline.
		masterPartId?: VectorPartGuid;
		/// The ids of the layers of that master the page does not show.
		masterHiddenLayers?: Set<number>;
	}
}
