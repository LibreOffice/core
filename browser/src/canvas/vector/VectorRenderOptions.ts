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
	/// What a render of a part's primitive tree is for.
	export interface VectorRenderOptions {
		/// True when the render is the editable view of the document.
		editView?: boolean;
		/// What is being edited: an entered group, or an object active in place. Objects in front
		/// of it that cover part of it are drawn transparent; with paleOutside every object of
		/// the page outside it is also drawn pale, mixed towards white. Absent where nothing is.
		edited?: { objectId: number; paleOutside: boolean };
		/// The object active in place, a chart being edited, and the upright rectangle its window
		/// fills, as x, y, width and height in twips. The object is drawn into that rectangle.
		/// Absent where no object is active in place.
		activeInPlace?: { objectId: number; area: number[] };
	}
}
