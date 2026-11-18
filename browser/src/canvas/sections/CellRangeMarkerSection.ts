// @ts-strict-ignore
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

/*
	See CanvasSectionContainer.ts explanations about sections, event handlers and more.

	Marks out the cell ranges the engine reports, each kind under its own name so one kind
	leaves the others alone. The pixels come from the sheet geometry on every draw, so a marker
	follows scrolling and zoom. The styled tables and sparkline groups come from the cell range
	inventory of each sheet, which the client keeps and draws by itself.
*/

// A fillOpacity fills the ranges in color, otherwise they get a border in it. A handleCommand
// puts a drag handle on the corner of each range, which sends that command when dragged.
interface CellRangeMarkerStyle {
	color?: string;
	fillOpacity?: number;
	dashed?: boolean;
	handleCommand?: string;
}

// One kind of marker: its ranges as cell indexes, the sheet they are on, and how they are drawn.
interface CellRangeMarkers {
	cellRanges: cool.Bounds[];
	part: number;
	style: CellRangeMarkerStyle;
}

/**
 * One styled table of a sheet, and whether its cells can be edited.
 */
interface StyledTableRange {
	range: cool.Bounds;
	editable: boolean;
}

/**
 * Client-side cache of special cell ranges on a sheet that we may need to
 * highlight when the cursor is on them.
 */
