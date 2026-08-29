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
	/// The page list a vector-rendering part index addresses.
	export enum VectorMode {
		Slides = 0,
		MasterPages = 1,
		NotesPages = 2,
	}

	/// A part index paired with the mode that says which page list it
	/// addresses. The same index names a different page in each mode.
	export type VectorPartId = string;

	export function vectorPartId(part: number, mode: number): VectorPartId {
		return String(mode) + ':' + String(part);
	}
}
