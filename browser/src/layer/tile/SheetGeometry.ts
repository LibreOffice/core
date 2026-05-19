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

namespace cool {

export type GeometryUnit = 'corepixels' | 'tiletwips' | 'printtwips';

export interface DimensionRange {
	start: number;
	end: number;
}

export interface CellRange {
	columnrange: DimensionRange;
	rowrange: DimensionRange;
}

export interface DimensionPosSize {
	startpos: number;
	size: number;
}

export interface DimensionStartEndPos {
	startpos: number;
	endpos: number;
}

export interface IndexAndSpan {
	index: number;
	span: SpanData;
}

export interface SheetGeometryCoreData {
	commandName: string;
	maxtiledcolumn: string;
	maxtiledrow: string;
	columns: SheetDimensionCoreData;
	rows: SheetDimensionCoreData;
}

export interface SheetDimensionCoreData {
	sizes: string;
	hidden: string;
	filtered: string;
	groups: string;
}

/**
 * Used to parse and store the .unoSheetGeometry data that allows fast lookups between
 * different units. See @GeometryUnit
 */
export class SheetGeometry {
	/** sheetGeomJSON is expected to be the parsed JSON message from core
	 * in response to client command '.uno:SheetGeometryData' with
	 * all flags (ie 'columns', 'rows', 'sizes', 'hidden', 'filtered',
	 * 'groups') enabled.
	 */
	private _part: number;
	private _columns: SheetDimension;
	private _rows: SheetDimension;
	private _unoCommand: string;
	public maxVisibleColumnIndex: number;
	public maxVisibleRowIndex: number;

	constructor(sheetGeomJSON: SheetGeometryCoreData, tileWidthTwips: number, tileHeightTwips: number,
		tileSizePixels: number, part: number) {

		// Remove the unnecessary checks only after SheetGeometry client code has moved to TS.
		if (typeof sheetGeomJSON !== 'object' ||
			typeof tileWidthTwips !== 'number' ||
			typeof tileHeightTwips !== 'number' ||
			typeof tileSizePixels !== 'number' ||
			typeof part !== 'number') {
			console.error('Incorrect constructor argument types or missing required arguments');
			return;
		}

		this._part = -1;
		this._columns = new SheetDimension();
		this._rows = new SheetDimension();
		this._unoCommand = '.uno:SheetGeometryData';

		// Set various unit conversion info early on because on update() call below, these info are needed.
		this.setTileGeometryData(tileWidthTwips, tileHeightTwips, tileSizePixels,
			false /* update position info ?*/);

		this.update(sheetGeomJSON, /* checkCompleteness */ true, part);
	}

	private checkMaxIndex(sheetGeomJSON: SheetGeometryCoreData, column: boolean): number {
		/*
			About "hidden" variable format:
			* This information is sent from the core side.
			* Represents the hidden rows / columns.
			* Works with "true - false - true - false" pattern.
				* Like: 5 8 10 20... -> Means that visible up  to 5 (including), hidden up to 8 (including), visible up to 10 (including), hidden up to 20 (including)...
			* But now, above format assumes the first row / column is visible.
			* To determine if the data starts with true or false, the first value in data is formatted differently.
				* If first is visible: 0:0 5 8 10 20... -> Means that visible up to 1 (index 0), hidden up to 5 (including 5), visible up to 8 (including), hidden up to 10 (including)...
				* If first is hidden: 1:0 5 8 10 20... -> Means that hidden up to 0, visible up to 5, hidden up to 8, visible up to 10...
		*/

		// We will check one special case here: Only a few rows/columns are visible on top, others are hidden all the way to the bottom/right.
		const hiddenInfo = column ? sheetGeomJSON.columns.hidden.trim() : sheetGeomJSON.rows.hidden.trim();

		const isFirstHidden = hiddenInfo.indexOf('1') === 0;
		const splitted = hiddenInfo.split(' ');
		const isLastHidden = (splitted.length % 2 === 0 && !isFirstHidden) || (splitted.length % 2 === 1 && isFirstHidden);

		if (splitted.length === 1) {
			if (isFirstHidden) return 0; // All hidden.
			else return parseInt(splitted[0].split(':')[1]); // All visible.
		}
		else if (!isLastHidden) { // Last rows / columns are visible.
			return parseInt(splitted[splitted.length - 1]);
		}

		/*
			Last rows / columns are hidden (below else if case):
				* We need the index from the item that comes right before the last one. Becase we want last visible index.
				* There are only 2 items, so we need the first item (index 0).
				* First item is written in a different format (0:1->hiddenOrNot:LastIndex).
				* We need to split the first item to get the last index (second item is hidden, we ignore that).
		*/
		else if (splitted.length === 2) {
			return parseInt(splitted[0].split(':')[1]);
		}
		else { // Last rows / columns are hidden and there are more than 2 items.
			return parseInt(splitted[splitted.length - 2]);
		}
	}

	public update(sheetGeomJSON: SheetGeometryCoreData, checkCompleteness: boolean, part: number): boolean {
		if (!this._testValidity(sheetGeomJSON, checkCompleteness)) {
			return false;
		}

		var updateOK = true;
		if (sheetGeomJSON.columns) {
			if (!this._columns.update(sheetGeomJSON.columns)) {
				console.error(this._unoCommand + ': columns update failed.');
				updateOK = false;
			}

			if (sheetGeomJSON.columns.hidden)
				this.maxVisibleColumnIndex = this.checkMaxIndex(sheetGeomJSON, true);
			else
				this.maxVisibleColumnIndex = parseInt(sheetGeomJSON.maxtiledcolumn);
		}

		if (sheetGeomJSON.rows) {
			if (!this._rows.update(sheetGeomJSON.rows)) {
				console.error(this._unoCommand + ': rows update failed.');
				updateOK = false;
			}

			if (sheetGeomJSON.rows.hidden)
				this.maxVisibleRowIndex = this.checkMaxIndex(sheetGeomJSON, false);
			else
				this.maxVisibleRowIndex = parseInt(sheetGeomJSON.maxtiledrow);
		}

		if (updateOK) {
			console.assert(typeof part === 'number', 'part must be a number');
			if (part !== this._part) {
				this._part = part;
			}
		}

		this._columns.setMaxIndex(+sheetGeomJSON.maxtiledcolumn);
		this._rows.setMaxIndex(+sheetGeomJSON.maxtiledrow);

		return updateOK;
	}

	public setTileGeometryData(tileWidthTwips: number, tileHeightTwips: number, tileSizePixels: number,
		updatePositions: boolean): void {
		this._columns.setTileGeometryData(tileWidthTwips, tileSizePixels, updatePositions);
		this._rows.setTileGeometryData(tileHeightTwips, tileSizePixels, updatePositions);

		if (app.map) app.map.fire('sheetgeometrychanged');
	}

	public setViewArea(topLeftTwipsPoint: Point, sizeTwips: Point): boolean {

		if (!(topLeftTwipsPoint instanceof cool.Point) || !(sizeTwips instanceof cool.Point)) {
			console.error('invalid argument types');
			return false;
		}

		var left = topLeftTwipsPoint.x;
		var top = topLeftTwipsPoint.y;
		var right = left + sizeTwips.x;
		var bottom = top + sizeTwips.y;

		this._columns.setViewLimits(left, right);
		this._rows.setViewLimits(top, bottom);

		return true;
	}

