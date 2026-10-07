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
	Moving the crop area of a picture over its graphic by the handle in its middle: the whole
	graphic stays where it is on the page, and the window onto it - the picture as it is drawn -
	goes with the handle, its size kept. The crop values change by as much as the window moved,
	so the graphic does not move with it. The window may go beyond the graphic, as a crop mark may.

	The move is worked out on the picture's own square, so it runs along the picture however the
	picture is turned or sheared, and what is drawn while it runs is the crop preview: the window
	moved over the faint whole graphic. At the end the engine is told the window the move left.
*/
class CropPanInteraction extends SelectionInteraction {
	/// The handle in the middle of the picture.
	private readonly handle: SelectionHandle;
	/// The picture's square when the move began.
	private readonly square: cool.Matrix2D | undefined;
	/// The place on the square the move is counted from.
	private readonly from: cool.Point | undefined;
	/// How far the window has gone along each axis of the square, in shares of it.
	private across: number = 0;
	private down: number = 0;
	/// Where the mouse was told to be last.
	private at: cool.SimplePoint;

	constructor(selection: SelectionSection, handle: SelectionHandle) {
		super(selection);

		this.handle = handle;
		this.at = new cool.SimplePoint(handle.point.x, handle.point.y);
		this.square = selection.squareNow();
		// The move is counted from the middle of the window as the engine last gave it, which
		// is where the handle stands until a move is kept here ahead of the engine's answer.
		this.from = this.square ? new cool.Point(0.5, 0.5) : undefined;
	}

	public isACrop(): boolean {
		return true;
	}

	public ghostTransformation(): cool.Matrix2D | null {
		return null;
	}

	public handleHeld(): SelectionHandle | null {
		return this.handle;
	}

	public leadingPoint(): cool.SimplePoint | null {
		return this.at.clone();
	}

	public move(to: cool.SimplePoint): void {
		this.at = to.clone();
		if (!this.square || !this.from) return;

		const led = this.led(to);
		const asked = this.square.invert()?.apply(led.x, led.y);
		if (!asked) return;

		this.across = asked.x - this.from.x;
		this.down = asked.y - this.from.y;

		this.selection.redraw();
	}

	/// The move of the window on the page: along the square's axes by the shares it went.
	public transformation(): cool.Matrix2D | null {
		if (!this.square || (!this.across && !this.down)) return null;

		const { a, b, c, d } = this.square;
		return cool.Matrix2D.IDENTITY.translate(
			this.across * a + this.down * c,
			this.across * b + this.down * d,
		);
	}

	/// The handles go with the window.
	public handles(known: SelectionHandle[]): SelectionHandle[] {
		const matrix = this.transformation();
		if (!matrix) return known;

		return known.map((one: SelectionHandle) => ({
			...one,
			point: matrix.apply(one.point.x, one.point.y).round(),
		}));
	}

	/// The engine is told the window the move left.
	public finish(to: cool.SimplePoint): void {
		this.move(to);

		const matrix = this.transformation();
		const objectId = this.selection.selectedObjects()[0];
		if (!matrix || !this.square || objectId === undefined) return;

		CropHandles.cropTo(objectId, this.square.then(matrix));
	}
}
