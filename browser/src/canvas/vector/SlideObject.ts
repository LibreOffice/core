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
		/// The aids that mark out a placeholder: a dashed boundary around
		/// the area it occupies and, on a master page, the name of the
		/// area. They are drawn apart from the object's own content.
		aids?: Primitive[];
		/// "page" for the entry that stands for the slide itself: it is
		/// drawn first and holds the background, the page fill and the
		/// master page content, and its box is the slide.
		/// "texteditoverlay" for an entry that carries the text of a
		/// running text edit: it is drawn last, over the object it runs
		/// on, which hides its own text while the edit runs. There is one
		/// per view that is editing, and two of them can name the same
		/// object. Absent for a drawing object.
		kind?: 'page' | 'texteditoverlay';
		/// Which view's text edit an entry of kind "texteditoverlay"
		/// carries, so a reader can tell its own from another user's.
		viewId?: number;
		/// Id of the group the object sits in, 0 for an object directly
		/// on the slide. A group's members follow it in the object list
		/// and draw its content, so a group with members has no
		/// primitives of its own.
		parent?: number;
		/// Id of the layer the object is on.
		layer?: number;
		/// True for a placeholder that holds no content of its own yet.
		emptyPlaceholder?: boolean;
		/// True while a text edit is running on the object. It shows none of
		/// its own text then, and the entry of kind "texteditoverlay"
		/// carries what has been typed.
		textEdit?: boolean;
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
