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
 * Control.ColumnHeader
 */

/* global _UNO app */
namespace cool {

export class ColumnHeader extends Header {
	anchor: Array<Array<string>> = [[app.CSections.ColumnGroup.name, 'bottom', 'top'], [app.CSections.CornerHeader.name, 'right', 'left']];
	position: number[] = [0, 0]; // This section's myTopLeft is placed according to corner header and column group sections.
	size: number[] = [0, 19 * app.dpiScale]; // No initial width is necessary.
	expand: Array<string> = ['right']; // Expand horizontally.
	processingOrder: number = app.CSections.ColumnHeader.processingOrder;
	drawingOrder: number = app.CSections.ColumnHeader.drawingOrder;
	zIndex: number = app.CSections.ColumnHeader.zIndex;
	cursor: string = 'col-resize';

	_current: number;
	_selection: SelectionRange;

	constructor(cursor?: string) {
		super(app.CSections.ColumnHeader.name);

		if (cursor)
			this.cursor = cursor;
	}

	onInitialize(): void {
		this._map = window.L.Map.THIS;
		this._isColumn = true;
		this._current = -1;
		this.resizeHandleSize = 15 * app.dpiScale;
		this._selection = {start: -1, end: -1};
		this._mouseOverEntry = null;
		this._lastMouseOverIndex = undefined;
		this._hitResizeArea = false;
		this.sectionProperties.docLayer = this._map._docLayer;

		super.onInitialize();

		this._headerMenuID = 'col-header-menu';
		this._menuItem = {
			'.uno:InsertColumnsBefore': {
				name: app.IconUtil.createMenuItemLink(_UNO('.uno:InsertColumnsBefore', 'spreadsheet', true), 'InsertColumnsBefore'),
				isHtmlName: true,
				callback: (this._insertColBefore).bind(this)
			},
			'.uno:InsertColumnsAfter': {
				name: app.IconUtil.createMenuItemLink(_UNO('.uno:InsertColumnsAfter', 'spreadsheet', true), 'InsertColumnsAfter'),
				isHtmlName: true,
				callback: (this._insertColAfter).bind(this)
			},
			'.uno:DeleteColumns': {
				name: app.IconUtil.createMenuItemLink(_UNO('.uno:DeleteColumns', 'spreadsheet', true), 'DeleteColumns'),
				isHtmlName: true,
				callback: (this._deleteSelectedCol).bind(this)
			},
			'.uno:ColumnWidth': {
				name: app.IconUtil.createMenuItemLink(_UNO('.uno:ColumnWidth', 'spreadsheet', true), 'ColumnWidth'),
				isHtmlName: true,
				callback: (this._columnWidth).bind(this)
			},
			'.uno:SetOptimalColumnWidth': {
				name: app.IconUtil.createMenuItemLink(_UNO('.uno:SetOptimalColumnWidth', 'spreadsheet', true), 'SetOptimalColumnWidth'),
				isHtmlName: true,
				callback: (this._optimalWidth).bind(this)
			},
			'.uno:HideColumn': {
				name: app.IconUtil.createMenuItemLink(_UNO('.uno:HideColumn', 'spreadsheet', true), 'HideColumn'),
				isHtmlName: true,
				callback: (this._hideColumn).bind(this)
			},
			'.uno:ShowColumn': {
				name: app.IconUtil.createMenuItemLink(_UNO('.uno:ShowColumn', 'spreadsheet', true), 'ShowColumn'),
				isHtmlName: true,
				callback: (this._showColumn).bind(this)
			},
			'.uno:FreezePanes': {
				name: app.IconUtil.createMenuItemLink(_UNO('.uno:FreezePanes', 'spreadsheet', true), 'FreezePanes'),
				isHtmlName: true,
				callback: (this._freezePanes).bind(this)
			}
		};

		this._menuData = window.L.Control.JSDialogBuilder.getMenuStructureForMobileWizard(this._menuItem, true, '');
		this._headerInfo = new cool.HeaderInfo(this._map, true /* isCol */);
	}

	isMouseOverResizeArea(start: number, end:number, position: number, entryIsCurrent: boolean) : boolean {
		const isRTL = this.isCalcRTL();
		// NOTE: From a geometric perspective resizeAreaStart is really "resizeAreaEnd" in RTL case.
		let resizeAreaStart = isRTL ? Math.min(start + this.borderResizeHandle * app.dpiScale, end) : Math.max(start, end - this.borderResizeHandle * app.dpiScale);
		if (entryIsCurrent || (window as any).mode.isSmallScreenDevice()) {
			resizeAreaStart = isRTL ? start + this.resizeHandleSize : end - this.resizeHandleSize;
		}
		return isRTL ? (position < resizeAreaStart) : (position > resizeAreaStart);
	}

