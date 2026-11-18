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
	/// The page list a vector-rendering part index addresses.
	export enum VectorMode {
		Slides = 0,
		MasterPages = 1,
		NotesPages = 2,
	}

	/// The id of a vector-rendering part: the globally unique id (GUID) of
	/// the page, the same value the status message names a page by. It names
	/// the page wherever the page sits in its list, so it survives a slide
	/// being inserted, removed or moved.
	export type VectorPartGuid = string;

	/// The key a page is filed under while it is known by its place in a
	/// list only: the mode and the index, joined with a colon.
	export function vectorIndexKey(part: number, mode: number): string {
		return String(mode) + ':' + String(part);
	}
}
