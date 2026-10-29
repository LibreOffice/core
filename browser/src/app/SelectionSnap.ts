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
	What a dragged thing can line up with: the edges and the middles of the other objects of the
	page, or the points of the grid where the document is drawn on one. It is asked on every step
	of a drag where the thing now stands, it keeps what it found, and it draws the lines that
	show it. At the end of the drag it says where the thing really lands.

	It knows nothing of who is dragging. The section that draws a selection from tiles keeps one
	and so does the one that draws it from objects, and both ask it the same way.
*/
class SelectionSnap {
	/// Where the drag lines up, in core pixels, null on an axis that found nothing.
	private closestX: number | null = null;
	private closestY: number | null = null;

	/// The middle of the object that was lined up with, marked with a dot, null where the line
	/// is not a middle.
	private centerSnapX: number[] | null = null;
	private centerSnapY: number[] | null = null;

	/// Whether it was the middle of the dragged thing itself that met that middle.
	private centerToCenterX: boolean = false;
	private centerToCenterY: boolean = false;

	/// The middle of the dragged thing, marked as well when the two middles met.
	private draggedCenter: number[] | null = null;

	/// How far the point that lined up lies from the upper left corner of the dragged thing.
	private snapOffsetX: number = 0;
	private snapOffsetY: number = 0;

	/// Where the drag lines up, in core pixels, for whoever asks the point itself.
	public at(): (number | null)[] {
		return [this.closestX, this.closestY];
	}

	/// Whether anything was found to line up with.
	public found(): boolean {
		return this.closestX !== null || this.closestY !== null;
	}

	/// Forgets what was found, so the lines that marked it are gone.
	public forget(): void {
		this.closestX = null;
		this.closestY = null;
		this.centerSnapX = null;
		this.centerSnapY = null;
		this.centerToCenterX = false;
		this.centerToCenterY = false;
		this.draggedCenter = null;
	}

	/*
		Where the upper left corner of the dragged thing lands, in core pixels: where it lined up,
		or what is given on an axis that found nothing. What lined up is a point of the thing, its
		middle or an edge, so the distance from that point to the corner comes off again.
	*/
	public corner(x: number, y: number): number[] {
		return [
			this.closestX !== null ? this.closestX - this.snapOffsetX : x,
			this.closestY !== null ? this.closestY - this.snapOffsetY : y,
		];
	}

	/// Draws what was found into the section that asks, the lines of the grid or the lines that
	/// line objects up.
	public draw(section: CanvasSectionObject): void {
		if (!this.found()) return;

		if (app.map.stateChangeHandler.getItemValue('.uno:GridUse') === 'true')
			this.drawGridHelperLines(section);
		else this.drawShapeAlignmentHelperLines(section);
	}

	/*
		Looks for what the dragged thing could line up with, from where the drag has taken it. The
		size and the position are of the thing being dragged, in core pixels, and the distance is
		how far the drag has carried it.

		Where the document is drawn on a grid, a point of the grid is looked for and nothing else:
		the engine knows nothing of the lines that line objects up and would ignore them anyway
		once it can snap to the grid.
	*/
	public look(
		size: number[],
		position: number[],
		dragDistance: number[],
		snapKeyHeld: boolean = false,
	): void {
		this.forget();

		// inside an object active in place nothing snaps, as in the chart's own drag
		if (
			Interaction.placeOf(GraphicSelection.selectedObjectIDs).kind === 'inPlace'
		)
			return;

		// Alt turns the snap over against how it is set, for every kind of snap and for every
		// action that snaps, so a grid that is on is left alone while Alt is held and one that is
		// off is snapped to.
		const gridOn =
			app.map.stateChangeHandler.getItemValue('.uno:GridUse') === 'true';
		if (gridOn !== snapKeyHeld) {
			this.findClosestGridPoint(size, position, dragDistance);
			return;
		}

		const left = position[0] + dragDistance[0];
		const top = position[1] + dragDistance[1];
		this.checkObjectsBoundaries(
			[left, left + size[0] / 2, left + size[0]],
			[top, top + size[1] / 2, top + size[1]],
		);
	}

