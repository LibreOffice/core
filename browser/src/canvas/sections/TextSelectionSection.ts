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
		this.mode = mode;
		this.part = part;
		this.polygons = polygons;
	}

	onDraw(frameCount?: number, elapsedTime?: number): void {
		// Remove when visibility checks are done in layout views (on simple point instances).
		if (
			!app.activeDocument.isModeActive(this.mode) ||
			this.part !== app.map._docLayer._selectedPart ||
			this.polygons.length === 0
		)
			return;

		// We will use vX and vY. Thus, we need to set the pen position to canvas's origin.
		this.context.translate(-this.myTopLeft[0], -this.myTopLeft[1]);

		this.context.globalAlpha = 0.25;
		this.context.strokeStyle = this.color;
		this.context.fillStyle = this.color;

		let deflectionX = 0;
		let deflectionY = 0;

		if (app.map._docLayer._docType === 'spreadsheet') {
			if (app.isXOrdinateInFrozenPane(this.polygons[0][0].pX))
				deflectionX = app.activeDocument.activeLayout.viewedRectangle.pX1;

			if (app.isYOrdinateInFrozenPane(this.polygons[0][0].pY))
				deflectionY = app.activeDocument.activeLayout.viewedRectangle.pY1;
		}

		// In Calc RTL, polygons come from core in LTR document coords; mirror
		// each x around the tile section's right edge so the rectangle lands
		// on the visible cells that the user actually selected.
		const mirrorX = app.calc.isRTL();
		const mirrorAxis = mirrorX
			? 2 * app.sectionContainer.getDocumentAnchor()[0] +
				app.sectionContainer.getDocumentAnchorSection().size[0]
			: 0;
		const toCanvasX = (vX: number) =>
			mirrorX ? mirrorAxis - (vX + deflectionX) : vX + deflectionX;

		for (let i = 0; i < this.polygons.length; i++) {
			const polygon = this.polygons[i];

			this.context.beginPath();
			this.context.moveTo(
				toCanvasX(polygon[0].vX),
				polygon[0].vY + deflectionY,
			);

			for (let i = 1; i < polygon.length; i++) {
				this.context.lineTo(
					toCanvasX(polygon[i].vX),
					polygon[i].vY + deflectionY,
				);
			}

			this.context.closePath();
			this.context.stroke();
			this.context.fill();
		}

		this.context.globalAlpha = 1.0;

		// We are done. Set the pen back to its initial position. This is needed or all other sections can draw at unexpected coordinates.
		this.context.translate(this.myTopLeft[0], this.myTopLeft[1]);
	}
}
