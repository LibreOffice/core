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
	Shearing the selection around its middle by the handle above it, in the handle's shearing
	mode. It works as the office's slant does: the selection's upright axis turns by the angle
	the mouse has gone round the middle, and keeps its length, while its across axis stays. That
	is a shear together with a scale of the upright axis, so the handle never runs off as a pure
	shear would. Between ninety and two hundred and seventy degrees the selection stands upside
	down, which turns it over, and the shear is held to eighty-nine degrees either way. Only the
	point it is worked around differs from the office: the middle of the selection, as for a
	turn, not the side facing the handle.

	The slant goes on from the one there was: taking hold of the handle changes nothing, and the
	angle the mouse goes round the middle is added to it. Beyond the handle a mark stands where
	the slant is nought, which resets it.
*/
class ShearingInteraction extends SelectionInteraction {
	/// The middle the selection is sheared around, in twips.
	private middle: cool.SimplePoint;

	/// The angle from that middle to the mouse when the drag began.
	private from: number;

	/// The slant the selection gets, as the canvas counts angles: the one there was and how far
	/// the mouse has gone round the middle since the press, snapped.
	private slantTo: number = 0;

	/// How far the press was from the middle, in twips, which is the circle the handle runs on.
	private radius: number;

	/*
		The axis of the object the slant is read from that runs down from its upper side, in the
		slant's own coordinates: how far it goes across and how far down. It is the first
		selected object's, and it is what the slant changes.
	*/
	private down: cool.Point | null = null;

	/// The slant there was, as the canvas counts angles, read from that axis.
	private startSlant: number = 0;

	/// The way straight up from the middle, at a right angle to the upper side, as the canvas
	/// counts angles: where the handle of the selection without a slant stands.
	private upright: number;

	/*
		Maps the slant's own coordinates onto the page: the selection's upper side runs across,
		the way away from the handle runs down, and the middle is at nought. The slant is worked
		out there and taken onto the page through it.
	*/
	private frame: cool.Matrix2D;

	constructor(
		selection: SelectionSection,
		middle: cool.SimplePoint,
		at: cool.SimplePoint,
		turnedBy: number,
	) {
		super(selection);

		this.middle = middle;
		this.from = ShearingInteraction.angleAt(middle, at);
		this.radius = Math.hypot(at.x - middle.x, at.y - middle.y);

		/*
			A single object is sheared from the slant it has, read from its own mapping. Several
			objects, or a group, whose own mapping is a plain rectangle, are sheared as the
			selection's square, which starts upright and without a slant and carries what was
			done to it since, as a turn does. The frame lies along the upper side of what is read,
			the way round that the handle is turned by.
		*/
		const objects = selection.selectedObjects();
		const drawing = RenderGeometrySection.theOnesThatDraw(
			RenderGeometrySection.standingFor(objects),
		);
		const single =
			objects.length === 1 && drawing.length === 1 && drawing[0] === objects[0];
		const mapping = single
			? RenderGeometrySection.transformOf(objects[0])
			: selection.squareNow();

		let turn = turnedBy;
		if (mapping) {
			const own = Math.atan2(mapping.b, mapping.a);
			turn = Math.cos(own - turnedBy) >= 0 ? own : own + Math.PI;
		}

		const cos = Math.cos(turn);
		const sin = Math.sin(turn);
		this.frame = new cool.Matrix2D(cos, sin, -sin, cos, middle.x, middle.y);
		this.upright = Math.atan2(-cos, sin);

		// The axis down from the upper side of what is read, taken into the slant's coordinates.
		// A slant turns that axis from straight down, so the angle it stands at is the slant.
		if (mapping) {
			this.down = new cool.Point(
				mapping.c * cos + mapping.d * sin,
				-mapping.c * sin + mapping.d * cos,
			);
			this.startSlant = Math.atan2(-this.down.x, this.down.y);
		}
		this.slantTo = this.startSlant;
	}

	/// The angle from the middle to that point, as the canvas counts angles.
	private static angleAt(
		middle: cool.SimplePoint,
		point: cool.SimplePoint,
	): number {
		return Math.atan2(point.y - middle.y, point.x - middle.x);
	}