	private findClosestX(xList: number[]) {
		let closest = 1000;
		let pickX = null;
		let snapOffset = 0;
		let centerSnap = null;
		let centerToCenter = false;
		{
			const rectangles = GraphicSelection.snapRectangles();

			for (let i = 0; i < rectangles.length; i++) {
				// Candidate snap ordinates of the other object: left edge, center, right edge.
				const targets = [
					rectangles[i][0],
					rectangles[i][0] + rectangles[i][2] / 2,
					rectangles[i][0] + rectangles[i][2],
				];

				for (let j = 0; j < xList.length; j++) {
					for (let k = 0; k < targets.length; k++) {
						const distance = Math.abs(targets[k] - xList[j]);
						if (distance < closest) {
							closest = distance;
							pickX = targets[k];
							snapOffset = xList[j] - xList[0]; // 0, half width or full width.
							// k === 1 means the other object's center was the snap target.
							centerSnap =
								k === 1
									? [targets[1], rectangles[i][1] + rectangles[i][3] / 2]
									: null;
							// Both centers meet when the matched point is the active object's center too.
							centerToCenter = k === 1 && xList[j] === xList[1];
						}
					}
				}
			}
		}

		if (closest < 10 * app.dpiScale) {
			this.closestX = pickX;
			this.snapOffsetX = snapOffset;
			this.centerSnapX = centerSnap;
			this.centerToCenterX = centerToCenter;
		} else {
			this.closestX = null;
			this.centerSnapX = null;
			this.centerToCenterX = false;
		}
	}

	private findClosestY(yList: number[]) {
		let closest = 1000;
		let pickY = null;
		let snapOffset = 0;
		let centerSnap = null;
		let centerToCenter = false;
		{
			const rectangles = GraphicSelection.snapRectangles();

			for (let i = 0; i < rectangles.length; i++) {
				// Candidate snap ordinates of the other object: top edge, center, bottom edge.
				const targets = [
					rectangles[i][1],
					rectangles[i][1] + rectangles[i][3] / 2,
					rectangles[i][1] + rectangles[i][3],
				];

				for (let j = 0; j < yList.length; j++) {
					for (let k = 0; k < targets.length; k++) {
						const distance = Math.abs(targets[k] - yList[j]);
						if (distance < closest) {
							closest = distance;
							pickY = targets[k];
							snapOffset = yList[j] - yList[0]; // 0, half height or full height.
							// k === 1 means the other object's center was the snap target.
							centerSnap =
								k === 1
									? [rectangles[i][0] + rectangles[i][2] / 2, targets[1]]
									: null;
							// Both centers meet when the matched point is the active object's center too.
							centerToCenter = k === 1 && yList[j] === yList[1];
						}
					}
				}
			}
		}

		if (closest < 10 * app.dpiScale) {
			this.closestY = pickY;
			this.snapOffsetY = snapOffset;
			this.centerSnapY = centerSnap;
			this.centerToCenterY = centerToCenter;
		} else {
			this.closestY = null;
			this.centerSnapY = null;
			this.centerToCenterY = false;
		}
	}

