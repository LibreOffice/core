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
 */

/* global _UNO app */

declare var _UNO: any;

namespace cool {

export class RowHeader extends cool.Header {
	anchor: Array<Array<string>> = [[app.CSections.CornerHeader.name, 'bottom', 'top'], [app.CSections.RowGroup.name, 'right', 'left']];
	position: number[] = [0, 0]; // This section's myTopLeft is placed according to corner header and row group sections.
	size: number[] = [48 * app.dpiScale, 0]; // No initial height is necessary.
	expand: string[] = ['top', 'bottom']; // Expand vertically.
	processingOrder: number = app.CSections.RowHeader.processingOrder;
	drawingOrder: number = app.CSections.RowHeader.drawingOrder;
	zIndex: number = app.CSections.RowHeader.zIndex;
	cursor: string = 'row-resize';

	_current: number;
	_selection: SelectionRange;

	constructor(cursor?: string) {
		super(app.CSections.RowHeader.name);

		if (cursor)
			this.cursor = cursor;
	}

	onInitialize(): void {
		this._map = window.L.Map.THIS;
		this._isColumn = false;
		this._current = -1;
		this.resizeHandleSize = 15 * app.dpiScale;
		this._selection = {start: -1, end: -1};
		this._mouseOverEntry = null;
		this._lastMouseOverIndex = undefined;
		this._hitResizeArea = false;
		this.sectionProperties.docLayer = this._map._docLayer;

		super.onInitialize();

		this._headerMenuID = 'row-header-menu';
		this._menuItem = {
			'.uno:InsertRowsBefore': {
				name: app.IconUtil.createMenuItemLink(_UNO('.uno:InsertRowsBefore', 'spreadsheet', true), 'InsertRowsBefore'),
				isHtmlName: true,
				callback: (this._insertRowAbove).bind(this)
			},
			'.uno:InsertRowsAfter': {
				name: app.IconUtil.createMenuItemLink(_UNO('.uno:InsertRowsAfter', 'spreadsheet', true), 'InsertRowsAfter'),
				isHtmlName: true,
				callback: (this._insertRowBelow).bind(this)
			},
			'.uno:DeleteRows': {
				name: app.IconUtil.createMenuItemLink(_UNO('.uno:DeleteRows', 'spreadsheet', true), 'DeleteRows'),
				isHtmlName: true,
				callback: (this._deleteSelectedRow).bind(this)
			},
			'.uno:RowHeight': {
				name: app.IconUtil.createMenuItemLink(_UNO('.uno:RowHeight', 'spreadsheet', true), 'RowHeight'),
				isHtmlName: true,
				callback: (this._rowHeight).bind(this)
			},
			'.uno:SetOptimalRowHeight': {
				name: app.IconUtil.createMenuItemLink(_UNO('.uno:SetOptimalRowHeight', 'spreadsheet', true), 'SetOptimalRowHeight'),
				isHtmlName: true,
				callback: (this._optimalHeight).bind(this)
			},
			'.uno:HideRow': {
				name: app.IconUtil.createMenuItemLink(_UNO('.uno:HideRow', 'spreadsheet', true), 'HideRow'),
				isHtmlName: true,
				callback: (this._hideRow).bind(this)
			},
			'.uno:ShowRow': {
				name: app.IconUtil.createMenuItemLink(_UNO('.uno:ShowRow', 'spreadsheet', true), 'ShowRow'),
				isHtmlName: true,
				callback: (this._showRow).bind(this)
			},
			'.uno:FreezePanes': {
				name: app.IconUtil.createMenuItemLink(_UNO('.uno:FreezePanes', 'spreadsheet', true), 'FreezePanes'),
				isHtmlName: true,
				callback: (this._freezePanes).bind(this)
			}
		};

		this._menuData = window.L.Control.JSDialogBuilder.getMenuStructureForMobileWizard(this._menuItem, true, '');
		this._headerInfo = new cool.HeaderInfo(this._map, false /* isCol */);
	}

	isMouseOverResizeArea(start: number, end:number, position: number, entryIsCurrent: boolean) : boolean {
		let resizeAreaStart = Math.max(start, end - this.borderResizeHandle * app.dpiScale);
		if (entryIsCurrent || (window as any).mode.isSmallScreenDevice()) {
			if (this.resizeHandleSize > (end - start) / 4) {
				resizeAreaStart = end - ((end - start) / 4);
			} else {
				resizeAreaStart =  end - this.resizeHandleSize;
			}
		}
		return position > resizeAreaStart;
	}

