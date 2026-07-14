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

/* global app */

/*
	This file is Calc only. This adds a section for grouped columns in Calc.
	When user selects some columns and groups them using "Data->Group and Outline->Group" menu path, this section is added into
	sections list of CanvasSectionContainer. See _addRemoveGroupSections in file CalcTileLayer.js

	This class is an extended version of "CanvasSectionObject".
*/

namespace cool {

export class ColumnGroup extends GroupBase {
	anchor: any = ['top', [app.CSections.CornerGroup.name, 'right', 'left']];
	expand: string[] = ['left', 'right']; // Expand horizontally.
	processingOrder: number = app.CSections.ColumnGroup.processingOrder;
	drawingOrder: number = app.CSections.ColumnGroup.drawingOrder;
	zIndex: number = app.CSections.ColumnGroup.zIndex;

	_sheetGeometry: cool.SheetGeometry;
	_cornerHeaderWidth: number;
	_splitPos: cool.Point;

	constructor() { super(app.CSections.ColumnGroup.name); }

	update(): void {
		if (this.isRemoved) // Prevent calling while deleting the section. It causes errors.
			return;

		this._sheetGeometry = this._map._docLayer.sheetGeometry;
		this._groups = Array(this._sheetGeometry.getColumnGroupLevels());

		// Calculate width on the fly.
		this.size[1] = this._computeSectionHeight();

		this._cornerHeaderWidth = this.containerObject.getSectionWithName(app.CSections.CornerHeader.name).size[0];

		this._splitPos = (this._map._docLayer._splitPanesContext as cool.SplitPanesContext).getSplitPos();

		this._collectGroupsData(this._sheetGeometry.getColumnGroupsDataInView());
	}

	onNewDocumentTopLeft(): void {
		this.update();
	}

	// This returns the required height for the section.
	_computeSectionHeight(): number {
		return this._levelSpacing + (this._groupHeadSize + this._levelSpacing) * (this._groups.length + 1);
	}

	isGroupHeaderVisible (startX: number, startPos: number): boolean {
		if (startPos > this._splitPos.x) {
			return startX > this._splitPos.x + this._cornerHeaderWidth;
		}
		else {
			return startX >= this._cornerHeaderWidth && (startX > app.activeDocument.activeLayout.viewedRectangle.pX1 || startX < this._splitPos.x);
		}
	}

	getEndPosition (endPos: number): number {
		if (endPos <= this._splitPos.x)
			return endPos;
		else {
			return Math.max(endPos + this._cornerHeaderWidth - app.activeDocument.activeLayout.viewedRectangle.pX1, this._splitPos.x + this._cornerHeaderWidth);
		}
	}

	getRelativeX (docPos: number): number {
		if (docPos < this._splitPos.x)
			return docPos + this._cornerHeaderWidth;
		else
			return Math.max(docPos - app.activeDocument.activeLayout.viewedRectangle.pX1, this._splitPos.x) + this._cornerHeaderWidth;
	}

	_getGroupAnchorPos (group: GroupEntry): [number, number] {
		return [this.getRelativeX(group.startPos), this._levelSpacing + (this._groupHeadSize + this._levelSpacing) * group.level];
	}

	_getLevelHeaderPos (level: number): [number, number] {
		return [Math.round((this._cornerHeaderWidth - this._groupHeadSize) * 0.5), this._levelSpacing + (this._groupHeadSize + this._levelSpacing) * level];
	}

	_getOutlineType (): string {
		return 'column';
	}

	drawGroupControl (group: GroupEntry): void {
		let [startX, startY] = this._getGroupAnchorPos(group);
		startX = Math.round(startX);
		startY = Math.round(startY);
		const strokeColor = this.getColors().strokeColor;
		const endX = this.getEndPosition(group.endPos);

		if (this.isGroupHeaderVisible(startX, group.startPos)) {
			this.drawGroupBoxes(startX, startY, group.hidden);
		}

		if (!group.hidden && endX > this._cornerHeaderWidth + this._groupHeadSize && endX > startX) {
			//draw tail
			this.context.beginPath();
			startX += this._groupHeadSize;
			startX = startX >= this._cornerHeaderWidth + this._groupHeadSize ? startX: this._cornerHeaderWidth + this._groupHeadSize;
			startY += this._groupHeadSize * 0.5;
			startX = Math.round(startX) + 1;
			startY = Math.round(startY);
			this.context.strokeStyle = strokeColor;
			this.context.lineWidth = 1.0;
			this.context.moveTo(this.transformX(startX) + 0.5, startY + 0.5);
			this.context.lineTo(this.transformX(endX - app.roundedDpiScale) + 0.5, startY + 0.5);
			this.context.stroke();
		}
	}

	// When user clicks somewhere on the section, onMouseClick event is called by CanvasSectionContainer.
	// Clicked point is also given to handler function. This function finds the clicked header.
	findClickedLevel (point: cool.SimplePoint): number {
		const mirrorX = this.isCalcRTL();
		if ((!mirrorX && point.pX < this._cornerHeaderWidth)
			|| (mirrorX && point.pX > this.size[0] - this._cornerHeaderWidth)) {
			let index = (point.pY / this.size[1]) * 100; // Percentage.
			const levelPercentage = (1 / (this._groups.length + 1)) * 100; // There is one more button than the number of levels.
			index = Math.floor(index / levelPercentage);
			return index;
		}
		return -1;
	}

	getTailsGroupRect (group: GroupEntry): number[] {
		const [startX, startY] = this._getGroupAnchorPos(group);
		const endX = group.endPos + this._cornerHeaderWidth - app.activeDocument.activeLayout.viewedRectangle.pX1;
		const endY = startY + this._groupHeadSize;
		return [startX, endX, startY, endY];
	}

	onRemove(): void {
		this.isRemoved = true;
		this.containerObject.getSectionWithName(app.CSections.ColumnHeader.name).position[1] = 0;
		this.containerObject.getSectionWithName(app.CSections.CornerHeader.name).position[1] = 0;
	}
}

}
