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
	The polygon of a path a press took hold of by its line: the editor of the path, which polygon
	of it, the name the editor gives each handle of the selection, and where the move goes once
	it is let go.
*/
interface HeldPolygon {
	editing: PathEditing;
	polygon: number;
	/// The name the editor of the path gives that handle, or nothing for a handle of another
	/// path or of no path.
	inPath: (handleName: string) => string | null;
	/// Takes the move, by that offset in whole twips.
	handOver: (by: cool.Point) => void;
}

/*
	Moving one polygon of a path, taken hold of by its line rather than by a handle: every point
	of it and every weight go by the same offset, and the rest of the path stays. A path of one
	polygon moves as a whole. The polygon is drawn thin where the drag has it, and its handles go
	along.
*/
class PolygonDrag extends SelectionInteraction {
	private held: HeldPolygon;

	/// Where the mouse was in the document when the drag began, in twips.
	private from: cool.SimplePoint;

	/// Where the mouse is now, in twips.
	private at: cool.SimplePoint;

	constructor(
		selection: SelectionSection,
		held: HeldPolygon,
		at: cool.SimplePoint,
	) {
		super(selection);

		this.held = held;
		this.from = at.clone();
		this.at = at.clone();
	}

	/// How far the polygon has been taken, in whole twips, as the engine is told it.
	private offset(): cool.Point {
		return new cool.Point(
			this.at.x - this.from.x,
			this.at.y - this.from.y,
		).round();
	}

	public move(to: cool.SimplePoint): void {
		this.at = to.clone();
		this.selection.redraw();
	}

	public finish(to: cool.SimplePoint): void {
		this.move(to);

		const by = this.offset();
		if (by.x || by.y) this.held.handOver(by);
	}

	public handles(known: SelectionHandle[]): SelectionHandle[] {
		const by = this.offset();

		return known.map((handle: SelectionHandle) => {
			const inPath = this.held.inPath(handle.name);
			return inPath !== null &&
				PathEditing.belongsToPolygon(inPath, this.held.polygon)
				? { ...handle, point: handle.point.add(by) }
				: handle;
		});
	}

	public leadingPoint(): cool.SimplePoint | null {
		return this.at;
	}

	/// The path with the polygon where the drag has it.
	public outline(): cool.ObjectPathPolygon[] | null {
		return this.held.editing.pathWithPolygonMoved(
			this.held.polygon,
			this.offset(),
		);
	}
}
