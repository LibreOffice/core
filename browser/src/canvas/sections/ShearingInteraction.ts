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
			A single object is sheared from the slant it has, read from its own mapping, and so are
			several objects, or a group's members, that all have the same turn and slant, read
			from the first of them. Otherwise several objects, or a group, whose own mapping is a
			plain rectangle, are sheared as the selection's square, which starts upright and
			without a slant and carries what was done to it since, as a turn does. The frame lies along the upper side of what is read,
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
			: (selection.mappingTheyShare() ?? selection.squareNow());

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

	/*
		The slant keys give, kept between one key and the next: the slant as one interaction from
		the selection as it was before the first key, the mapping each object that draws had
		then, the change that has been laid on since, and which objects it was. It goes on while
		nothing else changes those objects, so the keys count the angle on as a drag does and
		each result is worked out from the start. Any other change sent from here, or one the
		engine makes on its own such as an undo, ends it, and the next key starts again from what
		it finds.
	*/
	private static keys: {
		shearing: ShearingInteraction;
		mappings: Map<number, cool.Matrix2D>;
		objects: string;
		laid: cool.Matrix2D;
	} | null = null;

	/// The slant keys gave the selection where they were the last to change it.
	private static keysStillHold(
		selection: SelectionSection,
	): ShearingInteraction | null {
		const keys = ShearingInteraction.keys;
		if (!keys || keys.objects !== selection.selectedObjects().join(','))
			return null;
		return keys.shearing;
	}

	/// Ends the slant keys gave, so the next key starts from the objects as they stand. Given an
	/// object, only where the keys changed that one.
	public static forgetTheKeys(objectId?: number): void {
		const keys = ShearingInteraction.keys;
		if (objectId !== undefined && keys && !keys.mappings.has(objectId)) return;
		ShearingInteraction.keys = null;
	}

	/*
		Slants the selection around that middle, in the frame turned by that angle, to the slant
		worked out from the one there is, both in degrees as the canvas counts angles. Where keys
		gave the slant there is, it goes on from their start. Sends the change and gives it back,
		or nothing where the slant stays.
	*/
	public static shearObjects(
		selection: SelectionSection,
		middle: cool.SimplePoint,
		turnedBy: number,
		slantFrom: (stands: number) => number,
	): cool.Matrix2D | null {
		let shearing = ShearingInteraction.keysStillHold(selection);
		if (!shearing || !ShearingInteraction.keys) {
			const objects = selection.selectedObjects();
			const first = objects.length
				? RenderGeometrySection.transformOf(objects[0])
				: null;
			if (!first) return null;

			shearing = new ShearingInteraction(selection, middle, middle, turnedBy);
			const mappings = new Map<number, cool.Matrix2D>();
			for (const objectId of RenderGeometrySection.theOnesThatDraw(
				RenderGeometrySection.standingFor(objects),
			)) {
				const mapping = RenderGeometrySection.transformOf(objectId);
				if (mapping) mappings.set(objectId, mapping);
			}
			ShearingInteraction.keys = {
				shearing: shearing,
				mappings: mappings,
				objects: objects.join(','),
				laid: cool.Matrix2D.IDENTITY,
			};
		}
		const keys = ShearingInteraction.keys;

		const stands = (shearing.slantTo * 180) / Math.PI;
		shearing.slantTo = SelectionInteraction.normalAngle(
			(slantFrom(stands) * Math.PI) / 180,
		);

		// What is laid on now takes the objects from where the keys left them to the new slant.
		const wanted = shearing.transformation() ?? cool.Matrix2D.IDENTITY;
		const back = keys.laid.invert();
		if (!back) return null;
		const change = back.then(wanted);
		if (change.equals(cool.Matrix2D.IDENTITY, 1e-9)) return null;

		const objects = selection.selectedObjects();
		// The engine is told the slant worked out from the start, since it keeps sizes rounded
		// and a slant opening up a thin line again would make that rounding grow.
		SelectionSection.sendTransform(objects, wanted, 'shear', keys.mappings);
		keys.laid = wanted;
		return change;
	}

	/*
		The slant the selection has, in the frame turned by that angle, as the canvas counts
		angles: the one keys gave it where they were the last to change it, or the one read from
		the first object. Near ninety degrees the object's axis says little about the slant, since
		the shear stays at eighty-nine there.
	*/
	public static slantOf(selection: SelectionSection, turnedBy: number): number {
		const keys = ShearingInteraction.keysStillHold(selection);
		if (keys) return keys.slantTo;

		const nowhere = new cool.SimplePoint(0, 0);
		return new ShearingInteraction(selection, nowhere, nowhere, turnedBy)
			.startSlant;
	}

	/// A slant as it is written, in whole degrees from minus half a turn to half a turn.
	public static readingOf(slant: number): string {
		return SelectionInteraction.degreesOf(
			SelectionInteraction.normalAngle(slant),
		);
	}

	/// An angle in degrees brought into the turn from minus half a turn to half a turn.
	private static degreesInTheTurn(degrees: number): number {
		return ((((degrees + 180) % 360) + 360) % 360) - 180;
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
		Together the upright axis turns by the angle and keeps its length, except within a degree
		of ninety either way, where the held shear makes it shorter.
	*/
	public static slant(turned: number): cool.Matrix2D {
		const degrees = ShearingInteraction.degreesInTheTurn(
			(turned * 180) / Math.PI,
		);

		const negative = degrees < 0;
		const size = Math.abs(degrees);
		const upsideDown = size > 90;

		// Up to ninety degrees the shear stays at eighty-nine and the cosine thins the selection
		// out. At ninety it keeps a hundredth of its height, so it stands as a thin line that a
		// further slant opens up again.
		const cos = Math.cos((size * Math.PI) / 180);
		const scale = upsideDown ? cos : Math.max(cos, 0.01);
		// An object holds a shear of eighty-nine degrees at most either way, so the shear is
		// held there on both sides of ninety, upside down as well.
		let shear = upsideDown ? size - 180 : size;
		shear = Math.max(-89, Math.min(89, shear));
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
		return ShearingInteraction.readingOf(this.slantTo);
	}

	/// The mark of no slant, straight above the middle at a right angle to the upper side.
	public marks(): AngleMark[] {
		return [
			SelectionInteraction.markAt(
				this.middle,
				this.radius,
				this.upright,
				0,
				'0\u00B0',
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
