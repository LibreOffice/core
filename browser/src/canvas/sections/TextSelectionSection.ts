// @ts-strict-ignore

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
	This class is only for rendering the selected text from all views.
*/
class TextSelectionSection extends CanvasSectionObject {
	interactable = false;
	processingOrder = app.CSections.TextSelection.processingOrder;
	drawingOrder = app.CSections.TextSelection.drawingOrder;
	zIndex = app.CSections.TextSelection.zIndex;
	documentObject: boolean = true;

	/*
		There are questions like: Can a view have a selection in a part other than its currently selected part?
		New view layouts can allow multiple parts to be drawn at the same time.
		Unfortunately, we need to keep the selection's part and mode separate from the view.
		The behaviour between selections of a specific view:
			* Behaviour should be decided based on what the app is capable (remove one selection when other is initiated or keep them all etc).
			* Selections data (not the socket message, its data) should be handled in the DocumentViewBase class.
	*/
	private mode: number;
	private part: number;
	public color: string;
	private polygons: Array<Array<cool.SimplePoint>> = [[]];

	// The polygons drawn when the selection last changed, while it glides from them to the new
	// ones, or null while it stands.
	private glidesFrom: Array<Array<cool.SimplePoint>> | null = null;
	// When the glide began, in the time of performance.now().
	private glideStart: number = 0;
	// The animation frame the glide draws next, or null while none is asked for.
	private glideFrame: number | null = null;

	constructor(name: string, mode: number, part: number, color: string) {
		super(name);
		this.part = part;
		this.mode = mode;
		this.color = color;
	}

	public setSelectionInfo(
		mode: number,
		part: number,
		polygons: Array<Array<cool.SimplePoint>>,
	) {
		const glides =
			RenderManager.isVectorRendering() &&
			Cursor.slideTime > 0 &&
			mode === this.mode &&
			part === this.part &&
			TextSelectionSection.nearby(this.polygons, polygons) &&
			window.matchMedia?.('(prefers-reduced-motion: reduce)').matches !== true;
		this.glidesFrom = glides ? this.shownPolygons(performance.now()) : null;
		this.glideStart = performance.now();
		this.mode = mode;
		this.part = part;
		this.polygons = polygons;
		if (glides) this.glide();
	}

	/*
		In a view drawn from vector primitives the selection glides to its new place when it
		keeps its polygons and their corners and no corner moves far, as when Shift and an
		arrow grow or shrink it on a line, so the eye follows the end that moves. A selection
		that changes its shape, or one of the same shape elsewhere, is drawn at once.
	*/
	private static nearby(
		before: Array<Array<cool.SimplePoint>>,
		after: Array<Array<cool.SimplePoint>>,
	): boolean {
		if (before.length === 0 || before.length !== after.length) return false;
		return before.every((polygon, i) => {
			const other = after[i];
			if (polygon.length === 0 || polygon.length !== other.length) return false;
			// A character or two is as far as a corner moves: twice the height of its
			// polygon covers that in any font.
			const reach = 2 * TextSelectionSection.heightOf(polygon);
			return polygon.every(
				(corner, j) =>
					Math.abs(corner.x - other[j].x) <= reach &&
					Math.abs(corner.y - other[j].y) <= reach,
			);
		});
	}

	private static heightOf(polygon: Array<cool.SimplePoint>): number {
		let top = Infinity;
		let bottom = -Infinity;
		for (const corner of polygon) {
			top = Math.min(top, corner.y);
			bottom = Math.max(bottom, corner.y);
		}
		return bottom - top;
	}

	// Draws the canvas again on every frame until the glide is over. A change during a glide
	// restarts its time, and the frames already asked for carry on.
	private glide(): void {
		if (this.glideFrame !== null) return;
		this.glideFrame = requestAnimationFrame(() => {
			this.glideFrame = null;
			app.sectionContainer.requestReDraw();
			if (performance.now() - this.glideStart < Cursor.slideTime) this.glide();
		});
	}

	// The polygons to draw at that time: on the way from the ones of the last change to the new
	// ones while the selection glides, slowing down as it arrives, or the new ones.
	private shownPolygons(now: number): Array<Array<cool.SimplePoint>> {
		const from = this.glidesFrom;
		const progress = (now - this.glideStart) / Cursor.slideTime;
		if (!from || !(progress < 1)) {
			this.glidesFrom = null;
			return this.polygons;
		}

		const eased = 1 - (1 - progress) * (1 - progress);
		return this.polygons.map((polygon, i) =>
			polygon.map((point, j) => {
				const start = from[i][j];
				return new cool.SimplePoint(
					start.x + (point.x - start.x) * eased,
					start.y + (point.y - start.y) * eased,
					point.part,
				);
			}),
		);
	}

	onDraw(frameCount?: number, elapsedTime?: number): void {
		Util.ensureValue(app.activeDocument);
		Util.ensureValue(app.calc.splitCoordinate);

		// Remove when visibility checks are done in layout views (on simple point instances).
		if (
			!app.activeDocument.isModeActive(this.mode) ||
			this.part !== app.map._docLayer._selectedPart ||
			this.polygons.length === 0
		)
			return;

		this.context.translate(-this.myTopLeft[0], -this.myTopLeft[1]);

		this.context.globalAlpha = 0.25;
		this.context.strokeStyle = this.color;
		this.context.fillStyle = this.color;

		const polygons = this.shownPolygons(performance.now());
		for (let i = 0; i < polygons.length; i++) {
			const polygon = polygons[i];

			this.context.beginPath();

			for (let j = 0; j < polygon.length; j++) {
				const point = polygon[j].clone();

				if (j === 0) this.context.moveTo(point.vX, point.vY);
				else this.context.lineTo(point.vX, point.vY);
			}

			this.context.closePath();
			// In Calc the selection has to line up with the cell grid. The 1px
			// stroke is centred on the polygon edge, so its outer half is
			// painted just outside the boundary and shows up as a faint band one
			// pixel past the right and bottom grid lines. The fill already
			// covers the selected cells exactly, so the stroke is dropped here.
			if (!app.map._docLayer.isCalc()) this.context.stroke();
			this.context.fill();
		}

		this.context.globalAlpha = 1.0;

		// We are done. Set the pen back to its initial position. This is needed or all other sections can draw at unexpected coordinates.
		this.context.translate(this.myTopLeft[0], this.myTopLeft[1]);
	}
}