	private cloneSelectedPartInfoForGridSnap() {
		const selectedPart = Object.assign(
			{},
			app.impress.partList[app.map._docLayer._selectedPart],
		);
		selectedPart.leftBorder *= app.impress.twipsCorrection;
		selectedPart.upperBorder *= app.impress.twipsCorrection;
		selectedPart.rightBorder *= app.impress.twipsCorrection;
		selectedPart.lowerBorder *= app.impress.twipsCorrection;
		selectedPart.gridCoarseWidth *= app.impress.twipsCorrection;
		selectedPart.gridCoarseHeight *= app.impress.twipsCorrection;

		return selectedPart;
	}
	private getInnerRecrangleForGridSnap(selectedPart: any) {
		return new cool.SimpleRectangle(
			selectedPart.leftBorder,
			selectedPart.upperBorder,
			selectedPart.width - selectedPart.leftBorder - selectedPart.rightBorder,
			selectedPart.height - selectedPart.upperBorder - selectedPart.lowerBorder,
		);
	}
	private getCornerPointsForGridSnap(
		size: number[],
		position: number[],
		dragDistance: number[],
	) {
		return [
			new cool.SimplePoint(
				(position[0] + dragDistance[0]) * app.pixelsToTwips,
				(position[1] + dragDistance[1]) * app.pixelsToTwips,
			),
			new cool.SimplePoint(
				(size[0] + position[0] + dragDistance[0]) * app.pixelsToTwips,
				(position[1] + dragDistance[1]) * app.pixelsToTwips,
			),
			new cool.SimplePoint(
				(position[0] + dragDistance[0]) * app.pixelsToTwips,
				(size[1] + position[1] + dragDistance[1]) * app.pixelsToTwips,
			),
			new cool.SimplePoint(
				(size[0] + position[0] + dragDistance[0]) * app.pixelsToTwips,
				(size[1] + position[1] + dragDistance[1]) * app.pixelsToTwips,
			),
		];
	}

	private findClosestGridPoint(
		size: number[],
		position: number[],
		dragDistance: number[],
	) {
		// First rule of snap-to-grid: If you enable snap-to-grid, you have to snap.

		const selectedPart = this.cloneSelectedPartInfoForGridSnap();

		// The 4 corners of selected object's rectangle.
		const checkList = this.getCornerPointsForGridSnap(
			size,
			position,
			dragDistance,
		);

		// The rectangle that is shaped by the page margins.
		const innerRectangle = this.getInnerRecrangleForGridSnap(selectedPart);

		const gapX =
			selectedPart.gridCoarseWidth /
			(selectedPart.innerSpacesX > 0 ? selectedPart.innerSpacesX : 1);
		const gapY =
			selectedPart.gridCoarseHeight /
			(selectedPart.innerSpacesY > 0 ? selectedPart.innerSpacesY : 1);

		let minX = 100000;
		let minY = 100000;
		// The grid begins at the corner of the page's margins and goes on in every direction
		// without end, so a corner beyond the page snaps to it as one on the page does.
		for (let i = 0; i < 1; i++) {
			const countX = Math.round((checkList[i].x - innerRectangle.x1) / gapX);
			const countY = Math.round((checkList[i].y - innerRectangle.y1) / gapY);

			const diffX = Math.abs(
				checkList[i].x - innerRectangle.x1 - gapX * countX,
			);
			const diffY = Math.abs(
				checkList[i].y - innerRectangle.y1 - gapY * countY,
			);

			if (diffX < minX) {
				minX = diffX;
				this.closestX = innerRectangle.x1 + countX * gapX;
				this.snapOffsetX = [1, 3].includes(i) ? size[0] : 0; // Subtract width when a right corner snapped.
			}
			if (diffY < minY) {
				minY = diffY;
				this.closestY = innerRectangle.y1 + countY * gapY;
				this.snapOffsetY = [2, 3].includes(i) ? size[1] : 0; // Subtract height when a bottom corner snapped.
			}
		}

		// The grid is counted in twips, the rest of this in pixels.
		if (this.closestX !== null) this.closestX *= app.twipsToPixels;
		if (this.closestY !== null) this.closestY *= app.twipsToPixels;
	}

	private checkObjectsBoundaries(
		xListToCheck: number[],
		yListToCheck: number[],
	) {
		if (app.map._docLayer._docType === 'presentation') {
			this.findClosestX(xListToCheck);
			this.findClosestY(yListToCheck);

			// On a center-to-center match, also mark the active object's own center.
			// The object is not snapped until mouse up, so on a matched axis use the
			// snapped ordinate (closestX / closestY) to place the dot on the helper
			// line instead of the still-offset live position (xList[1] / yList[1]).
			if (this.centerToCenterX || this.centerToCenterY) {
				this.draggedCenter = [
					(this.centerToCenterX ? this.closestX : null) ?? xListToCheck[1],
					(this.centerToCenterY ? this.closestY : null) ?? yListToCheck[1],
				];
			} else this.draggedCenter = null;
		}
	}

