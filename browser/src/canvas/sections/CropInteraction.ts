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
	Cropping a picture by one of the eight marks on its edges. On the picture's square it is the
	very drag a scale is, and it is worked out the same way; what differs is what it does to the
	picture. A scale moves the picture with its window; a crop moves the window alone and the
	picture stays where it is, so nothing of the picture is drawn through the change while it
	runs, only the handles go with it, and the whole graphic shown faintly round the picture says
	what the window is being moved over. The engine is told at the end the window the drag left,
	and works the crop out itself, in the graphic's own units.
*/
class CropInteraction extends ScalingInteraction {
	/// Which of the eight the mark stands for.
	private readonly ofTheEight: string;

	constructor(
		selection: SelectionSection,
		handle: SelectionHandle,
		shapeAtStart: any,
	) {
		// The maths of the drag knows the eight by their kind, and a mark is one of the eight
		// with the word in front, so the base is handed the mark as that one.
		super(
			selection,
			{ ...handle, kind: handle.kind.substring(4) },
			shapeAtStart,
		);

		this.ofTheEight = handle.kind.substring(4);
		this.keepsItsFacing = true;
	}

	public isACrop(): boolean {
		return true;
	}

	/// A crop takes each axis on its own, as the engine crops. Only Shift ties the two.
	protected ratioKept(event: MouseEvent): boolean {
		return event?.shiftKey === true;
	}

	/// Whether Alt is held, which lets a mark go as close to the graphic's edge as it likes.
	private altHeld: boolean = false;

	public move(to: cool.SimplePoint, event: MouseEvent): void {
		this.altHeld = event?.altKey === true;
		super.move(to, event);
	}

	/*
		Where the mark is led to: where the drag takes it, unless that is within a handle's width
		of the edge of the whole graphic on the side the mark is on, and then onto that edge - a
		crop is hard to take back to nothing by hand otherwise. Alt lets the mark go where it is
		taken, as Alt turns every snap over.

		A step from the keyboard is pulled only onto an edge that lies ahead of the mark in the
		way the step goes. A step that sets off from the edge has none ahead on that side, so it
		leaves the edge, and the steps after it go on away from it.
	*/
	protected led(to: cool.SimplePoint): cool.SimplePoint {
		const place = super.led(to);
		if (this.altHeld) return place;

		const square = this.selection.squareNow();
		const objectId = this.selection.selectedObjects()[0];
		const picture =
			objectId === undefined
				? undefined
				: RenderGeometrySection.objectOf(objectId);
		const extent = picture ? CropHandles.graphicExtentOf(picture) : undefined;
		const mine = ObjectHandles.placeOfKind(this.ofTheEight);
		const onTheSquare = square?.invert();
		if (!square || !onTheSquare || !extent || !mine) return place;

		const asked = onTheSquare.apply(place.x, place.y);
		const from = onTheSquare.apply(this.handle.point.x, this.handle.point.y);

		// A handle's width, as a share of the square along that axis.
		const near = (axis: number): number =>
			HandleLook.widthOf('framing') /
			((axis === 0 ? square.lengthOfXAxis() : square.lengthOfYAxis()) *
				app.twipsToPixels);

		/*
			Whether the mark is pulled onto that edge along that axis: it is within reach, and for
			a key step it lies ahead of where the step set off, more than a hair away from it.
		*/
		const pulled = (axis: number, edge: number, at: number): boolean => {
			if (Math.abs(at - edge) >= near(axis)) return false;
			if (!this.exact) return true;

			const start = axis === 0 ? from.x : from.y;
			const ahead = edge - start;
			return Math.abs(ahead) > near(axis) * 0.01 && ahead * (at - start) > 0;
		};

		/*
			A side has an edge to be pulled to where its crop is not nought: the graphic's edge lies
			outside the window where the side is cropped, and inside it where the window reaches
			beyond the graphic. Where the crop is nought the graphic's edge is the window's, and a
			pull there would take every small first crop straight back to nothing.
		*/
		const apart = (edge: number, window: number, axis: number): boolean =>
			Math.abs(edge - window) > near(axis) * 0.01;
		let x = asked.x;
		let y = asked.y;
		if (mine.x === 0 && apart(extent.minX, 0, 0) && pulled(0, extent.minX, x))
			x = extent.minX;
		if (mine.x === 1 && apart(extent.maxX, 1, 0) && pulled(0, extent.maxX, x))
			x = extent.maxX;
		if (mine.y === 0 && apart(extent.minY, 0, 1) && pulled(1, extent.minY, y))
			y = extent.minY;
		if (mine.y === 1 && apart(extent.maxY, 1, 1) && pulled(1, extent.maxY, y))
			y = extent.maxY;
		if (x === asked.x && y === asked.y) return place;

		const back = square.apply(x, y);
		return new cool.SimplePoint(back.x, back.y);
	}

	public ghostTransformation(): cool.Matrix2D | null {
		return null;
	}

	/// The engine is told the window the drag left, as the square the drag took the picture's
	/// square to.
	public finish(to: cool.SimplePoint, event: MouseEvent): void {
		this.move(to, event);

		const matrix = this.transformation();
		const square = this.selection.squareNow();
		const objectId = this.selection.selectedObjects()[0];
		if (!matrix || !square || objectId === undefined) return;

		CropHandles.cropTo(objectId, square.then(matrix));
	}
}
