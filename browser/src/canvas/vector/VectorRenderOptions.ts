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
	/// One piece of an area, in clip coordinates. A rectangle is x, y, width
	/// and height. A polygon is the x and y of each corner. An ellipse is its
	/// center and two radii. A slice is its center, radius, and start and end
	/// angle in radians clockwise from the x axis.
	export type AreaPiece =
		| { rectangle: number[] }
		| { polygon: number[] }
		| { ellipse: number[] }
		| { slice: number[] };

	/// The part drawn: the union of the pieces inside the frame, or with
	/// inverted the rest of the frame. Frame and pieces are in their own
	/// coordinates, which transform, the six canvas matrix values, maps onto
	/// twips. The frame is x, y, width and height. No two pieces overlap.
	export interface DrawClip {
		transform: number[];
		frame: number[];
		pieces: AreaPiece[];
		inverted?: boolean;
	}

	/// How an animation draws something: whether at all, the transform as
	/// the six canvas matrix values in twips, the opacity 0 to 1, the colors
	/// in place of its own, and the part drawn, before the transform. An
	/// absent field changes nothing.
	export interface DrawState {
		hidden?: boolean;
		transform?: number[];
		alpha?: number;
		colorOverride?: ColorOverride;
		clip?: DrawClip;
	}

	/// How an animation draws an object. paragraphs holds the paragraphs of
	/// its text with a state of their own, by index. Each is drawn in that
	/// state, and the object is drawn without them.
	export interface ObjectRenderState extends DrawState {
		paragraphs?: Map<number, DrawState>;
	}

	/// What a render of a part's primitive tree is for.
	export interface VectorRenderOptions {
		/// True when the render is the editable view of the document.
		editView?: boolean;
		/// True when fills and lines keep the colors the document gives them, whatever the theme
		/// of the view. False or absent when they follow a dark view.
		authoredColors?: boolean;
		/// What is being edited: an entered group, or an object active in place. Objects in front
		/// of it that cover part of it are drawn transparent; with paleOutside every object of
		/// the page outside it is also drawn pale, mixed towards the document background. Absent
		/// where nothing is.
		edited?: { objectId: number; paleOutside: boolean };
		/// The object active in place, a chart being edited, and the upright rectangle its window
		/// fills, as x, y, width and height in twips, which is what it covers of the page while
		/// its parts are drawn upright into it. Absent where no object is active in place.
		activeInPlace?: { objectId: number; area: number[] };
		/// The states of the animated objects, by object id. The members of
		/// a group take the state of the group. Absent when nothing is
		/// animated.
		states?: Map<number, ObjectRenderState>;
		/// The positions in the paint order of the part that are drawn, from
		/// start up to but not including end. The page entry is at position
		/// 0, and the master draws with it. Absent where the whole order is.
		paintRange?: { start: number; end: number };
	}
}
