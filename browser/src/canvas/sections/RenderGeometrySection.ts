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
	Draws two hairline rectangles per object of the part a view shows, each in the object's own
	color: the upright box the object paints into, and the unit rectangle put through the object's
	transformation, which shows the rotation and the shear. It is a way to see the geometry the
	client holds for each object, which is what a hit test and a client-side move work from.

	The rectangles come from the vector rendering cache, so nothing is asked of the engine for them
	and an object with no primitives of its own, such as a group, still shows its box. It draws
	after the section that paints the page and before the overlay, so the cursors, selections and
	handles the overlay draws stay on top.

	The debug panel's "Debug Overlays" tool adds and removes the section.
*/

/* global app RenderManager */

class RenderGeometrySection extends CanvasSectionObject {
	zIndex: number = app.CSections.RenderGeometry.zIndex;
	drawingOrder: number = app.CSections.RenderGeometry.drawingOrder;
	processingOrder: number = app.CSections.RenderGeometry.processingOrder;
	boundToSection: string = 'tiles';
	interactable: boolean = false;

	// True once the class listens to the vector cache. The section comes and goes with the
	// debug tool, so the listener belongs to the class and is registered once.
	private static _listening = false;

	constructor() {
		super(app.CSections.RenderGeometry.name);

		// The cache fills and changes on its own, so a redraw follows what arrives.
		if (!RenderGeometrySection._listening) {
			RenderGeometrySection._listening = true;
			RenderManager.onVectorChanged(() => {
				if (app.sectionContainer) app.sectionContainer.requestReDraw();
			});
		}
	}

	/*
		An object keeps its color for as long as it lives, so a rectangle that moves or resizes is
		easy to follow. Stepping the hue by the golden angle puts neighboring ids far apart.
	*/
	public static colorOfObject(id: number): string {
		return 'hsl(' + ((id * 137.508) % 360).toFixed(1) + ', 100%, 45%)';
	}

	/*
		The four corners of the unit rectangle put through the transformation, in twips. They are
		transformed here rather than by the canvas so the line stays one pixel wide whatever the
		object's scale is.
	*/
	public static unitRectangleCorners(transform: number[]): number[][] {
		const [a, b, c, d, e, f] = transform;
		return [
			[e, f],
			[a + e, b + f],
			[a + c + e, b + d + f],
			[c + e, d + f],
		];
	}

	onDraw(): void {
		if (!app.map._debug.renderGeometryOn) return;
		// The page section skips the animation frames too, so the two stay lined up.
		if (this.containerObject.isInZoomAnimation()) return;

		const docLayer = app.map._docLayer;
		if (!docLayer || app.file.fileBasedView) return;

		const data = RenderManager.requestPart(docLayer._selectedPart);
		if (!data) return;

		// The section container has translated the context to the document anchor, so the scroll
		// offset alone maps the part origin into place, as it does for the page itself.
		Util.ensureValue(app.activeDocument);
		const viewedRectangle = app.activeDocument.activeLayout.viewedRectangle;
		const xDiff = -viewedRectangle.pX1;
		const yDiff = -viewedRectangle.pY1;
		const scale = app.twipsToPixels;

		this.context.save();
		this.context.lineWidth = 1;
		// Six tenths see-through, so an outline reads over the object it describes without hiding
		// it.
		this.context.globalAlpha = 0.4;

		for (const id of data.order) {
			const object = data.objects.get(id);
			if (!object) continue;

			this.context.strokeStyle = RenderGeometrySection.colorOfObject(id);

			if (
				object.x !== undefined &&
				object.y !== undefined &&
				object.width !== undefined &&
				object.height !== undefined
			) {
				// The half pixel offset puts the one pixel wide line on a pixel rather than across
				// two.
				this.context.strokeRect(
					xDiff + Math.round(object.x * scale) + 0.5,
					yDiff + Math.round(object.y * scale) + 0.5,
					Math.round(object.width * scale),
					Math.round(object.height * scale),
				);
			}

			if (object.transform)
				this.strokeUnitRectangle(object.transform, xDiff, yDiff);
		}

		this.context.restore();
	}

	private strokeUnitRectangle(
		transform: number[],
		xDiff: number,
		yDiff: number,
	): void {
		const scale = app.twipsToPixels;
		const corners = RenderGeometrySection.unitRectangleCorners(transform);

		this.context.beginPath();
		corners.forEach(([x, y]: number[], index: number) => {
			const pX = xDiff + Math.round(x * scale) + 0.5;
			const pY = yDiff + Math.round(y * scale) + 0.5;
			if (index === 0) this.context.moveTo(pX, pY);
			else this.context.lineTo(pX, pY);
		});
		this.context.closePath();
		this.context.stroke();
	}

	/// Adds the section when it is missing, and asks for a redraw either way.
	public static update(): void {
		if (!app.sectionContainer) return;

		if (
			!app.sectionContainer.doesSectionExist(app.CSections.RenderGeometry.name)
		)
			app.sectionContainer.addSection(new RenderGeometrySection());

		app.sectionContainer.requestReDraw();
	}

	/// Removes the section and asks for a redraw, so its rectangles go.
	public static remove(): void {
		if (!app.sectionContainer) return;

		if (
			app.sectionContainer.doesSectionExist(app.CSections.RenderGeometry.name)
		)
			app.sectionContainer.removeSection(app.CSections.RenderGeometry.name);

		app.sectionContainer.requestReDraw();
	}
}
