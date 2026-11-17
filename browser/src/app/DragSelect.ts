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
	A selection drawn by a drag on the empty page of a view that draws from objects: it follows the
	mouse while the button is down, is drawn over the page, and selects what it encloses when the
	button comes up. Which kind of it a press begins is the kind's own business; the mouse control
	only hands it the drag.
*/
interface DragSelect {
	/// Takes the drag on to that place of the page, in twips.
	dragTo(at: cool.Point): void;

	/// The selection it makes in the context one stands in, or nothing where that context makes
	/// none of it.
	selects(): number[] | undefined;

	/// Draws it over the page, the canvas being moved by those amounts, in canvas pixels, from
	/// the start of the document.
	draw(context: CanvasRenderingContext2D, xDiff: number, yDiff: number): void;
}

/*
	The selection frame: a rectangle from where the press was to the mouse, which selects the
	objects of the level one stands at that lie wholly inside it. With Shift it changes the state
	in the selection of those objects instead, adding or taking away each one.
*/
class FrameSelect implements DragSelect {
	/// The corner the press was at and the one the mouse is at, in twips.
	private from: cool.Point;
	private to: cool.Point;
	/// Whether the frame changes the selection there is rather than making a new one.
	private extending: boolean;

	private constructor(press: cool.Point, extending: boolean) {
		this.from = press;
		this.to = press;
		this.extending = extending;
	}

	/*
		The frame a drag that began with that press, in twips, begins, where it begins one: in a
		view that draws slides from objects, while no text is edited, for a press with no modifier
		but Shift that met no object, where the place one stands at draws frames. Null otherwise.
	*/
	public static begins(
		press: cool.Point,
		modifier: number,
	): FrameSelect | null {
		if (!RenderGeometrySection.answersPointer()) return null;
		if (RenderManager.kind() !== 'slides') return null;
		if (app.file.textCursor.visible) return null;
		if (modifier !== 0 && modifier !== app.UNOModifier.SHIFT) return null;
		if (RenderGeometrySection.objectIdAt(press.x, press.y) !== undefined)
			return null;
		const nowhere = new cool.Range2D(0, 0, 0, 0);
		if (Interaction.context().frameSelects(nowhere, false) === undefined)
			return null;
		return new FrameSelect(press, modifier === app.UNOModifier.SHIFT);
	}

	public dragTo(at: cool.Point): void {
		this.to = at;
	}

	/// The frame as it stands, in twips.
	private frame(): cool.Range2D {
		return cool.Range2D.fromPoints([this.from, this.to]);
	}

	public selects(): number[] | undefined {
		return Interaction.context().frameSelects(this.frame(), this.extending);
	}

	/// Dashed in the colour of the outline under the pointer, on whole pixels.
	public draw(
		context: CanvasRenderingContext2D,
		xDiff: number,
		yDiff: number,
	): void {
		const frame = this.frame();
		const scale = app.twipsToPixels;
		context.save();
		context.strokeStyle = RenderGeometrySection.hoverColor();
		context.lineWidth = 1;
		context.setLineDash(RenderGeometrySection.hoverDashes);
		context.strokeRect(
			xDiff + Math.round(frame.minX * scale) + 0.5,
			yDiff + Math.round(frame.minY * scale) + 0.5,
			Math.round(frame.width * scale),
			Math.round(frame.height * scale),
		);
		context.restore();
	}
}