interface SheetCellRanges {
	tables: StyledTableRange[];
	sparklineGroups: cool.Bounds[][];
	mergedCells: cool.Bounds[];
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
		this.sectionProperties.handleCounts = new Map<string, number>();
		this.sectionProperties.sheetCellRanges = new Map<number, SheetCellRanges>();
		this.isAlwaysVisible = true;
	}

	// Store the cell ranges of a sheet, replacing the ones stored before.
	public setSheetCellRanges(part: number, ranges: SheetCellRanges) {
		this.sectionProperties.sheetCellRanges.set(part, ranges);
		this.containerObject?.requestReDraw();
	}

	public hasSheetCellRanges(part: number): boolean {
		return this.sectionProperties.sheetCellRanges.has(part);
	}

	// Drop the stored cell ranges of a sheet, or of every sheet when no part is given.
	public clearSheetCellRanges(part?: number) {
		if (part === undefined) this.sectionProperties.sheetCellRanges.clear();
		else this.sectionProperties.sheetCellRanges.delete(part);
	}

	// Mark out the editable tables of a sheet from its stored cell ranges, each with a handle that
	// resizes the table.
	public showTableMarkers(part: number) {
		const ranges: SheetCellRanges | undefined =
			this.sectionProperties.sheetCellRanges.get(part);
		const tableRanges = ranges
			? ranges.tables
					.filter((table) => table.editable)
					.map((table) => table.range)
			: [];

		if (!tableRanges.length) {
			this.clearMarkers('TableRange');
			return;
		}

		this.setMarkers('TableRange', tableRanges, part, {
			handleCommand: '.uno:SetCalcTableRange',
		});
	}

	/**
	 * Get the merged cell of a sheet that contains a cell, or null when
	 * there is none, or when the sheet's cell ranges are not yet available.
	 */
	public getMergedCellAt(
		part: number,
		col: number,
		row: number,
	): cool.Bounds | null {
		const ranges: SheetCellRanges | undefined =
			this.sectionProperties.sheetCellRanges.get(part);
		if (!ranges) return null;

		const mergedCell = ranges.mergedCells.find((range) =>
			range.contains([col, row]),
		);
		return mergedCell ?? null;
	}

	/**
	 * Extend a cell range of a sheet, as [startCol, startRow, endCol, endRow],
	 * to account for every merged cell that it touches.
	 */
	public coverMergedCells(part: number, cellRange: number[]): number[] {
		const ranges: SheetCellRanges | undefined =
			this.sectionProperties.sheetCellRanges.get(part);
		if (!ranges) return cellRange;

		// Growing over one merged cell may reach another one.
		const covered = new cool.Bounds(
			new cool.Point(cellRange[0], cellRange[1]),
			new cool.Point(cellRange[2], cellRange[3]),
		);

		// NB: This can potentially become a bottleneck when the number of
		// merged cell areas grows.
		for (let extended = true; extended;) {
			extended = false;
			for (const mergedCell of ranges.mergedCells) {
				if (!covered.intersects(mergedCell)) continue;
				if (covered.contains(mergedCell)) continue;

				covered.extend(mergedCell.min);
				covered.extend(mergedCell.max);
				extended = true;
			}
		}
		return [covered.min.x, covered.min.y, covered.max.x, covered.max.y];
	}

	// Get the sparkline group of a sheet whose ranges contain the cell cursor, if any.
	private getSparklineGroupAtCursor(part: number): cool.Bounds[] | undefined {
		const ranges: SheetCellRanges | undefined =
			this.sectionProperties.sheetCellRanges.get(part);
		if (!ranges || !app.calc.cellCursorVisible) return undefined;

		const cursor = app.calc.cellAddress.toArray();
		return ranges.sparklineGroups.find((group) =>
			group.some((range) => range.contains(cursor)),
		);
	}

	// Replace the ranges drawn under name.
	public setMarkers(
		name: string,
		cellRanges: cool.Bounds[],
		part: number,
		style: CellRangeMarkerStyle,
	) {
		this.sectionProperties.markers.set(name, { cellRanges, part, style });
		this.syncHandles(name);
		this.containerObject?.requestReDraw();
	}

	public clearMarkers(name: string) {
		if (!this.sectionProperties.markers.delete(name)) return;

		this.syncHandles(name);
		this.containerObject?.requestReDraw();
	}

	private handleName(name: string, index: number): string {
		return name + ' handle ' + index;
	}

	// One handle section per range, and none for a marker without a handle, on another sheet,
	// or in a document that cannot be edited. Adding and removing sections lays the container
	// out again, so a marker change is what calls this.
	private syncHandles(name: string) {
		const markers = this.sectionProperties.markers.get(name);
		const sheetGeometry = app.map._docLayer.sheetGeometry;
		const wanted =
			markers &&
			markers.style.handleCommand &&
			markers.part === app.map._docLayer._selectedPart &&
			app.map.isEditMode()
				? markers.cellRanges.length
				: 0;
		const current = this.sectionProperties.handleCounts.get(name) ?? 0;

		for (let i = current; i < wanted; i++)
			app.sectionContainer.addSection(
				new app.definitions.CellRangeHandleSection(
					this.handleName(name, i),
					this,
				),
			);

		for (let i = wanted; i < current; i++)
			app.sectionContainer.removeSection(this.handleName(name, i));

		this.sectionProperties.handleCounts.set(name, wanted);

		if (wanted && sheetGeometry)
			this.positionHandles(name, markers, sheetGeometry);
	}

	// A handle sits with its bottom-right corner on the corner of the range's last cell.
	private positionHandles(
		name: string,
		markers: CellRangeMarkers,
		sheetGeometry: any,
	) {
		const count = this.sectionProperties.handleCounts.get(name) ?? 0;

		for (let i = 0; i < count; i++) {
			const handle: any = app.sectionContainer.getSectionWithName(
				this.handleName(name, i),
			);
			if (!handle) continue;

			const endCell = sheetGeometry.getCellSimpleRectangle(
				markers.cellRanges[i].max.x,
				markers.cellRanges[i].max.y,
			);
			handle.calculatePositionViaCellCursor([endCell.pX2, endCell.pY2]);
			handle.setCellRange(
				markers.cellRanges[i],
				markers.part,
				markers.style.handleCommand,
			);
		}
	}

	// The range a handle drag is heading for, drawn by the client alone while the drag lasts.
	public showDragRange(cellRange: cool.Bounds, part: number, color: string) {
		this.setMarkers('HandleDrag', [cellRange], part, { color, dashed: true });
	}

	public hideDragRange() {
		this.clearMarkers('HandleDrag');
	}

	private drawMarkers(markers: CellRangeMarkers, sheetGeometry: any) {
		if (!markers.style.color) return;

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

		this.sectionProperties.markers.forEach(
			(markers: CellRangeMarkers, name: string) => {
				if (markers.part !== docLayer._selectedPart) return;

				this.drawMarkers(markers, docLayer.sheetGeometry);
				this.positionHandles(name, markers, docLayer.sheetGeometry);
			},
		);

		// The group under the cell cursor is filled, the others are not marked.
		const sparklineGroup = this.getSparklineGroupAtCursor(
			docLayer._selectedPart,
		);
		if (sparklineGroup) {
			const color = getComputedStyle(document.documentElement)
				.getPropertyValue('--color-primary')
				.trim();
			this.drawMarkers(
				{
					cellRanges: sparklineGroup,
					part: docLayer._selectedPart,
					style: { color: color || '#0b87e7', fillOpacity: 0.25 },
				},
				docLayer.sheetGeometry,
			);
		}

		this.context.restore();
	}
}