	drawHeaderEntry (entry: HeaderEntryData): void {
		if (!entry)
			return;

		const content = entry.index + 1;
		const startY = entry.pos - entry.size;

		if (entry.size <= 0)
			return;

		const highlight = entry.isCurrent || entry.isHighlighted;

		// background gradient
		let selectionBackgroundGradient = null;
		if (highlight) {
			selectionBackgroundGradient = this.context.createLinearGradient(0, startY, 0, startY + entry.size);
			selectionBackgroundGradient.addColorStop(0, this._selectionBackgroundGradient[0]);
			selectionBackgroundGradient.addColorStop(0.5, this._selectionBackgroundGradient[1]);
			selectionBackgroundGradient.addColorStop(1, this._selectionBackgroundGradient[2]);
		}

		// draw background
		this.context.beginPath();
		this.context.fillStyle = highlight ? selectionBackgroundGradient : entry.isOver ? this._hoverColor : this._backgroundColor;
		this.context.fillRect(0, startY, this.size[0], entry.size);

		// draw resize handle
		const handleSize = this.resizeHandleSize;
		if (entry.isCurrent && entry.size > 2 * handleSize && !this.inResize()) {
			const center = startY + entry.size - handleSize / 2;
			const x = 2 * app.dpiScale;
			const w = this.size[0] - 4 * app.dpiScale;
			const size = 2 * app.dpiScale;
			const offsetOnePixel = 1 * app.dpiScale;

			this.context.fillStyle = '#BBBBBB';
			this.context.beginPath();
			this.context.fillRect(x + 2 * app.dpiScale, center - size - offsetOnePixel, w - 4 * app.dpiScale, size);
			this.context.beginPath();
			this.context.fillRect(x + 2 * app.dpiScale, center + offsetOnePixel, w - 4 * app.dpiScale, size);
		}

		// draw text content
		this.context.fillStyle = highlight ? this._selectionTextColor : this._textColor;
		this.context.font = this.getFont();
		this.context.textAlign = 'center';
		this.context.textBaseline = 'middle';
		this.context.fillText(content.toString(), this.size[0] / 2, entry.pos - (entry.size / 2) + app.roundedDpiScale);

		// draw row borders.
		this.context.strokeStyle = this._borderColor;
		const offset = this.getLineOffset();
		this.context.lineWidth = this.getLineWidth();
		this.context.strokeRect(offset, startY - offset, this.size[0], entry.size);
	}

	getHeaderEntryBoundingClientRect (index: number): Partial<DOMRect> {
		let entry = this._mouseOverEntry;

		if (index)
			entry = this._headerInfo.getRowData(index);

		if (!entry)
			return;

		const rect = this.containerObject.getCanvasBoundingClientRect();

		const rowStart = (entry.pos - entry.size) / app.dpiScale;
		const rowEnd = entry.pos / app.dpiScale;

		const left = rect.left;
		const right = rect.right;
		const top = rect.top + rowStart;
		const bottom = rect.top + rowEnd;
		return {left: left, right: right, top: top, bottom: bottom};
	}

	onClick (point: cool.SimplePoint, e: MouseEvent): void {
		if (!this._mouseOverEntry)
			return;

		if (this._hitResizeArea)
			return;

		const row = this._mouseOverEntry.index;

		let modifier = 0;
		if (e.shiftKey) {
			modifier += app.UNOModifier.SHIFT;
		}
		if (e.ctrlKey) {
			modifier += app.UNOModifier.CTRL;
		}

		this._selectRow(row, modifier);
	}

	// eslint-disable-next-line @typescript-eslint/explicit-module-boundary-types
	_onDialogResult (e: any): void {
		if (e.type === 'submit' && !isNaN(e.value)) {
			const extra = {
				aExtraHeight: {
					type: 'unsigned short',
					value: e.value
				}
			};

			this._map.sendUnoCommand('.uno:SetOptimalRowHeight', extra);
		}
	}

	onDragEnd (dragDistance: number[]): void {
		if (dragDistance[1] === 0)
			return;

		let height = this._dragEntry.size;
		let row = this._dragEntry.index;

		const nextRow = this._headerInfo.getNextIndex(this._dragEntry.index);
		if (this._headerInfo.isZeroSize(nextRow)) {
			row = nextRow;
			height = 0;
		}

		height += dragDistance[1];
		height /= app.dpiScale;
		height = this._map._docLayer._pixelsToTwips({x: 0, y: height}).y;

		const command = {
			RowHeight: {
				type: 'unsigned short',
				value: this._map._docLayer.twipsToHMM(Math.max(height, 0))
			},
			Row: {
				type: 'long',
				value: row + 1 // core expects 1-based index.
			}
		};

		this._map.sendUnoCommand('.uno:RowHeight', command);
		this._mouseOverEntry = null;
	}

	onMouseUp(): void {
		super.onMouseUp();

		if (!(this.containerObject.isDraggingSomething() && this._dragEntry)) {
			this._lastSelectedIndex = null;
			this._startSelectionEntry = null;
		}
	}

	setOptimalHeightAuto(): void {
		if (this._mouseOverEntry) {
			const extra = {
				aExtraHeight: {
					type: 'unsigned short',
					value: 0
				}
			};

			this._map.sendUnoCommand('.uno:SetOptimalRowHeight', extra);
		}
	}

	_getParallelPos (point: cool.Point): number {
		return point.y;
	}

	_getOrthogonalPos (point: cool.Point): number {
		return point.x;
	}

	selectIndex(index: number, modifier: number): void {
		this._selectRow(index, modifier);
	}
}

}

app.definitions.rowHeader = cool.RowHeader;