	/**
	 * Find which will be new first visible row after updating the sheet geometry
	 */
	public getFirstNewVisibleRow(): number {
		// Find first the visible rows in current view (before new sheet geometry is applied)
		const target: number = this.getViewRowRange().start;
		const invisibleRows: BoolSpanList = this.getRowsGeometry().getInvisibleSpanList(); //(hidden rows list in new geometry)

		if (target === 0)
			return 0;

		let start: number = 0;
		let end: number = invisibleRows._spanlist.length - 1;
		let ans: number = -1;

		// Just one span means every row is visible
		if (start === 0 && end === 0)
			return target;

		// Modified binary search to find next visible of after the target row
		while (start <= end) {
			const mid = Math.floor((start + end) / 2);
			if (invisibleRows._spanlist[mid] <= target) {
				start = mid + 1;
			} else {
				ans = mid;
				end = mid - 1;
			}
		}

		// InvisibleRows is list of ranges in form of
		// Like: 5 8 10 20... -> Means that visible up  to 5 (including), hidden up to 8 (including), visible up to 10 (including), hidden up to 20 (including)...
		if (ans !== -1) {
			if (ans === 0)
				return 0;
			// If its hidden index then +1 will be visible
			if (ans % 2 === 0 && (ans + 1 < invisibleRows._spanlist.length)) {
				return invisibleRows._spanlist[ans + 1];
			} else {
				return invisibleRows._spanlist[ans];
			}
		}
		return 0;
	}

	public getPart(): number {
		return this._part;
	}

	public getColumnsGeometry(): SheetDimension {
		return this._columns;
	}

	public getRowsGeometry(): SheetDimension {
		return this._rows;
	}

	// returns an object with keys 'start' and 'end' indicating the
	// column range in the current view area.
	public getViewColumnRange(): DimensionRange {
		return this._columns.getViewElementRange();
	}

	// returns an object with keys 'start' and 'end' indicating the
	// row range in the current view area.
	public getViewRowRange(): DimensionRange {
		return this._rows.getViewElementRange();
	}

	public getViewCellRange(): CellRange {
		return {
			columnrange: this.getViewColumnRange(),
			rowrange: this.getViewRowRange()
		};
	}

	// Returns an object with the following fields:
	// rowIndex should be zero based.
	// 'startpos' (start position of the row in core pixels), 'size' (row size in core pixels).
	// Note: All these fields are computed by assuming zero sizes for hidden/filtered rows.
	public getRowData(rowIndex: number): DimensionPosSize {
		return this._rows.getElementData(rowIndex);
	}

	public getColumnGroupLevels(): number {
		return this._columns.getGroupLevels();
	}

	public getRowGroupLevels(): number {
		return this._rows.getGroupLevels();
	}

	public getColumnGroupsDataInView(): GroupData[] {
		return this._columns.getGroupsDataInView();
	}

	public getRowGroupsDataInView(): GroupData[] {
		return this._rows.getGroupsDataInView();
	}

	// accepts a point in display twips coordinates at current zoom
	// and returns the equivalent point in display-twips at the given zoom.
	public getTileTwipsAtZoom(point: Point, zoomScale: number): Point {
		if (!(point instanceof cool.Point)) {
			console.error('Bad argument type, expected cool.Point');
			return point;
		}

		return new cool.Point(this._columns.getTileTwipsAtZoom(point.x, zoomScale),
			this._rows.getTileTwipsAtZoom(point.y, zoomScale));
	}

	// accepts a point in core-pixel coordinates at current zoom
	// and returns the equivalent point in core-pixels at the given zoomScale.
	public getCorePixelsAtZoom(point: Point, zoomScale: number): Point {
		if (!(point instanceof cool.Point)) {
			console.error('Bad argument type, expected cool.Point');
			return point;
		}

		return new cool.Point(this._columns.getCorePixelsAtZoom(point.x, zoomScale),
			this._rows.getCorePixelsAtZoom(point.y, zoomScale));
	}

	// accepts a point in core-pixel coordinates at *given* zoomScale
	// and returns the equivalent point in core-pixels at the current zoom.
	public getCorePixelsFromZoom(point: Point, zoomScale: number): Point {
		if (!(point instanceof cool.Point)) {
			console.error('Bad argument type, expected cool.Point');
			return point;
		}

		return new cool.Point(this._columns.getCorePixelsFromZoom(point.x, zoomScale),
			this._rows.getCorePixelsFromZoom(point.y, zoomScale));
	}

	// accepts a point in print twips coordinates and returns the equivalent point
	// in tile-twips.
	public getTileTwipsPointFromPrint(point: Point): Point {
		if (!(point instanceof cool.Point)) {
			console.error('Bad argument type, expected cool.Point');
			return point;
		}

		return new cool.Point(this._columns.getTileTwipsPosFromPrint(point.x),
			this._rows.getTileTwipsPosFromPrint(point.y));
	}

	public convertToTileTwips(simplePoint: cool.SimplePoint): void {
		simplePoint.x = this._columns.getTileTwipsPosFromPrint(simplePoint.x);
		simplePoint.y = this._rows.getTileTwipsPosFromPrint(simplePoint.y);
	}

	public convertRectangleToTileTwips(simpleRectangle: cool.SimpleRectangle): void {
		// getTileTwipsPosFromPrint expects an absolute position from origin,
		// so convert each edge independently and derive width/height from
		// the converted edges.
		const x1PT = simpleRectangle.x1;
		const y1PT = simpleRectangle.y1;
		const x2PT = x1PT + simpleRectangle.width;
		const y2PT = y1PT + simpleRectangle.height;

		const x1TT = this._columns.getTileTwipsPosFromPrint(x1PT);
		const y1TT = this._rows.getTileTwipsPosFromPrint(y1PT);
		const x2TT = this._columns.getTileTwipsPosFromPrint(x2PT);
		const y2TT = this._rows.getTileTwipsPosFromPrint(y2PT);

		simpleRectangle.x1 = x1TT;
		simpleRectangle.y1 = y1TT;
		simpleRectangle.width = x2TT - x1TT;
		simpleRectangle.height = y2TT - y1TT;
	}

	public convertRawRectangleToTileTwips(rectangle: number[]): void {
		// getTileTwipsPosFromPrint expects an absolute position from origin,
		// so convert each edge independently and derive width/height from
		// the converted edges.
		const x1PT = rectangle[0];
		const y1PT = rectangle[1];
		const x2PT = x1PT + rectangle[2];
		const y2PT = y1PT + rectangle[3];

		const x1TT = this._columns.getTileTwipsPosFromPrint(x1PT);
		const y1TT = this._rows.getTileTwipsPosFromPrint(y1PT);
		const x2TT = this._columns.getTileTwipsPosFromPrint(x2PT);
		const y2TT = this._rows.getTileTwipsPosFromPrint(y2PT);

		rectangle[0] = x1TT;
		rectangle[1] = y1TT;
		rectangle[2] = x2TT - x1TT;
		rectangle[3] = y2TT - y1TT;
	}

	// accepts a point in tile-twips coordinates and returns the equivalent point
	// in print-twips.
	public getPrintTwipsPointFromTile(point: Point): Point {
		if (!(point instanceof cool.Point)) {
			console.warn('Bad argument type, expected cool.Point');
		}

		return new cool.Point(this._columns.getPrintTwipsPosFromTile(point.x),
			this._rows.getPrintTwipsPosFromTile(point.y));
	}

