/* -*- js-indent-level: 8; fill-column: 100 -*- */
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
	The grey button in the lower right corner of a table that is a shape of its own, as in
	Impress. It sits between the button that adds a column and the one that adds a row.
	Dragging it resizes the table in both directions at once: the drop gives the table its new
	width and height, with the upper left corner staying where it is. Core spreads the change
	over the columns and the rows.
*/
class TableCornerResizeMarkerSection extends HTMLObjectSection {
	static readonly sectionName: string = 'TableCornerResizeMarker';

	/*
		The distance between the table's right and bottom edges and the button, in CSS pixels.
		The lower right corner of the table lands this far up and left of the button.
	*/
	private offset: number;

	constructor(
		documentPosition: cool.SimplePoint,
		size: number,
		offset: number,
	) {
		super(
			TableCornerResizeMarkerSection.sectionName,
			size,
			size,
			documentPosition,
			'table-corner-resize-marker',
			true,
		);

		this.offset = offset;
		this.sectionProperties.dragStartPosition = null;
		this.sectionProperties.initialPosition = this.position.slice();
		// The smallest position the button can be dragged to, in core pixels.
		this.sectionProperties.leftMost = 0;
		this.sectionProperties.topMost = 0;
	}

	private offsetInCorePixels(): number {
		return this.offset * app.dpiScale;
	}

	/*
		The last column and the last row keep at least the width the resize markers keep between
		two borders, so the button stops that far right of the previous column border and that
		far below the previous row border.
	*/
	private calculateLimits(): void {
		Util.ensureValue(app.activeDocument);
		const tableMiddleware = app.activeDocument.tableMiddleware;
		const columns = tableMiddleware.currentTableData.columns;
		const rows = tableMiddleware.currentTableData.rows;

		const lastColumnBorder =
			columns.entries.length > 0
				? columns.entries[columns.entries.length - 1].position
				: columns.left;
		const lastRowBorder =
			rows.entries.length > 0
				? rows.entries[rows.entries.length - 1].position
				: rows.left;

		const margin =
			tableMiddleware.resizeMarkerMaxApproximation + this.offsetInCorePixels();

		this.sectionProperties.leftMost =
			(columns.tableOffset + lastColumnBorder) * app.twipsToPixels + margin;
		this.sectionProperties.topMost =
			(rows.tableOffset + lastRowBorder) * app.twipsToPixels + margin;
	}

	// The lower right corner of the table the button stands for, in twips.
	private cornerPosition(): number[] {
		const offset = this.offsetInCorePixels();
		return [
			Math.round((this.position[0] - offset) * app.pixelsToTwips),
			Math.round((this.position[1] - offset) * app.pixelsToTwips),
		];
	}

	public onMouseEnter(point: cool.SimplePoint, e: MouseEvent): void {
		this.getHTMLObject().classList.add('hovered');
		this.context.canvas.style.cursor = 'nwse-resize';
	}

	public onMouseLeave(point: cool.SimplePoint, e: MouseEvent): void {
		this.sectionProperties.dragStartPosition = null;
		this.getHTMLObject().classList.remove('hovered');
	}

	public onMouseDown(point: cool.SimplePoint, e: MouseEvent): void {
		this.sectionProperties.dragStartPosition = point;
		// A touch gives no mouse enter, so the limits are worked out at the press.
		this.calculateLimits();
	}

	public onMouseMove(
		point: cool.SimplePoint,
		dragDistance: Array<number>,
		e: MouseEvent,
	): void {
		if (
			!this.containerObject.isDraggingSomething() ||
			!this.sectionProperties.dragStartPosition
		)
			return;

		// The point is relative to the button, which moves with the mouse, so the distance
		// from where the press landed on the button is how far the button still has to go.
		const x = Math.max(
			this.position[0] + point.pX - this.sectionProperties.dragStartPosition.pX,
			this.sectionProperties.leftMost,
		);
		const y = Math.max(
			this.position[1] + point.pY - this.sectionProperties.dragStartPosition.pY,
			this.sectionProperties.topMost,
		);

		this.setPosition(x, y);
		this.adjustHTMLObjectPosition();
		this.containerObject.requestReDraw();
	}

	private onDragEnd(): void {
		Util.ensureValue(app.activeDocument);
		const tableMiddleware = app.activeDocument.tableMiddleware;
		const columns = tableMiddleware.currentTableData.columns;
		const rows = tableMiddleware.currentTableData.rows;
		const corner = this.cornerPosition();
		const width = corner[0] - (columns.tableOffset + columns.left);
		const height = corner[1] - (rows.tableOffset + rows.left);

		// The button goes back to the corner it started from. The resized table arrives with
		// the next table selection message, which places every marker anew.
		this.setPosition(
			this.sectionProperties.initialPosition[0],
			this.sectionProperties.initialPosition[1],
		);
		this.adjustHTMLObjectPosition();
		this.containerObject.requestReDraw();

		if (width <= 0 || height <= 0) return;

		// Without a position the upper left corner stays where it is.
		app.map.sendUnoCommand('.uno:TransformDialog', {
			TransformWidth: { type: 'long', value: width },
			TransformHeight: { type: 'long', value: height },
		});
	}

	public onMouseUp(point: cool.SimplePoint, e: MouseEvent): void {
		this.sectionProperties.dragStartPosition = null;

		if (
			this.containerObject.isDraggingSomething() &&
			this.containerObject.targetSection === this.name
		)
			this.onDragEnd();
	}

	public onDraw(frameCount?: number, elapsedTime?: number): void {
		super.onDraw(frameCount, elapsedTime);

		if (
			!this.containerObject.isDraggingSomething() ||
			this.containerObject.targetSection !== this.name
		)
			return;

		// The outline the table gets with the button where it is now. The drawing context
		// starts at the button, so the table's upper left corner is up and left of the origin.
		Util.ensureValue(app.activeDocument);
		const tableMiddleware = app.activeDocument.tableMiddleware;
		const left = tableMiddleware.getTableLeftX() - this.position[0];
		const top = tableMiddleware.getTableTopY() - this.position[1];
		const corner = -this.offsetInCorePixels();

		this.context.strokeStyle = '#3388FF';
		this.context.lineWidth = 2;
		this.context.beginPath();
		this.context.rect(left, top, corner - left, corner - top);
		this.context.stroke();
		this.context.closePath();
	}
}
