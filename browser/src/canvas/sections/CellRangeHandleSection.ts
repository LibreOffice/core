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

// The drag handle of a cell marker, on the corner of its range. The drag runs here: the range
// under the pointer is drawn as it moves, and the marker's command is sent once, at the end.
app.definitions.CellRangeHandleSection = class CellRangeHandleSection extends (
	AutoFillBaseSection
) {
	processingOrder: number = app.CSections.CellRangeHandle.processingOrder;
	drawingOrder: number = app.CSections.CellRangeHandle.drawingOrder;
	zIndex: number = app.CSections.CellRangeHandle.zIndex;

	constructor(name: string, markerSection: CellRangeMarkerSection) {
		super(name);
		// Draw the triangle in onDraw(); '' stops the container filling a square.
		this.sectionProperties.fillColor = this.backgroundColor;
		this.backgroundColor = '';

		this.sectionProperties.markerSection = markerSection;
		this.sectionProperties.cellRange = null;
		this.sectionProperties.part = -1;
		this.sectionProperties.command = null;
		this.sectionProperties.dragRange = null;
	}

	// The range this handle hangs off, as cell indexes, the sheet it is on, and the command a
	// drag of it sends.
	public setCellRange(cellRange: cool.Bounds, part: number, command: string) {
		this.sectionProperties.cellRange = cellRange;
		this.sectionProperties.part = part;
		this.sectionProperties.command = command;
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

	// A press a few pixels off the handle still takes it. The handle is six pixels across and
	// the drag starts here, so the press cannot ask for that much accuracy.
	public isHit(point: number[]): boolean {
		const margin = 2 + 2 * app.dpiScale;
		const rtl = app.map._docLayer.isCalcRTL();
		const left =
			this.myTopLeft[0] + (this.documentObject && rtl ? -this.size[0] : 0);

		return (
			point[0] >= left - margin &&
			point[0] <= left + this.size[0] + margin &&
			point[1] >= this.myTopLeft[1] - margin &&
			point[1] <= this.myTopLeft[1] + this.size[1] + margin
		);
	}

	public calculatePositionViaCellSelection(point: Array<number>) {
		// not used for CellRangeHandleSection
	}

	// Give bottom-right position of the range's bottom-right cell, in core pixels.
	public calculatePositionViaCellCursor(point: Array<number>) {
		// Anchor the marker's bottom-right at the point.
		this.sectionProperties.cellCursorPoint = [
			point[0] - this.size[0],
			point[1] - this.size[1],
		];
		this.setMarkerPosition();
	}

	// The range the pointer asks for. The start stays, the end follows the pointer in the one
	// direction it moved furthest in, and the first row and one below it are kept.
	private rangeForPoint(point: cool.SimplePoint): cool.Bounds | null {
		const sheetGeometry = app.map._docLayer.sheetGeometry;
		const cellRange = this.sectionProperties.cellRange;
		if (!sheetGeometry || !cellRange) return null;

		// The point is local to the handle, position is where the handle sits, so the two
		// together give the pointer in document pixels.
		const pointer = new cool.Point(
			this.position[0] + point.pX,
			this.position[1] + point.pY,
		);
		const cell = sheetGeometry.getCellFromPos(pointer, 'corepixels');

		const start = cellRange.getTopLeft();
		const end = cellRange.getBottomRight();
		const alongColumns =
			Math.abs(point.pX - this.size[0]) > Math.abs(point.pY - this.size[1]);

		return new cool.Bounds(
			start,
			new cool.Point(
				alongColumns ? Math.max(cell.x, start.x) : end.x,
				alongColumns ? end.y : Math.max(cell.y, start.y + 1),
			),
		);
	}

	public onMouseDown(point: cool.SimplePoint, e: MouseEvent) {
		this.sectionProperties.dragRange = this.sectionProperties.cellRange;
	}

	public onMouseMove(
		point: cool.SimplePoint,
		dragDistance: Array<number>,
		e: MouseEvent,
	) {
		if (dragDistance === null) return;

		const cellRange = this.rangeForPoint(point);
		if (!cellRange) return;

		this.sectionProperties.dragRange = cellRange;
		this.sectionProperties.markerSection.showDragRange(
			cellRange,
			this.sectionProperties.part,
			this.sectionProperties.fillColor,
		);
	}

	// The range the drag ended on goes to the engine as the marker's own command, under the
	// CellRange parameter every such command takes.
	public onMouseUp(point: cool.SimplePoint, e: MouseEvent) {
		const cellRange = this.sectionProperties.dragRange;
		const oldRange = this.sectionProperties.cellRange;
		this.sectionProperties.dragRange = null;
		this.sectionProperties.markerSection.hideDragRange();

		if (!cellRange || !oldRange || cellRange.equals(oldRange)) return;
		if (!this.sectionProperties.command) return;

		app.map.sendUnoCommand(this.sectionProperties.command, {
			CellRange: {
				type: 'string',
				value:
					cellRange.min.x +
					', ' +
					cellRange.min.y +
					', ' +
					cellRange.max.x +
					', ' +
					cellRange.max.y,
			},
		});
	}

	public onMouseEnter() {
		this.context.canvas.style.cursor = 'se-resize';
	}

	public onDoubleClick(point: cool.SimplePoint, e: MouseEvent) {
		// no-op: a range handle does not respond to double-click
	}
};