	private drawGridHelperLines(section: CanvasSectionObject) {
		Util.ensureValue(app.activeDocument);

		section.context.save();

		section.context.translate(-section.myTopLeft[0], -section.myTopLeft[1]);

		section.context.beginPath();

		if (this.closestX !== null) {
			section.context.strokeStyle = HelperLineStyles.gridSolidStyle;
			section.context.setLineDash([]);

			const x =
				section.containerObject.getDocumentAnchor()[0] +
				this.closestX -
				app.activeDocument.activeLayout.viewedRectangle.pX1;

			this.drawXAxis(section, x);

			// Draw a second line on top of solid white-ish line.
			section.context.setLineDash([4, 3]);
			section.context.strokeStyle = HelperLineStyles.gridDashedStyle;

			this.drawXAxis(section, x);
		}

		if (this.closestY !== null) {
			section.context.strokeStyle = HelperLineStyles.gridSolidStyle;
			section.context.setLineDash([]);

			const y =
				section.containerObject.getDocumentAnchor()[1] +
				this.closestY -
				app.activeDocument.activeLayout.viewedRectangle.pY1;

			this.drawYAxis(section, y);

			// Draw a second line on top of solid white-ish line.
			section.context.setLineDash([4, 3]);
			section.context.strokeStyle = HelperLineStyles.gridDashedStyle;

			this.drawYAxis(section, y);
		}

		section.context.closePath();

		section.context.restore();
	}

	private drawShapeAlignmentHelperLines(section: CanvasSectionObject) {
		Util.ensureValue(app.activeDocument);

		section.context.save();

		section.context.setLineDash([4, 3]);
		section.context.strokeStyle = HelperLineStyles.smartGuidesStyle;
		section.context.translate(-section.myTopLeft[0], -section.myTopLeft[1]);

		section.context.beginPath();

		if (this.closestX !== null)
			this.drawXAxis(
				section,
				section.containerObject.getDocumentAnchor()[0] +
					this.closestX -
					app.activeDocument.activeLayout.viewedRectangle.pX1,
			);

		if (this.closestY !== null)
			this.drawYAxis(
				section,
				section.containerObject.getDocumentAnchor()[1] +
					this.closestY -
					app.activeDocument.activeLayout.viewedRectangle.pY1,
			);

		section.context.closePath();

		// When snapping to another object's center, mark that center with a red dot.
		if (this.centerSnapX !== null)
			this.drawCenterSnapDot(section, this.centerSnapX);

		if (this.centerSnapY !== null)
			this.drawCenterSnapDot(section, this.centerSnapY);

		// On a center-to-center match, also mark the active object's own center.
		if (this.draggedCenter !== null)
			this.drawCenterSnapDot(section, this.draggedCenter);

		section.context.restore();
	}

	private drawXAxis(section: CanvasSectionObject, x: number) {
		section.context.moveTo(x, 0);
		section.context.lineTo(x, section.context.canvas.height);
		section.context.stroke();
	}

	private drawYAxis(section: CanvasSectionObject, y: number) {
		section.context.moveTo(0, y);
		section.context.lineTo(section.context.canvas.width, y);
		section.context.stroke();
	}

	private drawCenterSnapDot(
		section: CanvasSectionObject,
		centerPoint: number[],
	) {
		Util.ensureValue(app.activeDocument);

		const x =
			section.containerObject.getDocumentAnchor()[0] +
			centerPoint[0] -
			app.activeDocument.activeLayout.viewedRectangle.pX1;
		const y =
			section.containerObject.getDocumentAnchor()[1] +
			centerPoint[1] -
			app.activeDocument.activeLayout.viewedRectangle.pY1;

		section.context.beginPath();
		section.context.fillStyle = 'red';
		section.context.arc(x, y, 3 * app.dpiScale, 0, 2 * Math.PI);
		section.context.fill();
		section.context.closePath();
	}
}