	// accepts a rectangle in print twips coordinates and returns the equivalent rectangle
	// in tile-twips aligned to the cells.
	public getTileTwipsSheetAreaFromPrint(rectangle: Bounds): Bounds {
		if (!(rectangle instanceof cool.Bounds)) {
			console.error('Bad argument type, expected cool.Bounds');
			return rectangle;
		}

		var topLeft = rectangle.getTopLeft();
		var bottomRight = rectangle.getBottomRight();

		var horizBounds = this._columns.getTileTwipsRangeFromPrint(topLeft.x, bottomRight.x);
		var vertBounds = this._rows.getTileTwipsRangeFromPrint(topLeft.y, bottomRight.y);

		topLeft = new cool.Point(horizBounds.startpos, vertBounds.startpos);
		bottomRight = new cool.Point(horizBounds.endpos, vertBounds.endpos);

		return new cool.Bounds(topLeft, bottomRight);
	}

	// Returns full sheet size as cool.Point in the given unit.
	// unit must be one of 'corepixels', 'tiletwips', 'printtwips'
	public getSize(unit: GeometryUnit): Point {
		return new cool.Point(this._columns.getSize(unit),
			this._rows.getSize(unit));
	}

	// Returns the cell rectangle as SimpleRectangle in tile twips.
	public getCellSimpleRectangle(columnIndex: number, rowIndex: number): cool.SimpleRectangle {
		const horizPosSize = this._columns.getElementData(columnIndex);
		const vertPosSize = this._rows.getElementData(rowIndex);
		return cool.SimpleRectangle.fromCorePixels([horizPosSize.startpos, vertPosSize.startpos, horizPosSize.size, vertPosSize.size]);
	}

	// Returns the core pixel position/size of the requested cell at a specified zoom.
	public getCellRect(columnIndex: number, rowIndex: number, zoomScale: number): Bounds {
		var horizPosSize = this._columns.getElementData(columnIndex, zoomScale);
		var vertPosSize = this._rows.getElementData(rowIndex, zoomScale);

		var topLeft = new cool.Point(horizPosSize.startpos, vertPosSize.startpos);
		var size = new cool.Point(horizPosSize.size, vertPosSize.size);

		return new cool.Bounds(topLeft, topLeft.add(size));
	}

	public getCellFromPos(pos: Point, unit: GeometryUnit): Point {
		console.assert(pos instanceof cool.Point);
		return new cool.Point(
			this._columns.getIndexFromPos(pos.x, unit),
			this._rows.getIndexFromPos(pos.y, unit)
		);
	}

	// Returns the start position of the column containing posX in the specified unit.
	// unit must be one of 'corepixels', 'tiletwips', 'printtwips'
	public getSnapDocPosX(posX: number, unit: GeometryUnit): number {
		return this._columns.getSnapPos(posX, unit);
	}

	// Returns the start position of the row containing posY in the specified unit.
	// unit must be one of 'corepixels', 'tiletwips', 'printtwips'
	public getSnapDocPosY(posY: number, unit: GeometryUnit): number {
		return this._rows.getSnapPos(posY, unit);
	}

	private _testValidity(sheetGeomJSON: SheetGeometryCoreData, checkCompleteness: boolean): boolean {

		if (!sheetGeomJSON.commandName) {
			console.error(this._unoCommand + ' response has no property named "commandName".');
			return false;
		}

		if (sheetGeomJSON.commandName !== this._unoCommand) {
			console.error('JSON response has wrong commandName: ' +
				sheetGeomJSON.commandName + ' expected: ' +
				this._unoCommand);
			return false;
		}

		if (typeof sheetGeomJSON.maxtiledcolumn !== 'string' ||
			!/^\d+$/.test(sheetGeomJSON.maxtiledcolumn)) {
			console.error('JSON is missing/unreadable maxtiledcolumn property');
			return false;
		}

		if (typeof sheetGeomJSON.maxtiledrow !== 'string' ||
			!/^\d+$/.test(sheetGeomJSON.maxtiledrow)) {
			console.error('JSON is missing/unreadable maxtiledrow property');
			return false;
		}

		if (checkCompleteness) {

			if (!sheetGeomJSON.rows || !sheetGeomJSON.columns) {

				console.error(this._unoCommand + ' response is incomplete.');
				return false;
			}

			if (typeof sheetGeomJSON.rows !== 'object' ||
				typeof sheetGeomJSON.columns !== 'object') {

				console.error(this._unoCommand + ' response has invalid rows/columns children.');
				return false;
			}

			var expectedFields = ['sizes', 'hidden', 'filtered'];
			for (var idx = 0; idx < expectedFields.length; idx++) {

				var fieldName = expectedFields[idx];
				var encodingForCols = SheetGeometry.getDimensionDataField(sheetGeomJSON.columns, fieldName);
				var encodingForRows = SheetGeometry.getDimensionDataField(sheetGeomJSON.rows, fieldName);

				// Don't accept empty string or any other types.
				if (typeof encodingForRows !== 'string' || !encodingForRows) {
					console.error(this._unoCommand + ' response has invalid value for rows.' +
						fieldName);
					return false;
				}

				// Don't accept empty string or any other types.
				if (typeof encodingForCols !== 'string' || !encodingForCols) {
					console.error(this._unoCommand + ' response has invalid value for columns.' +
						fieldName);
					return false;
				}
			}
		}

		return true;
	}

	private static getDimensionDataField(dimData: SheetDimensionCoreData, fieldName: string): string {
		switch (fieldName) {
		case 'sizes':
			return dimData.sizes;
		case 'hidden':
			return dimData.hidden;
		case 'filtered':
			return dimData.filtered;
		case 'groups':
			return dimData.groups;
		default:
			return undefined;
		}
	}
}

export interface GroupData {
	level: string;
	index: string;
	startPos: string;
	endPos: string;
	hidden: string;
}

// Used to represent/query geometry data about either rows or columns.
export class SheetDimension {
	private _sizes: SpanList;
	private _hidden: BoolSpanList;
	private _filtered: BoolSpanList;
	private _outlines: DimensionOutlines;
	private _visibleSizes: SpanList;
	private _invisibleSpanList: BoolSpanList;

	private _maxIndex: number;
	private _tileSizeTwips: number;
	private _tileSizePixels: number;
	private _coreZoomFactor: number;
	private _twipsPerCorePixel: number;

	private _viewStartIndex: number;
	private _viewEndIndex: number;

	constructor() {

		this._sizes = new SpanList();
		this._hidden = new BoolSpanList();
		this._filtered = new BoolSpanList();
		this._outlines = new DimensionOutlines();

		// This is used to store the span-list of sizes
		// with hidden/filtered elements set to zero size.
		// This needs to be updated whenever
		// this._sizes/this._hidden/this._filtered are modified.
		this._visibleSizes = undefined;
	}

	public getVisibleSizes(): SpanList {
		return this._visibleSizes;
	}

	public getInvisibleSpanList(): BoolSpanList { return this._invisibleSpanList; }

