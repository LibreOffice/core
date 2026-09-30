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
}
