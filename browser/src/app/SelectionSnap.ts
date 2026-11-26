/* -*- js-indent-level: 8 -*- */
/*
 * Copyright the Collabora Office contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

/// A place along one axis where a dragged thing would stand as far from its neighbour as other
/// objects stand from theirs, in core pixels.
interface EqualSpacing {
	/// Where the near edge of the dragged thing lands.
	at: number;
	/// How wide the gaps are that come out equal.
	gap: number;
	/// How far that is from where the drag has taken the thing.
	distance: number;
}

/*
	What a dragged thing can line up with: the edges and the middles of the other objects of the
	page, a place where it stands as far from its neighbour as other objects in its row or column
	stand from theirs, or the points of the grid where the document is drawn on one. It is asked
	on every step of a drag where the thing now stands, it keeps what it found, and it draws the
	lines and the gaps that show it. At the end of the drag it says where the thing really lands.

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

	/// How far the edge or the middle that lined up was from where the drag left it.
	private distanceX: number = 0;
	private distanceY: number = 0;

	/// Whether the place found on an axis is one of equal spacing rather than an edge or a
	/// middle, which has no line across the page.
	private spacingX: boolean = false;
	private spacingY: boolean = false;

	/// The gaps that are equal once the dragged thing lands, each drawn as a line with an arrow at
	/// either end, from x1, y1 to x2, y2 in core pixels.
	private equalGaps: number[][] = [];

	/// Every edge of another object that an edge of the dragged thing lies on once it lands, in
	/// core pixels, one list per axis. Empty where the thing lined up only by its middle, or is a
	/// single point.
	private edgesX: number[] = [];
	private edgesY: number[] = [];

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
		this.spacingX = false;
		this.spacingY = false;
		this.equalGaps = [];
		this.edgesX = [];
		this.edgesY = [];
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

		// A point, as a handle or a new object's first corner is, has no gap to its neighbours.
		if (size[0] > 0 && size[1] > 0)
			this.checkEqualSpacing([left, top, size[0], size[1]]);

		if (size[0] > 0 && size[1] > 0) this.findMatchingEdges(size, [left, top]);
	}

	/*
		Collects the edges of other objects that the edges of the dragged thing lie on where it
		lands. Two objects of the same height that line up at the top line up at the bottom as
		well, so both lines are found. The size is that of the dragged thing and the place is its
		upper left corner as the drag leaves it, both in core pixels.
	*/
	private findMatchingEdges(size: number[], place: number[]): void {
		if (app.map._docLayer._docType !== 'presentation') return;

		const landed = this.corner(place[0], place[1]);
		const rectangles = GraphicSelection.snapRectangles();

		for (let axis = 0; axis < 2; axis++) {
			if (axis === 0 ? this.closestX === null : this.closestY === null)
				continue;

			const edges: number[] = [];
			const own = [landed[axis], landed[axis] + size[axis]];
			for (const other of rectangles) {
				for (const edge of [other[axis], other[axis] + other[axis + 2]]) {
					// The engine places objects in twips, so an edge can sit a fraction of a
					// pixel away from one it lies on.
					const lies = own.some((point: number) => Math.abs(point - edge) < 1);
					if (
						lies &&
						!edges.some((known: number) => Math.abs(known - edge) < 1)
					)
						edges.push(edge);
				}
			}

			if (axis === 0) this.edgesX = edges;
			else this.edgesY = edges;
		}
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
								k === 1 ?
									[targets[1], rectangles[i][1] + rectangles[i][3] / 2]
								:	null;
							// Both centers meet when the matched point is the active object's center too.
							centerToCenter = k === 1 && xList[j] === xList[1];
						}
					}
				}
			}
		}

		if (closest < VectorSettings.snapDistance() * app.dpiScale) {
			this.closestX = pickX;
			this.distanceX = closest;
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
								k === 1 ?
									[rectangles[i][0] + rectangles[i][2] / 2, targets[1]]
								:	null;
							// Both centers meet when the matched point is the active object's center too.
							centerToCenter = k === 1 && yList[j] === yList[1];
						}
					}
				}
			}
		}

		if (closest < VectorSettings.snapDistance() * app.dpiScale) {
			this.closestY = pickY;
			this.distanceY = closest;
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

	/*
		Looks along both axes for a place where the dragged thing stands as far from its neighbour
		as other objects in the same row or column stand from theirs, or exactly halfway between
		its two neighbours. The box is the dragged thing where the drag has taken it, left, top,
		width and height in core pixels. A place found here wins on its axis over an edge or a
		middle only when it is closer, and where both hold the gaps are marked as well.
	*/
	private checkEqualSpacing(box: number[]): void {
		if (app.map._docLayer._docType !== 'presentation') return;

		const rectangles = GraphicSelection.snapRectangles();
		const reach = VectorSettings.snapDistance() * app.dpiScale;
		const found: (EqualSpacing | null)[] = [
			SelectionSnap.findEqualSpacing(rectangles, box, 0, reach),
			SelectionSnap.findEqualSpacing(rectangles, box, 1, reach),
		];

		const landed = box.slice();

		if (found[0]) {
			const corner =
				this.closestX !== null ? this.closestX - this.snapOffsetX : null;
			if (this.closestX === null || found[0].distance < this.distanceX - 0.5) {
				this.closestX = found[0].at;
				this.snapOffsetX = 0;
				this.centerSnapX = null;
				this.centerToCenterX = false;
				this.spacingX = true;
			} else if (Math.abs((corner as number) - found[0].at) >= 0.5)
				found[0] = null;
		}
		if (found[1]) {
			const corner =
				this.closestY !== null ? this.closestY - this.snapOffsetY : null;
			if (this.closestY === null || found[1].distance < this.distanceY - 0.5) {
				this.closestY = found[1].at;
				this.snapOffsetY = 0;
				this.centerSnapY = null;
				this.centerToCenterY = false;
				this.spacingY = true;
			} else if (Math.abs((corner as number) - found[1].at) >= 0.5)
				found[1] = null;
		}

		if (this.closestX !== null) landed[0] = this.closestX - this.snapOffsetX;
		if (this.closestY !== null) landed[1] = this.closestY - this.snapOffsetY;

		// The dot on the dragged thing's own middle follows where it lands now.
		this.draggedCenter =
			this.centerToCenterX || this.centerToCenterY ?
				[landed[0] + box[2] / 2, landed[1] + box[3] / 2]
			:	null;

		for (let axis = 0; axis < 2; axis++) {
			const spacing = found[axis];
			if (spacing)
				this.equalGaps.push(
					...SelectionSnap.gapsOf(rectangles, landed, axis, spacing.gap),
				);
		}
	}

	/*
		The place along one axis, 0 across and 1 down, where the box stands as far from a
		neighbour as two other objects of its row stand from each other, or halfway between its
		two neighbours, whichever is closest to where the box is now. The row is every object that
		shares some of the box's extent on the other axis. Null when nothing lies within reach.
	*/
	private static findEqualSpacing(
		rectangles: number[][],
		box: number[],
		axis: number,
		reach: number,
	): EqualSpacing | null {
		const row = SelectionSnap.rowOf(rectangles, box, axis);
		const start = box[axis];
		const size = box[axis + 2];
		const middle = start + size / 2;

		// The nearest neighbour on either side, the one whose near edge is closest.
		let before: number[] | null = null;
		let after: number[] | null = null;
		for (const other of row) {
			if (other[axis] + other[axis + 2] < middle) {
				if (
					!before ||
					other[axis] + other[axis + 2] > before[axis] + before[axis + 2]
				)
					before = other;
			} else if (other[axis] > middle) {
				if (!after || other[axis] < after[axis]) after = other;
			}
		}
		if (!before && !after) return null;

		const candidates: EqualSpacing[] = [];
		const offer = (at: number, gap: number) => {
			if (gap > 0)
				candidates.push({ at: at, gap: gap, distance: Math.abs(at - start) });
		};

		for (const gap of SelectionSnap.gapsBetween(row, axis)) {
			if (before) offer(before[axis] + before[axis + 2] + gap, gap);
			if (after) offer(after[axis] - gap - size, gap);
		}

		if (before && after) {
			const free = after[axis] - (before[axis] + before[axis + 2]) - size;
			offer(before[axis] + before[axis + 2] + free / 2, free / 2);
		}

		let best: EqualSpacing | null = null;
		for (const candidate of candidates)
			if (
				candidate.distance < reach &&
				(!best || candidate.distance < best.distance)
			)
				best = candidate;

		return best;
	}

	/// The objects that share some of the box's extent on the axis other than the one given.
	private static rowOf(
		rectangles: number[][],
		box: number[],
		axis: number,
	): number[][] {
		const cross = 1 - axis;
		return rectangles.filter(
			(other: number[]) =>
				other[cross] < box[cross] + box[cross + 2] &&
				other[cross] + other[cross + 2] > box[cross],
		);
	}

	/// The gap from each object of the row to the next one along the axis that shares some of
	/// its extent across, where they do not overlap.
	private static gapsBetween(row: number[][], axis: number): number[] {
		return SelectionSnap.neighbourPairs(row, axis).map(
			(pair: number[][]) => pair[1][axis] - (pair[0][axis] + pair[0][axis + 2]),
		);
	}

	/// Each object of the row with the next one along the axis that it does not overlap and that
	/// shares some of its extent across.
	private static neighbourPairs(row: number[][], axis: number): number[][][] {
		const cross = 1 - axis;
		const pairs: number[][][] = [];

		for (const first of row) {
			const end = first[axis] + first[axis + 2];
			let next: number[] | null = null;
			for (const other of row) {
				if (other === first || other[axis] <= end) continue;
				if (
					other[cross] >= first[cross] + first[cross + 2] ||
					other[cross] + other[cross + 2] <= first[cross]
				)
					continue;
				if (!next || other[axis] < next[axis]) next = other;
			}
			if (next) pairs.push([first, next]);
		}

		return pairs;
	}

	/*
		Every gap of the given size along the axis once the box has landed: those between other
		objects of its row, and those from the box to its neighbours. Each is a line from one edge
		to the other, across the middle of what the two sides share.
	*/
	private static gapsOf(
		rectangles: number[][],
		box: number[],
		axis: number,
		gap: number,
	): number[][] {
		const row = SelectionSnap.rowOf(rectangles, box, axis);
		row.push(box);

		const lines: number[][] = [];
		for (const pair of SelectionSnap.neighbourPairs(row, axis)) {
			const from = pair[0][axis] + pair[0][axis + 2];
			const to = pair[1][axis];
			if (Math.abs(to - from - gap) >= 1) continue;

			const cross = 1 - axis;
			const low = Math.max(pair[0][cross], pair[1][cross]);
			const high = Math.min(
				pair[0][cross] + pair[0][cross + 2],
				pair[1][cross] + pair[1][cross + 2],
			);
			const across = (low + high) / 2;

			lines.push(
				axis === 0 ? [from, across, to, across] : [across, from, across, to],
			);
		}

		return lines;
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

		// The edges the dragged thing lies on, or else the one place it lined up with.
		const across =
			this.edgesX.length > 0 ? this.edgesX
			: this.closestX !== null && !this.spacingX ? [this.closestX]
			: [];
		for (const x of across)
			this.drawXAxis(
				section,
				section.containerObject.getDocumentAnchor()[0] +
					x -
					app.activeDocument.activeLayout.viewedRectangle.pX1,
			);

		const down =
			this.edgesY.length > 0 ? this.edgesY
			: this.closestY !== null && !this.spacingY ? [this.closestY]
			: [];
		for (const y of down)
			this.drawYAxis(
				section,
				section.containerObject.getDocumentAnchor()[1] +
					y -
					app.activeDocument.activeLayout.viewedRectangle.pY1,
			);

		section.context.closePath();

		section.context.setLineDash([]);
		for (const gap of this.equalGaps) this.drawGap(section, gap);

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

	/// Draws one of the equal gaps as a line with an arrow at either end.
	private drawGap(section: CanvasSectionObject, gap: number[]) {
		Util.ensureValue(app.activeDocument);

		const anchor = section.containerObject.getDocumentAnchor();
		const view = app.activeDocument.activeLayout.viewedRectangle;
		const x1 = anchor[0] + gap[0] - view.pX1;
		const y1 = anchor[1] + gap[1] - view.pY1;
		const x2 = anchor[0] + gap[2] - view.pX1;
		const y2 = anchor[1] + gap[3] - view.pY1;

		const length = Math.hypot(x2 - x1, y2 - y1);
		if (length < 1) return;

		// Along the line, and across it, both of length 1.
		const dx = (x2 - x1) / length;
		const dy = (y2 - y1) / length;
		const head = Math.min(5 * app.dpiScale, length / 2);

		section.context.lineWidth = app.dpiScale;
		section.context.beginPath();
		section.context.moveTo(x1, y1);
		section.context.lineTo(x2, y2);

		for (const [x, y, sign] of [
			[x1, y1, 1],
			[x2, y2, -1],
		]) {
			section.context.moveTo(
				x + sign * head * dx - head * dy,
				y + sign * head * dy + head * dx,
			);
			section.context.lineTo(x, y);
			section.context.lineTo(
				x + sign * head * dx + head * dy,
				y + sign * head * dy - head * dx,
			);
		}

		section.context.stroke();
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