	public update(jsonObject: SheetDimensionCoreData): boolean {

		if (typeof jsonObject !== 'object') {
			return false;
		}

		var regenerateVisibleSizes = false;
		var loadsOK = true;
		if (jsonObject.sizes !== undefined) {
			loadsOK = this._sizes.load(jsonObject.sizes);
			regenerateVisibleSizes = true;
		}

		if (jsonObject.hidden !== undefined) {
			var thisLoadOK = this._hidden.load(jsonObject.hidden);
			loadsOK = loadsOK && thisLoadOK;
			regenerateVisibleSizes = true;
		}

		if (jsonObject.filtered !== undefined) {
			thisLoadOK = this._filtered.load(jsonObject.filtered);
			loadsOK = loadsOK && thisLoadOK;
			regenerateVisibleSizes = true;
		}

		if (jsonObject.groups !== undefined) {
			thisLoadOK = this._outlines.load(jsonObject.groups);
			loadsOK = loadsOK && thisLoadOK;
		}

		if (loadsOK && regenerateVisibleSizes) {
			this._updateVisible();
		}

		return loadsOK;
	}

	public setMaxIndex(maxIndex: number): void {
		this._maxIndex = maxIndex;
	}

	public setTileGeometryData(tileSizeTwips: number, tileSizePixels: number, updatePositions: boolean = true): void {

		// Avoid position re-computations if no change in Zoom/dpiScale.
		if (this._tileSizeTwips === tileSizeTwips &&
			this._tileSizePixels === tileSizePixels) {
			return;
		}

		this._tileSizeTwips = tileSizeTwips;
		this._tileSizePixels = tileSizePixels;

		// number of core-pixels in the tile is the same as the number of device pixels used to render the tile.
		this._coreZoomFactor = this._tileSizePixels * 15.0 / this._tileSizeTwips;
		this._twipsPerCorePixel = this._tileSizeTwips / this._tileSizePixels;

		if (updatePositions) {
			// We need to compute positions data for every zoom change.
			this._updatePositions();
		}
	}

	private _updateVisible() {

		this._invisibleSpanList = this._hidden.union(this._filtered); // this._hidden is not modified.
		this._visibleSizes = this._sizes.applyZeroValues(this._invisibleSpanList); // this._sizes is not modified.
		this._updatePositions();
	}

	private _updatePositions() {

		var posCorePx = 0; // position in core pixels.
		var posPrintTwips = 0;
		this._visibleSizes.addCustomDataForEachSpan(function (
			index: number,
			size: number, /* size in twips of one element in the span */
			spanLength: number /* #elements in the span */) {

			// Important: rounding needs to be done in core pixels to match core.
			var sizeCorePxOne = Math.floor(size / this._twipsPerCorePixel);
			posCorePx += (sizeCorePxOne * spanLength);
			// position in core-pixel aligned twips.
			var posTileTwips = Math.floor(posCorePx * this._twipsPerCorePixel);
			posPrintTwips += (size * spanLength);

			var customData = {
				sizecore: sizeCorePxOne,
				poscorepx: posCorePx,
				postiletwips: posTileTwips,
				posprinttwips: posPrintTwips
			};

			return customData;
		}.bind(this));
	}

	// returns the element pos/size in core pixels by default.
	public getElementData(index: number, zoomScale?: number): DimensionPosSize {
		if (zoomScale !== undefined) {
			var startpos = 0;
			var size = 0;
			this._visibleSizes.forEachSpanInRange(0, index, function (spanData) {
				var count = spanData.end - spanData.start + 1;
				var sizeOneCorePx = Math.floor(spanData.size * zoomScale / 15.0);
				if (index > spanData.end) {
					startpos += (sizeOneCorePx * count);
				}
				else if (index >= spanData.start && index <= spanData.end) {
					// final span
					startpos += (sizeOneCorePx * (index - spanData.start));
					size = sizeOneCorePx;
				}
			});

			return {
				startpos: startpos,
				size: size
			};
		}

		var span = this._visibleSizes.getSpanDataByIndex(index);
		if (span === undefined) {
			return undefined;
		}

		return this._getElementDataFromSpanByIndex(index, span);
	}

	public getElementDataAny(index: number, unitName: GeometryUnit): DimensionPosSize {
		var span = this._visibleSizes.getSpanDataByIndex(index);
		if (span === undefined) {
			return undefined;
		}

		return this._getElementDataAnyFromSpanByIndex(index, span, unitName);
	}

	// returns element pos/size in core pixels by default.
	private _getElementDataFromSpanByIndex(index: number, span: any): DimensionPosSize {
		return this._getElementDataAnyFromSpanByIndex(index, span, 'corepixels');
	}

	// returns element pos/size in the requested unit.
	private _getElementDataAnyFromSpanByIndex(index: number, span: any, unitName: GeometryUnit): DimensionPosSize {

		if (span === undefined || index < span.start || span.end < index) {
			return undefined;
		}

		if (unitName !== 'corepixels' &&
			unitName !== 'tiletwips' && unitName !== 'printtwips') {
			console.error('unsupported unitName: ' + unitName);
			return undefined;
		}

		var numSizes = span.end - index + 1;
		var inPixels = unitName === 'corepixels';
		if (inPixels) {
			return {
				startpos: (span.data.poscorepx - span.data.sizecore * numSizes),
				size: span.data.sizecore
			};
		}

		if (unitName === 'printtwips') {
			return {
				startpos: (span.data.posprinttwips - span.size * numSizes),
				size: span.size
			};
		}

		// unitName is 'tiletwips'
		// It is very important to calculate this from core pixel units to mirror the core calculations.
		var twipsPerCorePixel = this._twipsPerCorePixel;
		return {
			startpos: Math.floor(
				(span.data.poscorepx - span.data.sizecore * numSizes) * twipsPerCorePixel),
			size: Math.floor(span.data.sizecore * twipsPerCorePixel)
		};
	}

	public forEachInRange(start: number, end: number, callback: ((dimIndex: number, posSize: DimensionPosSize) => void)): void {

		this._visibleSizes.forEachSpanInRange(start, end, function (span: SpanViewData) {
			var first = Math.max(span.start, start);
			var last = Math.min(span.end, end);
			for (var index = first; index <= last; ++index) {
				callback(index, this._getElementDataFromSpanByIndex(index, span));
			}
		}.bind(this));
	}

	// callback with a position and index for each grid line in this pixel range
	public forEachInCorePixelRange(startPix: number, endPix: number, callback: ((startPosCorePx: number, index: number) => void)): void {
		this._visibleSizes.forEachSpan(function (spanData: any) {
			// do we overlap ?
			var spanFirstCorePx = spanData.data.poscorepx -
				(spanData.data.sizecore * (spanData.end - spanData.start + 1));
			if (spanFirstCorePx < endPix && spanData.data.poscorepx > startPix) {
				var firstCorePx = Math.max(
					spanFirstCorePx,
					startPix + spanData.data.sizecore -
					((startPix - spanFirstCorePx) % spanData.data.sizecore));
				var lastCorePx = Math.min(endPix, spanData.data.poscorepx);

				var index = spanData.start + Math.floor((firstCorePx - spanFirstCorePx) / spanData.data.sizecore);
				for (var pos = firstCorePx; pos <= lastCorePx; pos += spanData.data.sizecore) {
					callback(pos, index);
					index += 1;
				}
			}
		});
	}

