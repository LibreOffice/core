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
	what the window is being moved over. The engine is told at the end which mark went where and
	works the crop out itself, in the graphic's own units.
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

	public ghostTransformation(): cool.Matrix2D | null {
		return null;
	}

	public finish(to: cool.SimplePoint, event: MouseEvent): void {
		this.move(to, event);
		if (!this.at) return;

		const objectId = this.selection.selectedObjects()[0];
		if (objectId === undefined) return;

		app.socket.sendMessage(
			'cropobject id=' +
				String(objectId) +
				' kind=' +
				String(Number(this.ofTheEight)) +
				' x=' +
				String(Math.round(this.at.x)) +
				' y=' +
				String(Math.round(this.at.y)),
		);
	}
}
