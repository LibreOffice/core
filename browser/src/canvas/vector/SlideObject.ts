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
	/// One drawable object on a slide, carrying its primitive tree.
	export interface SlideObject {
		/// Stable identity of the object: the engine's SdrObject unique
		/// id, unchanged across edits to the same object.
		id?: number;
		/// Id of the group the object sits in, 0 for an object directly
		/// on the slide. A group's members follow it in the object list
		/// and draw its content, so a group with members has no
		/// primitives of its own.
		parent?: number;
		/// Id of the layer the object is on.
		layer?: number;
		/// True for a placeholder that holds no content of its own yet.
		emptyPlaceholder?: boolean;
		/// Rectangle the object paints, in twips: the primitives' range,
		/// so it takes in the line width and a shadow.
		x?: number;
		y?: number;
		width?: number;
		height?: number;
		/// Mapping of the unit square onto the object, in twips, as the
		/// six canvas matrix values [a, b, c, d, e, f].
		transform?: number[];
		primitives?: Primitive[];
	}
}