	// computes element index from tile-twips position and returns
	// an object with this index and the span data.
	private _getSpanAndIndexFromTileTwipsPos(pos: number): IndexAndSpan {
		var result = {} as IndexAndSpan;
		var span = this._visibleSizes.getSpanDataByCustomDataField(pos, 'postiletwips');
		result.span = span;
		if (span === undefined) {
			// enforce limits.
			result.index = (pos >= 0) ? this._maxIndex : 0;
			result.span = this._visibleSizes.getSpanDataByIndex(result.index);
			return result;
		}
		var elementCount = span.end - span.start + 1;
		var posStart = ((span.data.poscorepx - span.data.sizecore * elementCount) * this._twipsPerCorePixel);
		var posEnd = span.data.postiletwips;
		var sizeOne = (posEnd - posStart) / elementCount;

		// always round down as relativeIndex is zero-based.
		var relativeIndex = Math.floor((pos - posStart) / sizeOne);

		result.index = span.start + relativeIndex;
		return result;
	}

	// computes element index from tile-twips position.
	private _getIndexFromTileTwipsPos(pos: number): number {
		return this._getSpanAndIndexFromTileTwipsPos(pos).index;
	}

	// computes element index from print twips position and returns
	// an object with this index and the span data.
	private _getSpanAndIndexFromPrintTwipsPos(pos: number): IndexAndSpan {
		var result = {} as IndexAndSpan;
		var span = this._visibleSizes.getSpanDataByCustomDataField(pos, 'posprinttwips');
		result.span = span;
		if (span === undefined) {
			// enforce limits.
			result.index = (pos >= 0) ? this._maxIndex : 0;
			result.span = this._visibleSizes.getSpanDataByIndex(result.index);
			return result;
		}
		var elementCount = span.end - span.start + 1;
		var posStart = (span.data.posprinttwips - span.size * elementCount);
		var sizeOne = span.size;

		// always round down as relativeIndex is zero-based.
		var relativeIndex = Math.floor((pos - posStart) / sizeOne);

		result.index = span.start + relativeIndex;
		return result;
	}

	public setViewLimits(startPosTileTwips: number, endPosTileTwips: number): void {

		this._viewStartIndex = Math.max(0, this._getIndexFromTileTwipsPos(startPosTileTwips));
		this._viewEndIndex = Math.min(this._maxIndex, this._getIndexFromTileTwipsPos(endPosTileTwips));
	}

	public getViewElementRange(): DimensionRange {
		return {
			start: this._viewStartIndex,
			end: this._viewEndIndex
		};
	}

	public getGroupLevels(): number {
		return this._outlines.getLevels();
	}

	public getGroupsDataInView(): GroupData[] {
		var groupsData: GroupData[] = [];
		var levels = this._outlines.getLevels();
		if (!levels) {
			return groupsData;
		}

		this._outlines.forEachGroupInRange(0, this._viewEndIndex,
			function (levelIdx: number, groupIdx: number, start: number, end: number, hidden: number) {

				var startElementData = this.getElementData(start);
				var endElementData = this.getElementData(end);
				groupsData.push({
					level: (levelIdx + 1).toString(),
					index: groupIdx.toString(),
					startPos: startElementData.startpos.toString(),
					endPos: (endElementData.startpos + endElementData.size).toString(),
					hidden: hidden ? '1' : '0'
				});
			}.bind(this));

		return groupsData;
	}

	public getMaxIndex(): number {
		return this._maxIndex;
	}

	// Accepts a position in display twips at current zoom and returns corresponding
	// display twips position at the given zoomScale.
	public getTileTwipsAtZoom(posTT: number, zoomScale: number): number {
		if (typeof posTT !== 'number' || typeof zoomScale !== 'number') {
			console.error('Wrong argument types');
			return;
		}

		var posPT = this.getPrintTwipsPosFromTile(posTT);
		return this.getTileTwipsPosFromPrint(posPT, zoomScale);
	}

	// Accepts a position in core-pixels at current zoom and returns corresponding
	// core-pixels position at the given zoomScale.
	public getCorePixelsAtZoom(posCP: number, zoomScale: number): number {
		if (typeof posCP !== 'number' || typeof zoomScale !== 'number') {
			console.error('Wrong argument types');
			return;
		}

		var posCPZ = 0; // Position in core-pixels at zoomScale.
		var posCPRem = posCP; // Unconverted core-pixels position at current zoom.
		this._visibleSizes.forEachSpan(function (span) {
			var elementCount = span.end - span.start + 1;
			var sizeOneCP = span.data.sizecore;
			var sizeOneCPZ = Math.floor(span.size / 15.0 * zoomScale);
			var sizeCP = sizeOneCP * elementCount;
			var sizeCPZ = sizeOneCPZ * elementCount;

			if (posCPRem < sizeOneCP) {
				// Done converting. FIXME: make this callback return false to end the forEachSpan when done.
				return;
			}

			if (posCPRem >= sizeCP) {
				// Whole span can be converted.
				posCPRem -= sizeCP;
				posCPZ += sizeCPZ;
				return;
			}

			// Only part of the span can be converted.
			// sizeOneCP <= posCPRem < sizeCP.
			var elems = Math.floor(posCPRem / sizeOneCP);
			posCPRem -= (elems * sizeOneCP);
			posCPZ += (elems * sizeOneCPZ);
		});

		return posCPZ + (posCPRem * zoomScale / this._coreZoomFactor);
	}

	// Accepts a position in core-pixels at *given* zoomScale and returns corresponding
	// core-pixels position at the current zoom.
	public getCorePixelsFromZoom(posCPZ: number, zoomScale: number): number {
		if (typeof posCPZ !== 'number' || typeof zoomScale !== 'number') {
			console.error('Wrong argument types');
			return;
		}

		var posCP = 0; // Position in core-pixels at current zoom.
		var posCPZRem = posCPZ; // Unconverted core-pixels position at zoomScale.
		this._visibleSizes.forEachSpan(function (span) {
			var elementCount = span.end - span.start + 1;
			var sizeOneCP = span.data.sizecore;
			var sizeOneCPZ = Math.floor(span.size / 15.0 * zoomScale);
			var sizeCP = sizeOneCP * elementCount;
			var sizeCPZ = sizeOneCPZ * elementCount;

			if (posCPZRem < sizeOneCPZ) {
				// Done converting.
				return;
			}

			if (posCPZRem >= sizeCPZ) {
				// Whole span can be converted.
				posCPZRem -= sizeCPZ;
				posCP += sizeCP;
				return;
			}

			// Only part of the span can be converted.
			// sizeOneCPZ <= posCPZRem < sizeCPZ.
			var elems = Math.floor(posCPZRem / sizeOneCPZ);
			posCPZRem -= (elems * sizeOneCPZ);
			posCP += (elems * sizeOneCP);
		});

		return posCP + (posCPZRem * this._coreZoomFactor / zoomScale);
	}

