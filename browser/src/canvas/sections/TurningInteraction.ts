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
	Turning the selection around its middle by the handle that sits above it. The handle is the
	client's own, the engine has none of the kind, so the turn is worked out here and what the
	engine is told at the end is a transformation of the objects.
*/
class TurningInteraction extends SelectionInteraction {
	/// The middle the selection turns around, in twips.
	private middle: cool.SimplePoint;

	/// The angle from that middle to the mouse when the turn began.
	private from: number;

	/// How far it has turned since, as the canvas counts angles.
	private turned: number = 0;

	constructor(
		selection: SelectionSection,
		middle: cool.SimplePoint,
		at: cool.SimplePoint,
	) {
		super(selection);

		this.middle = middle;
		this.from = TurningInteraction.angleAt(middle, at);
	}

	/// The angle from the middle to that point, as the canvas counts angles.
	private static angleAt(
		middle: cool.SimplePoint,
		point: cool.SimplePoint,
	): number {
		return Math.atan2(point.y - middle.y, point.x - middle.x);
	}

	/*
		Turns the objects of a selection around a point by that many degrees, counted against the
		clock. Every object is handed the mapping it is to be drawn by, which is the one it would
		be drawn by here, so that what is shown and what is sent are the same thing.
	*/
	public static turnObjects(
		selection: SelectionSection,
		middle: cool.SimplePoint,
		degrees: number,
	): cool.Matrix2D | null {
		if (!degrees) return null;

		const matrix = TurningInteraction.turnAbout(middle, degrees);

		SelectionSection.sendTransform(selection.selectedObjects(), matrix, 'turn');

		return matrix;
	}

	/// The change that turns the objects around that point by that many degrees, counted against
	/// the clock. The canvas counts an angle with the clock, so it is handed the other sign.
	public static turnAbout(
		middle: cool.SimplePoint,
		degrees: number,
	): cool.Matrix2D {
		return cool.Matrix2D.IDENTITY.rotateAround(
			middle.x,
			middle.y,
			(-degrees * Math.PI) / 180,
		);
	}

	/*
		Follows the mouse. While a modifier is held the angle the selection ends up standing at is
		snapped to the step it names, counted from the page and not from where the turn began, so
		a turned object goes straight back to standing upright. The angle the selection stands at
		is that of the across axis of its square: an object's own, nought for a frame around
		several, which then snaps the turn itself.
	*/
	public move(
		to: cool.SimplePoint,
		event?: { ctrlKey?: boolean; shiftKey?: boolean; altKey?: boolean },
	): void {
		this.turned = TurningInteraction.angleAt(this.middle, to) - this.from;

		const step = EditSteps.turnSnap(event);
		const square = this.selection.squareNow();
		if (step && square) {
			const stands = square.rotation();
			const stepInRadians = (step * Math.PI) / 180;
			const snapped =
				Math.round((stands + this.turned) / stepInRadians) * stepInRadians;
			this.turned = snapped - stands;
		}

		this.selection.redraw();
	}

	public finish(
		to: cool.SimplePoint,
		event?: { ctrlKey?: boolean; shiftKey?: boolean; altKey?: boolean },
	): void {
		this.move(to, event);

		// The canvas counts an angle with the clock, the engine against it.
		TurningInteraction.turnObjects(
			this.selection,
			this.middle,
			(-this.turned * 180) / Math.PI,
		);
	}

	public transformation(): cool.Matrix2D | null {
		if (!this.turned) return null;

		return TurningInteraction.turnAbout(
			this.middle,
			(-this.turned * 180) / Math.PI,
		);
	}

	public handles(known: SelectionHandle[]): SelectionHandle[] {
		const matrix = cool.Matrix2D.IDENTITY.rotateAround(
			this.middle.x,
			this.middle.y,
			this.turned,
		);

		// The axes turn with the selection, so the way each of the eight faces is worked out
		// again from where they stand now.
		return ObjectHandles.facedAwayFromTheirPlaces(
			known.map((handle: SelectionHandle) => ({
				...handle,
				point: matrix.apply(handle.point.x, handle.point.y).round(),
			})),
		);
	}
}