	drawHeaderEntry (entry: HeaderEntryData): void {
		if (!entry)
			return;

		const isRTL = this.isCalcRTL();
		const content = this._colIndexToAlpha(entry.index + 1);
		const startX = isRTL ? this.size[0] - entry.pos : entry.pos - entry.size;

		if (entry.size <= 0)
			return;

		const highlight = entry.isCurrent || entry.isHighlighted;

		// background gradient
		let selectionBackgroundGradient = null;
		if (highlight) {
			selectionBackgroundGradient = this.context.createLinearGradient(startX, 0, startX, this.size[1]);
			selectionBackgroundGradient.addColorStop(0, this._selectionBackgroundGradient[0]);
			selectionBackgroundGradient.addColorStop(0.5, this._selectionBackgroundGradient[1]);
			selectionBackgroundGradient.addColorStop(1, this._selectionBackgroundGradient[2]);
		}

		// draw background
		this.context.beginPath();
		this.context.fillStyle = highlight ? selectionBackgroundGradient : entry.isOver ? this._hoverColor : this._backgroundColor;
		this.context.fillRect(startX, 0, entry.size, this.size[1]);

		// draw resize handle
		const handleSize = this.resizeHandleSize;
		if (entry.isCurrent && entry.size > 2 * handleSize && !this.inResize()) {
			const center = isRTL ? startX + handleSize / 2 : startX + entry.size - handleSize / 2;
			const y = 2 * app.dpiScale;
			const h = this.size[1] - 4 * app.dpiScale;
			const size = 2 * app.dpiScale;
			const offset = 1 * app.dpiScale;

			this.context.fillStyle = '#BBBBBB';
			this.context.beginPath();
			this.context.fillRect(center - size - offset, y + 2 * app.dpiScale, size, h - 4 * app.dpiScale);
			this.context.beginPath();
			this.context.fillRect(center + offset, y + 2 * app.dpiScale, size, h - 4 * app.dpiScale);
		}

		// draw text content
		this.context.fillStyle = highlight ? this._selectionTextColor : this._textColor;
		this.context.font = this.getFont();
		this.context.textAlign = 'center';
		this.context.textBaseline = 'middle';
		// The '+ 1' below is a hack - it's currently not possible to measure
		// the exact bounding box in html5's canvas, and the textBaseline
		// 'middle' measures everything including the descent etc.
		// '+ 1' looks visually fine, and seems safe enough
		this.context.fillText(content,
			isRTL ? startX + (entry.size / 2) : entry.pos - (entry.size / 2),
			(this.size[1] / 2) + 1);

		// draw column borders.
		this.context.strokeStyle = this._borderColor;
		var offset = this.getLineOffset();
		this.context.lineWidth = this.getLineWidth();
		this.context.strokeRect(startX - offset, offset, entry.size, this.size[1]);
	}

	getHeaderEntryBoundingClientRect (index: number): Partial<DOMRect> {
		let entry = this._mouseOverEntry;
		if (index) {
			entry = this._headerInfo.getColData(index);
		}

		if (!entry)
			return;

		const rect = this.containerObject.getCanvasBoundingClientRect();
		const colStart = (entry.pos - entry.size) / app.dpiScale;
		const colEnd = entry.pos / app.dpiScale;

		const isRTL = this.isCalcRTL();

		const left = isRTL ? rect.right - colEnd : rect.left + colStart;
		const right = isRTL ? rect.right - colStart : rect.left + colEnd;
		const top = rect.top;
		const bottom = rect.bottom;
		return {left: left, right: right, top: top, bottom: bottom};
	}

	onClick(point: cool.SimplePoint, e: MouseEvent): void {
		if (!this._mouseOverEntry)
			return;

		if (this._hitResizeArea)
			return;

		const col = this._mouseOverEntry.index;

		let modifier = 0;
		if (e.shiftKey) {
			modifier += app.UNOModifier.SHIFT;
		}
		if (e.ctrlKey) {
			modifier += app.UNOModifier.CTRL;
		}

		this._selectColumn(col, modifier);
	}

	// eslint-disable-next-line @typescript-eslint/explicit-module-boundary-types
	_onDialogResult(e: any) {
		if (e.type === 'submit' && !isNaN(e.value)) {
			const extra = {
				aExtraWidth: {
					type: 'unsigned short',
					value: e.value
				}
			};

			this._map.sendUnoCommand('.uno:SetOptimalColumnWidth', extra);
		}
	}

	onDragEnd (dragDistance: number[]): number {
		if (dragDistance[0] === 0)
			return;

		let width = this._dragEntry.size;
		let column = this._dragEntry.index;

		const nextCol = this._headerInfo.getNextIndex(this._dragEntry.index);
		if (this._headerInfo.isZeroSize(nextCol)) {
			column = nextCol;
			width = 0;
		}

		const isRTL = this.isCalcRTL();

		if (isRTL) {
			width -= dragDistance[0];
		}
		else {
			width += dragDistance[0];
		}

		width /= app.dpiScale;
		width = this._map._docLayer._pixelsToTwips({x: width, y: 0}).x;

		const command = {
			ColumnWidth: {
				type: 'unsigned short',
				value: this._map._docLayer.twipsToHMM(Math.max(width, 0))
			},
			Column: {
				type: 'unsigned short',
				value: column + 1 // core expects 1-based index.
			}
		};

		this._map.sendUnoCommand('.uno:ColumnWidth', command);
		this._mouseOverEntry = null;
	}

	onMouseUp(): void {
		super.onMouseUp();

		if (!(this.containerObject.isDraggingSomething() && this._dragEntry)) {
			this._lastSelectedIndex = null;
			this._startSelectionEntry = null;
		}
	}

	setOptimalWidthAuto(): void {
		if (this._mouseOverEntry) {
			const extra = {
				aExtraWidth: {
					type: 'unsigned short',
					value: 200
				},
				Column: {
					type: 'unsigned short',
					value: this._mouseOverEntry.index + 1 // 1-based
				}
			};

			this._map.sendUnoCommand('.uno:SetOptimalColumnWidth', extra);
		}
	}

	_getParallelPos (point: cool.Point): number {
		return point.x;
	}

	_getOrthogonalPos (point: cool.Point): number {
		return point.y;
	}

	selectIndex(index: number, modifier: number): void {
		this._selectColumn(index, modifier);
	}
}

}

app.definitions.columnHeader = cool.ColumnHeader;
