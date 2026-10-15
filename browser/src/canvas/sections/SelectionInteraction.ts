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
	A mark beyond the handle that turns or shears the selection: a triangle at an angle the handle
	can be taken to, pointing to the middle, which the handle snaps to while the mouse is on it.
	It stands at nought and gives no turn or no slant in one movement.
*/
interface AngleMark {
	/// The angle the handle snaps to while the mouse is on the mark, as the interaction counts
	/// it.
	angle: number;
	/// The three corners of the triangle, in twips: its tip first, the one nearest the middle.
	corners: cool.Point[];
	/// What is written in it.
	label: string;
}

/*
	One thing a person does to a selection between taking hold of it and letting go: scaling it
	by a handle, turning it, moving it, moving a point of a path. It begins when it is made,
	follows the mouse while it moves, and either finishes, which tells the engine what it did,
	or is called off, which tells it nothing.

	While it runs it says two things about itself: the transformation it stands for, which draws
	the objects as they would look, and where the handles stand, which draws them along with it.
	Both are worked out in the client, so nothing is asked of the engine until the end.
*/
abstract class SelectionInteraction {
	/// What is selected: it knows the objects, their handles and the middle of them.
	protected selection: SelectionSection;

	constructor(selection: SelectionSection) {
		this.selection = selection;
	}

	/*
		The vector from the place the press landed on to the place of the handle it took hold of,
		in twips. A drag puts what the handle names where the mouse is, so a press a little beside
		the middle of a handle would move it by that much on the very first step. Adding the
		vector back gives the place the handle itself stands at, so a drag sets off from there and
		follows the mouse from there on. It is the same from the first step to the last and
		carries no sign to be turned over, so a mirror, a kept ratio and the snap all need to know
		nothing of it.
	*/
	private fromThePressToTheHandle: cool.Point = new cool.Point(0, 0);

	/// Takes in where inside its handle the press landed.
	public tookHoldAt(press: cool.SimplePoint, handle: SelectionHandle): void {
		this.fromThePressToTheHandle = new cool.Point(
			handle.point.x - press.x,
			handle.point.y - press.y,
		);
	}

	/// The place the handle is led to for a mouse at that point, in twips.
	protected led(to: cool.SimplePoint): cool.SimplePoint {
		return new cool.SimplePoint(
			to.x + this.fromThePressToTheHandle.x,
			to.y + this.fromThePressToTheHandle.y,
		);
	}

	/// Follows the mouse to that point in the document.
	public abstract move(to: cool.SimplePoint, event?: MouseEvent): void;

	/// Ends it where the mouse is, and hands the engine what it did.
	public abstract finish(to: cool.SimplePoint, event?: MouseEvent): void;

	/// Ends it with nothing done, so the selection stands as it stood.
	// eslint-disable-next-line @typescript-eslint/no-empty-function
	public cancel(): void {}

	/*
		The transformation it stands for now, laid over the objects as they are drawn. Nothing while
		it stands for none, and then the objects are drawn where they are.
	*/
	public transformation(): cool.Matrix2D | null {
		return null;
	}

	/*
		The transformation the objects are drawn through while it runs, as a picture of what it
		would do. The same as the transformation it stands for, unless what it does is not a
		change of the objects' drawing - a crop moves the window, not the picture - and then
		nothing, so that the objects are drawn as they are.
	*/
	public ghostTransformation(): cool.Matrix2D | null {
		return this.transformation();
	}

	/// Whether it crops rather than scales: the window onto a picture moves, the picture stays.
	public isACrop(): boolean {
		return false;
	}

	/// Where the handles stand while it runs, from where they stand when nothing runs.
	public handles(known: SelectionHandle[]): SelectionHandle[] {
		return known;
	}

	/*
		Whether the view scrolls on by itself while the mouse stands near its edge. Carrying the
		objects does: the mouse leads them and the page has to come along. A drag that is led by
		a handle does not, because the handle follows the mouse on the page, and a page that
		moves under a mouse standing still would run away with it.
	*/
	public scrollsWithTheMouse(): boolean {
		return false;
	}

	/// The handle it has hold of, for the ones that are led by a handle. Nothing for the others.
	public handleHeld(): SelectionHandle | null {
		return null;
	}

	/// The point it is being led by, in twips, which the view keeps in sight. Nothing where it
	/// is led by no single point.
	public leadingPoint(): cool.SimplePoint | null {
		return null;
	}

	/*
		The path the objects would be drawn along, where what the interaction does is not a
		transformation of them and so cannot be shown by drawing them through one. It is drawn
		as a thin line while the interaction runs. Nothing where there is nothing to show that
		way.
	*/
	public outline(): cool.ObjectPathPolygon[] | null {
		return null;
	}

	/// The marks drawn while the interaction runs. None for most of them.
	public marks(): AngleMark[] {
		return [];
	}

	/// What is written beside the mouse while the interaction runs, such as the angle a turn has
	/// reached. Nothing for most of them.
	public reading(): string | null {
		return null;
	}

	/// An angle in radians as it is written, in whole degrees.
	protected static degreesOf(angle: number): string {
		return String(Math.round((angle * 180) / Math.PI)) + '\u00B0';
	}

	/// An angle brought into the turn from minus half a turn to half a turn, in radians.
	protected static normalAngle(angle: number): number {
		const full = Math.PI * 2;
		const inside = (((angle + Math.PI) % full) + full) % full;
		return inside - Math.PI;
	}

	/// The angle of the first mark the mouse is on, or the angle given where it is on none.
	protected static heldToTheMarks(
		angle: number,
		marks: AngleMark[],
		mouse: cool.SimplePoint,
	): number {
		for (const mark of marks)
			if (SelectionInteraction.inside(mark.corners, mouse)) return mark.angle;
		return angle;
	}

	/// Whether the point, in twips, lies on that mark.
	public static isOnTheMark(mark: AngleMark, point: cool.SimplePoint): boolean {
		return SelectionInteraction.inside(mark.corners, point);
	}

	/// Whether the point lies inside the triangle with those corners.
	private static inside(
		corners: cool.Point[],
		point: cool.SimplePoint,
	): boolean {
		const sides = corners.map((corner: cool.Point, at: number) => {
			const next = corners[(at + 1) % corners.length];
			return (
				(next.x - corner.x) * (point.y - corner.y) -
				(next.y - corner.y) * (point.x - corner.x)
			);
		});
		return sides.every((side) => side >= 0) || sides.every((side) => side <= 0);
	}

	/*
		The mark for that angle, at that angle from the middle: twice the size of a handle, with its
		tip as far beyond the circle the handle runs on as the handle stands beyond the selection,
		and what it stands for written inside it. The radius is the handle's distance from the
		middle, in twips.
	*/
	protected static markAt(
		middle: cool.SimplePoint,
		radius: number,
		at: number,
		angle: number,
		label: string,
	): AngleMark {
		const twips = app.pixelsToTwips;
		const size = 2 * ShapeHandlesSection.handleSize() * twips;
		const beyond = radius + TurningHandle.distance * app.dpiScale * twips;

		const out = new cool.Point(Math.cos(at), Math.sin(at));
		const across = new cool.Point(-out.y, out.x);
		const tip = new cool.Point(
			middle.x + out.x * beyond,
			middle.y + out.y * beyond,
		);
		const base = tip.add(out.multiplyBy(size));

		return {
			angle: angle,
			corners: [
				tip,
				base.add(across.multiplyBy(size * 0.5)),
				base.subtract(across.multiplyBy(size * 0.5)),
			],
			label: label,
		};
	}
}
