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

/*
	Dragging a handle that shapes the object itself: the corner radius of a rectangle and the
	points a custom shape is shaped by. What such a handle does to the object is the object's own
	business - a radius is not a transformation of it - so nothing is drawn of the object while
	it runs. The handle follows the mouse, and the engine is told where it was let go.
*/
class ShapingInteraction extends SelectionInteraction {
	/// The handle that was taken hold of.
	private handle: SelectionHandle;

	/// Where it has been taken, in twips.
	private at: cool.SimplePoint;

	constructor(selection: SelectionSection, handle: SelectionHandle) {
		super(selection);

		this.handle = handle;

		// Before the mouse has moved the handle stands where it stood, whatever part of it the
		// press landed on.
		this.at = new cool.SimplePoint(handle.point.x, handle.point.y);
	}

	/*
		Where the handle is drawn while it is dragged, which is where the mouse has taken it, held
		to whatever the object allows it. What that is, the object said when it sent the handle.
	*/
	private shownAt(): cool.Point {
		return this.selection.holdTheDrag(
			this.handle,
			new cool.Point(this.at.x, this.at.y),
		);
	}

	public move(to: cool.SimplePoint): void {
		this.at = this.led(to);
		this.selection.redraw();
	}

	/*
		The drag ends where the mouse let go, and what the engine is told about it is the business
		of the group the handle belongs to: a handle the engine knows is named to it, a point of a
		path is named as a point of that path, a point to tie a connector to is moved.
	*/
	public finish(to: cool.SimplePoint): void {
		this.move(to);

		this.selection.handOverTheDrag(this.handle, this.shownAt());
	}

	public handles(known: SelectionHandle[]): SelectionHandle[] {
		const shown = this.shownAt();

		// A point of a path carries its weights, and one weight can swing the other, so the
		// editor says where all three of them stand.
		const path = this.selection
			.pathEditor()
			?.handlesDuring(this.handle.name, shown, known);
		if (path) return path;

		return known.map((one: SelectionHandle) =>
			one.name === this.handle.name ? { ...one, point: shown } : one,
		);
	}

	public handleHeld(): SelectionHandle | null {
		return this.handle;
	}

	public leadingPoint(): cool.SimplePoint | null {
		const shown = this.shownAt();
		return new cool.SimplePoint(shown.x, shown.y);
	}

	/*
		The path the object would be drawn along while one of its points is dragged, which the
		editor of that path works out.
	*/
	public outline(): cool.ObjectPathPolygon[] | null {
		const path = this.selection
			.pathEditor()
			?.outlineFor(this.handle.name, this.shownAt());

		return (
			path ??
			this.selection.runWhileAnEndIsDragged(this.handle, this.shownAt()) ??
			this.selection.runWhileALineIsDragged(this.handle, this.shownAt())
		);
	}
}
