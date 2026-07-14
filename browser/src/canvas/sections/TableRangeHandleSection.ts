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
/* See CanvasSectionContainer.ts for explanations. */

app.definitions.TableRangeHandleSection = class TableRangeHandleSection extends (
	AutoFillBaseSection
) {
	processingOrder: number = app.CSections.TableRangeHandle.processingOrder;
	drawingOrder: number = app.CSections.TableRangeHandle.drawingOrder;
	zIndex: number = app.CSections.TableRangeHandle.zIndex;

	constructor(name: string) {
		super(name);
		// Draw the triangle in onDraw(); '' stops the container filling a square.
		this.sectionProperties.fillColor = this.backgroundColor;
		this.backgroundColor = '';
	}

	public onDraw(): void {
		// v2/v4/v3 are view-pixel corners, so RTL is already handled.
		const r = this.boundingRectangle;
		this.context.save();
		this.context.setTransform(1, 0, 0, 1, 0, 0);
		this.context.beginPath();
		this.context.moveTo(r.v2X, r.v2Y); // top-right
		this.context.lineTo(r.v4X, r.v4Y); // bottom-right
		this.context.lineTo(r.v3X, r.v3Y); // bottom-left
		this.context.closePath();
		this.context.fillStyle = this.sectionProperties.fillColor;
		this.context.fill();
		this.context.restore();
	}

	public isHit(point: number[]): boolean {
		// Hit-test the triangle, not the bounding square.
		if (!super.isHit(point)) return false;
		if (this.size[0] <= 0 || this.size[1] <= 0) return true;
		const rtl = app.map._docLayer.isCalcRTL();
		const addition = this.documentObject && rtl ? -this.size[0] : 0;
		const u = (point[0] - (this.myTopLeft[0] + addition)) / this.size[0];
		const v = (point[1] - this.myTopLeft[1]) / this.size[1];
		return rtl ? v >= u : u + v >= 1;
	}

	public calculatePositionViaCellSelection(point: Array<number>) {
		// not used for TableRangeHandleSection
	}

	// Give bottom-right position of the table's bottom-right cell, in core pixels.
	public calculatePositionViaCellCursor(point: Array<number>) {
		// Anchor the marker's bottom-right at the point.
		this.sectionProperties.cellCursorPoint = [
			point[0] - this.size[0],
			point[1] - this.size[1],
		];
		this.setMarkerPosition();
	}

	protected isFillAreaVisible(): boolean {
		return !!this.sectionProperties.docLayer._tableRangeHandlesVisible;
	}

	public onMouseEnter() {
		this.context.canvas.style.cursor = 'se-resize';
	}

	public onDoubleClick(point: cool.SimplePoint, e: MouseEvent) {
		// no-op: TableRangeHandle does not respond to double-click
	}
};