	// Accepts a position in print twips and returns the corresponding position in tile twips.
	public getTileTwipsPosFromPrint(posPT: number, zoomScale?: number): number {

		if (typeof posPT !== 'number') {
			console.error('Wrong argument type');
			return;
		}

		if (typeof zoomScale === 'number') {
			var posTT = 0;
			var posPTInc = 0;
			this._visibleSizes.forEachSpan(function (spanData) {
				var count = spanData.end - spanData.start + 1;
				var sizeSpanPT = spanData.size * count;
				var sizeOneCorePx = Math.floor(spanData.size * zoomScale / 15.0);
				var sizeSpanTT = Math.floor(sizeOneCorePx * count * 15 / zoomScale);

				if (posPTInc >= posPT) {
					return;
				}

				if (posPTInc + sizeSpanPT < posPT) {
					// add whole span.
					posPTInc += sizeSpanPT;
					posTT += sizeSpanTT;
					return;
				}

				// final span
				var remainingPT = posPT - posPTInc;
				var elemCountFinalSpan = Math.floor(remainingPT / spanData.size);
				var extra = remainingPT - (elemCountFinalSpan * spanData.size);
				posTT += (Math.floor(elemCountFinalSpan * sizeSpanTT / count) + extra);
				posPTInc = posPT;
			});

			return posTT;
		}

		var element = this._getSpanAndIndexFromPrintTwipsPos(posPT);
		var elementDataTT = this._getElementDataAnyFromSpanByIndex(element.index, element.span, 'tiletwips');
		var elementDataPT = this._getElementDataAnyFromSpanByIndex(element.index, element.span, 'printtwips');

		var offset = posPT - elementDataPT.startpos;
		console.assert(offset >= 0, 'offset should not be negative');

		// Preserve any offset from the matching column/row start position.
		return elementDataTT.startpos + offset;
	}

	// Accepts a position in tile twips and returns the corresponding position in print twips.
	public getPrintTwipsPosFromTile(posTT: number): number {

		if (typeof posTT !== 'number') {
			console.error('Wrong argument type');
			return;
		}

		var element = this._getSpanAndIndexFromTileTwipsPos(posTT);
		var elementDataTT = this._getElementDataAnyFromSpanByIndex(element.index, element.span, 'tiletwips');
		var elementDataPT = this._getElementDataAnyFromSpanByIndex(element.index, element.span, 'printtwips');

		var offset = posTT - elementDataTT.startpos;
		console.assert(offset >= 0, 'offset should not be negative');

		// Preserve any offset from the matching column/row start position.
		return elementDataPT.startpos + offset;
	}

	// Accepts a start and end positions in print twips, and returns the
	// corresponding positions in tile twips, by first computing the element range.
	public getTileTwipsRangeFromPrint(posStartPT: number, posEndPT: number): DimensionStartEndPos {
		var startElement = this._getSpanAndIndexFromPrintTwipsPos(posStartPT);
		var startData = this._getElementDataAnyFromSpanByIndex(startElement.index, startElement.span, 'tiletwips');
		if (posStartPT === posEndPT) {
			// range is hidden, send a minimal sized tile-twips range.
			// Set the size = twips equivalent of 1 core pixel,
			// to imitate what core does when it sends cursor/ranges in tile-twips coordinates.
			var rangeSize = Math.floor(this._twipsPerCorePixel);
			return {
				startpos: startData.startpos,
				endpos: startData.startpos + rangeSize
			};
		}
		var endElement = this._getSpanAndIndexFromPrintTwipsPos(posEndPT);
		var endData = this._getElementDataAnyFromSpanByIndex(endElement.index, endElement.span, 'tiletwips');

		var startPos = startData.startpos;
		var endPos = endData.startpos + endData.size;
		if (endPos < startPos) {
			endPos = startPos;
		}

		return {
			startpos: startPos,
			endpos: endPos
		};
	}

	public getSize(unit: GeometryUnit): number {
		var posSize = this.getElementDataAny(this._maxIndex, unit);
		if (!posSize) {
			return undefined;
		}

		return posSize.startpos + posSize.size;
	}

	public isUnitSupported(unitName: string): boolean {
		return (
			unitName === 'corepixels' ||
			unitName === 'tiletwips' ||
			unitName === 'printtwips'
		);
	}

	public getSnapPos(pos: number, unit: GeometryUnit): number {
		console.assert(typeof pos === 'number', 'pos is not a number');
		console.assert(this.isUnitSupported(unit), 'unit: ' + unit + ' is not supported');

		var origUnit = unit;

		if (unit === 'corepixels') {
			pos = pos * this._twipsPerCorePixel;
			unit = 'tiletwips';
		}

		console.assert(unit === 'tiletwips' || unit === 'printtwips', 'wrong unit assumption');
		var result = (unit === 'tiletwips') ?
			this._getSpanAndIndexFromTileTwipsPos(pos) :
			this._getSpanAndIndexFromPrintTwipsPos(pos);

		return this._getElementDataAnyFromSpanByIndex(result.index, result.span, origUnit).startpos;
	}

	public getIndexFromPos(pos: number, unit: GeometryUnit): number {
		console.assert(typeof pos === 'number', 'pos is not a number');
		console.assert(this.isUnitSupported(unit), 'unit: ' + unit + ' is not supported');

		if (unit === 'corepixels') {
			pos = pos * this._twipsPerCorePixel;
			unit = 'tiletwips';
		}

		console.assert(unit === 'tiletwips' || unit === 'printtwips', 'wrong unit assumption');
		var result = (unit === 'tiletwips') ?
			this._getSpanAndIndexFromTileTwipsPos(pos) :
			this._getSpanAndIndexFromPrintTwipsPos(pos);

		return result.index;
	}

}

export interface SpanData {
	index: number;
	value: number;
	data?: any;
}

interface SpanViewData extends SpanData {
	start: number;
	end: number;
	size: number;
}

interface ParsedSpan {
	index: number;
	value: number;
}

type BoolSpanData = number;

type ParsedBoolSpan = number;

interface ParsedSpanList {
	spanlist: ParsedSpan[];
}

interface ParsedBoolSpanList {
	spanlist: ParsedBoolSpan[];
	startBit: boolean;
}

class SpanList {

	public _spanlist: SpanData[];

	constructor(encoding?: string) {

		// spans are objects with keys: 'index' and 'value'.
		// 'index' holds the last element of the span.
		// Optionally custom data of a span can be added
		// under the key 'data' via addCustomDataForEachSpan.
		this._spanlist = [];
		if (typeof encoding !== 'string') {
			return;
		}

		this.load(encoding);
	}

	public load(encoding: string): boolean {

		if (typeof encoding !== 'string') {
			return false;
		}

		var result = <ParsedSpanList>parseSpanListEncoding(encoding, false /* boolean value ? */);
		if (result === undefined) {
			return false;
		}

		this._spanlist = result.spanlist;
		return true;
	}

	// Runs in O(#spans in 'this' + #spans in 'other')
	public applyZeroValues(other: BoolSpanList) {

		if (!(other instanceof BoolSpanList)) {
			return undefined;
		}

		// Ensure both spanlists have the same total range.
		if (this._spanlist[this._spanlist.length - 1].index !== other._spanlist[other._spanlist.length - 1]) {
			return undefined;
		}

		var maxElement = this._spanlist[this._spanlist.length - 1].index;
		var result = new SpanList();

		var thisIdx = 0;
		var otherIdx = 0;
		var zeroBit = other._startBit;
		var resultValue = zeroBit ? 0 : this._spanlist[thisIdx].value;

		while (thisIdx < this._spanlist.length && otherIdx < other._spanlist.length) {

			// end elements of the current spans of 'this' and 'other'.
			var thisElement = this._spanlist[thisIdx].index;
			var otherElement = other._spanlist[otherIdx];

			var lastElement = otherElement;
			if (thisElement < otherElement) {
				lastElement = thisElement;
				++thisIdx;
			}
			else if (otherElement < thisElement) {
				zeroBit = !zeroBit;
				++otherIdx;
			}
			else { // both elements are equal.
				zeroBit = !zeroBit;
				++thisIdx;
				++otherIdx;
			}

			var nextResultValue = resultValue;
			if (thisIdx < this._spanlist.length) {
				nextResultValue = zeroBit ? 0 : this._spanlist[thisIdx].value;
			}

			if (resultValue != nextResultValue || lastElement >= maxElement) {
				// In the result spanlist a new span start from lastElement+1
				// or reached the maximum possible element.
				result._spanlist.push({ index: lastElement, value: resultValue });
				resultValue = nextResultValue;
			}
		}

		return result;
	}

