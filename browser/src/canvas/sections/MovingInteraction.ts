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
	Moving the whole selection, as taking hold of an object and carrying it does. It is begun by
	a press on the object itself rather than on a handle, and it ends by telling the engine where
	the selection now stands.
*/
class MovingInteraction extends SelectionInteraction {
	/// Where the mouse was in the document when the move began, in twips.
	private from: cool.SimplePoint;

	/// Where the mouse was told to be last, which the view keeps in sight.
	private at: cool.SimplePoint;

	/// How far it has come since, in twips.
	private across: number = 0;
	private down: number = 0;

	constructor(selection: SelectionSection, at: cool.SimplePoint) {
		super(selection);

		this.from = at.clone();
		this.at = at.clone();
	}

	public move(to: cool.SimplePoint, event?: MouseEvent): void {
		this.at = to.clone();
		this.across = to.x - this.from.x;
		this.down = to.y - this.from.y;

		// What the selection could line up with, from where the move has taken it.
		this.selection.lookForASnapOfTheWhole(
			this.across * app.twipsToPixels,
			this.down * app.twipsToPixels,
			event,
		);

		this.selection.redraw();
	}

	/*
		Hands the engine the mapping every object of the selection is to be drawn by, which is the
		one it was drawn by here while the move ran, with whatever it lined up with taken into
		account.
	*/
	public finish(to: cool.SimplePoint, event?: MouseEvent): void {
		this.move(to, event);

		if (!this.across && !this.down) return;

		const matrix = this.transformation();
		if (matrix)
			SelectionSection.sendTransform(
				this.selection.selectedObjects(),
				matrix,
				'move',
			);
	}

	public scrollsWithTheMouse(): boolean {
		return true;
	}

	public leadingPoint(): cool.SimplePoint | null {
		return this.at.clone();
	}

	public transformation(): cool.Matrix2D | null {
		if (!this.across && !this.down) return null;

		const corner = this.selection.selectionCorner();
		const stands = this.applyTheSnap(corner);

		// The selection stops at the edge of the lawn, so nothing is moved out of reach.
		const by = this.selection.heldInsideTheLawn(
			new cool.Point(
				(stands[0] - corner[0]) * app.pixelsToTwips,
				(stands[1] - corner[1]) * app.pixelsToTwips,
			),
		);
		return cool.Matrix2D.IDENTITY.translate(by.x, by.y);
	}

	/*
		Applies the snap: the upper left corner of the selection as the mouse would leave it, moved
		onto whatever the selection lined up with on the page. Where it lined up with nothing it
		is where the mouse leaves it.
	*/
	private applyTheSnap(corner: number[]): number[] {
		return this.selection.snappedCorner(
			corner[0] + this.across * app.twipsToPixels,
			corner[1] + this.down * app.twipsToPixels,
		);
	}

	/*
		Every handle goes through the very change the objects are drawn with, so that where the
		move lines up with something on the page the handles and the frame go there as well.
		Moving them by how far the mouse went instead would leave them beside what is drawn.
	*/
	public handles(known: SelectionHandle[]): SelectionHandle[] {
		const matrix = this.transformation();
		if (!matrix) return known;

		return known.map((handle: SelectionHandle) => ({
			...handle,
			point: matrix.apply(handle.point.x, handle.point.y).round(),
		}));
	}
}