	/*
		The slant in its own coordinates for the mouse having gone that far round, as the office's
		SdrDragShear works it out: the upright axis is scaled by the cosine of the angle, which
		turns it over between ninety and two hundred and seventy degrees, and then sheared by the
		angle, taken back by half a turn while upside down and held to eighty-nine degrees.
		Together the upright axis turns by the angle and keeps its length.
	*/
	public static slant(turned: number): cool.Matrix2D {
		let degrees = (turned * 180) / Math.PI;
		degrees = ((((degrees + 180) % 360) + 360) % 360) - 180;

		const negative = degrees < 0;
		const size = Math.abs(degrees);
		const upsideDown = size > 90;

		const scale = Math.cos((size * Math.PI) / 180);
		let shear = upsideDown ? size - 180 : size;
		if (shear > 89) shear = 89;
		if (negative) shear = -shear;

		// A point is moved across by how far it lies from the middle, the other way from it.
		return new cool.Matrix2D(
			1,
			0,
			-scale * Math.tan((shear * Math.PI) / 180),
			scale,
			0,
			0,
		);
	}

	/*
		Follows the mouse. While a modifier is held the slant snaps to the step it names, counted
		from nought; while the mouse is on a mark it is that mark's.
	*/
	public move(
		to: cool.SimplePoint,
		event?: { ctrlKey?: boolean; shiftKey?: boolean; altKey?: boolean },
	): void {
		let slant = SelectionInteraction.normalAngle(
			this.startSlant +
				ShearingInteraction.angleAt(this.middle, to) -
				this.from,
		);

		const step = EditSteps.turnSnap(event);
		if (step) {
			const stepInRadians = (step * Math.PI) / 180;
			slant = Math.round(slant / stepInRadians) * stepInRadians;
		}

		this.slantTo = SelectionInteraction.heldToTheMarks(slant, this.marks(), to);
		this.selection.redraw();
	}

	/// The slant the selection gets.
	public reading(): string {
		return SelectionInteraction.degreesOf(
			SelectionInteraction.normalAngle(this.slantTo),
		);
	}

	/// The mark of no slant, straight above the middle at a right angle to the upper side.
	public marks(): AngleMark[] {
		return [
			SelectionInteraction.markAt(
				this.middle,
				this.radius,
				this.upright,
				0,
				'0',
			),
		];
	}

	public finish(
		to: cool.SimplePoint,
		event?: { ctrlKey?: boolean; shiftKey?: boolean; altKey?: boolean },
	): void {
		this.move(to, event);

		const matrix = this.transformation();
		if (matrix)
			SelectionSection.sendTransform(
				this.selection.selectedObjects(),
				matrix,
				'shear',
			);
	}

	/*
		The change on the page: out of the page into the slant's coordinates, the change there,
		and back. In the slant's coordinates the upper side stays, and the axis down from it goes
		from where it stands to where the slant puts it, keeping its length. Nothing where the
		slant is the one there was.
	*/
	public transformation(): cool.Matrix2D | null {
		const down = this.down;
		if (!down || !down.y) return null;
		if (
			Math.abs(
				SelectionInteraction.normalAngle(this.slantTo - this.startSlant),
			) < 1e-6
		)
			return null;

		const back = this.frame.invert();
		if (!back) return null;

		const length = Math.hypot(down.x, down.y);
		const slant = ShearingInteraction.slant(this.slantTo);
		const wantedAcross = slant.c * length;
		const wantedDown = slant.d * length;
		const change = new cool.Matrix2D(
			1,
			0,
			(wantedAcross - down.x) / down.y,
			wantedDown / down.y,
			0,
			0,
		);

		return back.then(change).then(this.frame);
	}

	public handles(known: SelectionHandle[]): SelectionHandle[] {
		const matrix = this.transformation();
		if (!matrix) return known;

		// The axes change with the selection, so the way each of the eight faces is worked out
		// again from where they stand now.
		return ObjectHandles.facedAwayFromTheirPlaces(
			known.map((handle: SelectionHandle) => ({
				...handle,
				point: matrix.apply(handle.point.x, handle.point.y).round(),
			})),
		);
	}
}