	public addCustomDataForEachSpan(getCustomDataCallback: (index: number, value: number, count: number) => any) {

		if (typeof getCustomDataCallback != 'function') {
			return;
		}

		var prevIndex = -1;
		this._spanlist.forEach(function (span) {
			span.data = getCustomDataCallback(
				span.index, span.value,
				span.index - prevIndex);
			prevIndex = span.index;
		});
	}

	public getSpanDataByIndex(index: number): SpanData {

		if (typeof index != 'number') {
			return undefined;
		}

		var spanid = this._searchByIndex(index);
		if (spanid == -1) {
			return undefined;
		}

		return this._getSpanData(spanid);
	}

	public getSpanDataByCustomDataField(value: number, fieldName: string): SpanViewData {

		if (typeof value != 'number' || typeof fieldName != 'string' || !fieldName) {
			return undefined;
		}

		var spanid = this._searchByCustomDataField(value, fieldName);
		if (spanid == -1) {
			return undefined;
		}

		return this._getSpanData(spanid);
	}

	public forEachSpanInRange(start: number, end: number, callback: ((span: SpanViewData) => void)) {

		if (typeof start != 'number' || typeof end != 'number' ||
			typeof callback != 'function' || start > end) {
			return;
		}

		var id = this._searchByIndex(start);
		if (id == -1)
			return;
		do {
			var span = this._getSpanData(id++);
			callback(span);
		} while (id < this._spanlist.length && span.index <= end);
	}

	public forEachSpan(callback: ((span: SpanViewData) => void)) {
		for (var id = 0; id < this._spanlist.length; ++id) {
			callback(this._getSpanData(id));
		}
	}

	private _getSpanData(spanid: number): SpanViewData {
		// TODO: Check if data is changed by the callers. If not, return the pointer instead.
		var span = this._spanlist[spanid];
		var clone: SpanViewData = {
			index: span.index,
			value: span.value,
			data: span.data,

			start: spanid > 0 ? this._spanlist[spanid - 1].index + 1: 0,
			end: span.index,
			size: span.value
		};

		return clone;
	}

	private _searchByIndex(index: number): number {

		return binarySearch(this._spanlist, index,
			function directionProvider(testIndex: number, prevSpan: SpanData, curSpan: SpanData) {
				var spanStart = prevSpan ?
					prevSpan.index + 1 : 0;
				var spanEnd = curSpan.index;
				return (testIndex < spanStart) ? -1 :
					(spanEnd < testIndex) ? 1 : 0;
			});
	}

	public _searchByCustomDataField(value: number, fieldName: string): number {

		// All custom searchable data values are assumed to start
		// from 0 at the start of first span and are in non-decreasing order.

		return binarySearch(this._spanlist, value,
			function directionProvider(testValue: number, prevSpan: SpanData, curSpan: SpanData, nextSpan: SpanData) {
				var valueStart = prevSpan ?
					prevSpan.data[fieldName] : 0;
				var valueEnd = curSpan.data[fieldName] - (nextSpan ? 1 : 0);
				if (valueStart === undefined || valueEnd === undefined) {
					// fieldName not present in the 'data' property.
					return -1;
				}
				return (testValue < valueStart) ? -1 :
					(valueEnd < testValue) ? 1 : 0;
			}, true /* find the first match in case of duplicates */);
		// About the last argument: duplicates can happen, for instance if the
		// custom field represents positions, and there are spans with zero sizes (hidden/filtered).
	}


}

class BoolSpanList {

	public _startBit: boolean;
	public _spanlist: BoolSpanData[];

	constructor(encoding?: string) {

		// list of spans, each span represented by the end index.
		this._spanlist = [];
		this._startBit = false;
		if (typeof encoding !== 'string') {
			return;
		}

		this.load(encoding);
	}

	public load(encoding?: string): boolean {

		if (typeof encoding !== 'string') {
			return false;
		}

		var result = <ParsedBoolSpanList>parseSpanListEncoding(encoding, true /* boolean value ? */);
		if (result === undefined) {
			return false;
		}

		this._spanlist = result.spanlist;
		this._startBit = result.startBit;
		return true;
	}

	// Runs in O(#spans in 'this' + #spans in 'other')
	public union(other: BoolSpanList) {

		if (!(other instanceof BoolSpanList)) {
			return undefined;
		}

		// Ensure both spanlists have the same total range.
		if (this._spanlist[this._spanlist.length - 1] !== other._spanlist[other._spanlist.length - 1]) {
			return undefined;
		}

		var maxElement = this._spanlist[this._spanlist.length - 1];

		var result = new BoolSpanList();
		var thisBit = this._startBit;
		var otherBit = other._startBit;
		var resultBit = thisBit || otherBit;
		result._startBit = resultBit;

		var thisIdx = 0;
		var otherIdx = 0;

		while (thisIdx < this._spanlist.length && otherIdx < other._spanlist.length) {

			// end elements of the current spans of 'this' and 'other'.
			var thisElement = this._spanlist[thisIdx];
			var otherElement = other._spanlist[otherIdx];

			var lastElement = otherElement;
			if (thisElement < otherElement) {
				lastElement = thisElement;
				thisBit = !thisBit;
				++thisIdx;
			}
			else if (otherElement < thisElement) {
				otherBit = !otherBit;
				++otherIdx;
			}
			else { // both elements are equal.
				thisBit = !thisBit;
				otherBit = !otherBit;
				++thisIdx;
				++otherIdx;
			}

			var nextResultBit = (thisBit || otherBit);
			if (resultBit != nextResultBit || lastElement >= maxElement) {
				// In the result spanlist a new span start from lastElement+1
				// or reached the maximum possible element.
				result._spanlist.push(lastElement);
				resultBit = nextResultBit;
			}
		}

		return result;
	}

}

function parseSpanListEncoding(encoding: string, booleanValue: boolean): (ParsedSpanList | ParsedBoolSpanList) {

	var spanlist: ParsedSpan[] = [];
	var boolspanlist: BoolSpanData[] = [];
	var splits = encoding.split(' ');
	if (splits.length < 2) {
		return undefined;
	}

	var startBitInt = 0;
	if (booleanValue) {
		var parts = splits[0].split(':');
		if (parts.length != 2) {
			return undefined;
		}
		startBitInt = parseInt(parts[0]);
		var first = parseInt(parts[1]);
		if (isNaN(startBitInt) || isNaN(first)) {
			return undefined;
		}
		boolspanlist.push(first);
	}

	var startBit = Boolean(startBitInt);

	for (var idx = 0; idx < splits.length - 1; ++idx) {

		if (booleanValue) {
			if (!idx) {
				continue;
			}

			var entry = parseInt(splits[idx]);
			if (isNaN(entry)) {
				return undefined;
			}

			boolspanlist.push(entry);
			continue;
		}

		var spanParts = splits[idx].split(':');
		if (spanParts.length != 2) {
			return undefined;
		}

		var span = {
			index: parseInt(spanParts[1]),
			value: parseInt(spanParts[0])
		};

		if (isNaN(span.index) || isNaN(span.value)) {
			return undefined;
		}

		spanlist.push(span);
	}

	if (booleanValue) {
		return {
			spanlist: boolspanlist,
			startBit: startBit
		};
	}

	return { spanlist: spanlist };
}

interface OutlineEntry {
	start: number;
	end: number;
	hidden: number;
	visible: number;
}

type OutlineCollection = OutlineEntry[];

export class DimensionOutlines {

