// @ts-strict-ignore
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
	See CanvasSectionContainer.ts explanations about sections, event handlers and more.

	Marks out the cell ranges the engine reports, each kind under its own name so one kind
	leaves the others alone. The pixels come from the sheet geometry on every draw, so a marker
	follows scrolling and zoom.
*/

// A fillOpacity fills the ranges in color, otherwise they get a border in it.
interface CellRangeMarkerStyle {
	color: string;
	fillOpacity?: number;
	dashed?: boolean;
}

// One kind of marker: its ranges as cell indexes, the sheet they are on, and how they are drawn.
interface CellRangeMarkers {
	cellRanges: cool.Bounds[];
	part: number;
	style: CellRangeMarkerStyle;
}

class CellRangeMarkerSection extends CanvasSectionObject {
	processingOrder: number = app.CSections.CellRangeMarker.processingOrder;
	drawingOrder: number = app.CSections.CellRangeMarker.drawingOrder;
	zIndex: number = app.CSections.CellRangeMarker.zIndex;
	documentObject: boolean = true;
	interactable: boolean = false;
	position: number[] = [0, 0];

	constructor() {
		super(app.CSections.CellRangeMarker.name);

		this.sectionProperties.markers = new Map<string, CellRangeMarkers>();
		this.isAlwaysVisible = true;
	}

	// Replace the ranges drawn under name.
	public setMarkers(
		name: string,
		cellRanges: cool.Bounds[],
		part: number,
		style: CellRangeMarkerStyle,
	) {
		this.sectionProperties.markers.set(name, { cellRanges, part, style });
		this.containerObject?.requestReDraw();
	}

	public clearMarkers(name: string) {
		if (this.sectionProperties.markers.delete(name))
			this.containerObject?.requestReDraw();
	}

	private drawMarkers(markers: CellRangeMarkers, sheetGeometry: any) {
		this.context.strokeStyle = markers.style.color;
		this.context.fillStyle = markers.style.color;

		const dashLength = 4 * app.roundedDpiScale;
		this.context.setLineDash(
			markers.style.dashed ? [dashLength, dashLength] : [],
		);

		for (const cellRange of markers.cellRanges) {
			const startCell = sheetGeometry.getCellSimpleRectangle(
				cellRange.min.x,
				cellRange.min.y,
			);
			const endCell = sheetGeometry.getCellSimpleRectangle(
				cellRange.max.x,
				cellRange.max.y,
			);
			const rectangle = new cool.SimpleRectangle(
				startCell.x1,
				startCell.y1,
				endCell.x2 - startCell.x1,
				endCell.y2 - startCell.y1,
			);

			if (markers.style.fillOpacity) {
				this.context.globalAlpha = markers.style.fillOpacity;
				this.drawViewRectangle(rectangle, true);
				this.context.globalAlpha = 1;
			} else this.drawViewRectangle(rectangle);
		}
	}

	public onDraw() {
		const docLayer = app.map._docLayer;
		if (!docLayer.sheetGeometry) return;

		this.context.save();
		this.context.setTransform(1, 0, 0, 1, 0, 0);

		this.context.lineJoin = 'miter';
		this.context.lineCap = 'butt';
		this.context.lineWidth = app.roundedDpiScale;

		this.sectionProperties.markers.forEach((markers: CellRangeMarkers) => {
			if (markers.part === docLayer._selectedPart)
				this.drawMarkers(markers, docLayer.sheetGeometry);
		});

		this.context.restore();
	}
}