	private _outlines: OutlineCollection[];

	constructor(encoding?: string) {

		this._outlines = [];
		if (typeof encoding !== 'string') {
			return;
		}

		this.load(encoding);
	}

	public load(encoding: string): boolean {

		if (typeof encoding !== 'string') {
			return false;
		}

		var levels = encoding.split(' ');
		if (levels.length < 2) {
			// No outline.
			this._outlines = [];
			return true;
		}

		var outlines: OutlineCollection[] = [];

		for (var levelIdx = 0; levelIdx < levels.length - 1; ++levelIdx) {
			var collectionSplits = levels[levelIdx].split(',');
			var collections = [];
			if (collectionSplits.length < 2) {
				return false;
			}

			for (var collIdx = 0; collIdx < collectionSplits.length - 1; ++collIdx) {
				var entrySplits = collectionSplits[collIdx].split(':');
				if (entrySplits.length < 4) {
					return false;
				}

				var olineEntry: OutlineEntry = {
					start: parseInt(entrySplits[0]),
					end: parseInt(entrySplits[1]), // this is size.
					hidden: parseInt(entrySplits[2]),
					visible: parseInt(entrySplits[3])
				};

				if (isNaN(olineEntry.start) || isNaN(olineEntry.end) ||
					isNaN(olineEntry.hidden) || isNaN(olineEntry.visible)) {
					return false;
				}

				// correct the 'end' attribute.
				olineEntry.end += (olineEntry.start - 1);

				collections.push(olineEntry);
			}

			outlines.push(collections);
		}

		this._outlines = outlines;
		return true;
	}

	public getLevels(): number {
		return this._outlines.length;
	}

	// Calls 'callback' for all groups in all levels that have an intersection with
	// the inclusive element range [start, end].
	// 'callback' is called with these parameters :
	// (levelIdx, groupIdx, groupStart, groupEnd, groupHidden).
	public forEachGroupInRange(start: number, end: number,
		callback: ((levelIndex: number, groupIndex: number, groupStart: number, groupEnd: number, groupHidden: number) => void)): void {

		if (typeof start != 'number' || typeof end != 'number' || typeof callback != 'function') {
			return;
		}

		if (!this._outlines.length || start > end) {
			return;
		}

		// Search direction provider for binarySearch().
		// Here we want to find the first group after or intersects elementIdx.
		// return value : 0 for match, -1 for "try previous entries", +1 for "try next entries".
		var directionProvider = function (elementIdx: number, prevGroup: OutlineEntry, curGroup: OutlineEntry/*, nextGroup*/): SearchDirection {

			var direction: SearchDirection = (elementIdx < curGroup.start) ? -1 :
				(curGroup.end < elementIdx) ? 1 : 0;

			if (direction >= 0) {
				return direction;
			}

			// If curGroup is the first one, or elementidx is after prevGroup's end, then it is a match.
			if (!prevGroup || (prevGroup.end < elementIdx)) {
				return 0;
			}

			return -1;
		};

		for (var levelIdx = this._outlines.length - 1; levelIdx >= 0; --levelIdx) {

			var groupsInLevel = this._outlines[levelIdx];
			// Find the first group after or that intersects 'start'.
			var startGroupIdx = binarySearch(groupsInLevel, start, directionProvider);
			if (startGroupIdx == -1) {
				// All groups at this level are before 'start'.
				continue;
			}

			var startGroup = groupsInLevel[startGroupIdx];
			if (end < startGroup.start) {
				// No group at this level intersects the range [start, end].
				continue;
			}

			for (var groupIdx = startGroupIdx; groupIdx < groupsInLevel.length; ++groupIdx) {
				var group = groupsInLevel[groupIdx];
				if (end < group.start) {
					continue;
				}

				callback(levelIdx, groupIdx, group.start,
					group.end, group.hidden);
			}
		}
	}
}

// Does binary search on array for key, possibly using a custom direction provider.
// Of course, this assumes that the array is sorted (w.r.t to the semantics of
// the directionProvider when it is provided).
// It returns the index of the match if successful else returns -1.
// 'firstMatch' if true, some additional work is done to ensure that the index of
// the first match (from the 0 index of the array) is returned in case there are
// duplicates.
//
// directionProvider will be provided the following parameters :
// (key, previousArrayElement, currentArrayElement, nextArrayElement)
// previousArrayElement and nextArrayElement can be undefined when
// currentArrayElement is the first or the last element of the array
// respectively. This function should return:
//   0: for a match(to stop search),
//   1: to try searching upper half,
//  -1: to try searching lower half

type SearchDirection = 0 | 1 | -1;
type DirectionProviderType = ((key: any, previousElement: any, currentElement: any, nextElement?: any) => SearchDirection);

function binarySearch(array: any[], key: any, directionProvider: DirectionProviderType, firstMatch: boolean = false): number {

	if (!Array.isArray(array) || !array.length) {
		return -1;
	}

	if (typeof directionProvider != 'function') {
		directionProvider = function (key, prevvalue, testvalue) {
			return (key === testvalue) ? 0 :
				(key < testvalue) ? -1 : 1;
		};
	}

	firstMatch = (firstMatch === true);

	var start = 0;
	var end = array.length - 1;

	// Bound checks and early exit.
	var startDir = directionProvider(key, undefined, array[0], array[1]);
	if (startDir <= 0) {
		return startDir;
	}

	var endDir = directionProvider(key, array[end - 1], array[end]);
	if (endDir >= 0) {

		if (endDir === 1) {
			return -1;
		}

		return firstMatch ? _findFirstMatch(array, key, directionProvider, end) : end;
	}

	var mid = -1;
	while (start <= end) {
		mid = Math.round((start + end) / 2);
		var direction = directionProvider(key, array[mid - 1],
			array[mid], array[mid + 1]);

		if (direction == 0) {
			break;
		}

		if (direction == -1) {
			end = mid - 1;
		}
		else {
			start = mid + 1;
		}
	}

	return (start > end) ? -1 :
		firstMatch ? _findFirstMatch(array, key, directionProvider, mid) : mid;
}

// Helper function for binarySearch().
function _findFirstMatch(array: any[], key: any, directionProvider: DirectionProviderType, randomMatchingIndex: number) {

	if (randomMatchingIndex === 0) {
		return 0;
	}

	var index = randomMatchingIndex - 1;
	while (index >= 0 && directionProvider(key,
		array[index - 1], array[index], array[index + 1]) == 0) {
		--index;
	}

	return index + 1;
}

}
