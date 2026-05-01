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

	This section is activated when user (currently) clicks on [View -> Focus Cell] button.
	When feature is activated, this section draws 2 rectangles:
		* One vertical that indicates the column of the cell cursor.
		* One horizontal that indicates the row of the cell cursor.

	So the purpose is to visually indicate the cell cursor position in the document better.
*/

class FocusCellSection extends CanvasSectionObject {
	processingOrder: number = app.CSections.FocusCell.processingOrder;
	drawingOrder: number = app.CSections.FocusCell.drawingOrder;
	zIndex: number = app.CSections.FocusCell.zIndex;
	documentObject: boolean = true;
	interactable: boolean = false;
	static instance: FocusCellSection = null;

	constructor() {
		super(app.CSections.FocusCell.name);

		this.sectionProperties.columnRectangle = null;
		this.sectionProperties.rowRectangle = null;
		this.sectionProperties.maxCol = 268435455;
		this.sectionProperties.maxRow = 20971124;
		this.isAlwaysVisible = true;
	}

	public onCellAddressChanged(): void {
		this.size[0] = app.calc.cellCursorRectangle.pWidth;
		this.size[1] = app.calc.cellCursorRectangle.pHeight;
		this.setPosition(
			app.calc.cellCursorRectangle.pX1,
			app.calc.cellCursorRectangle.pY1,
		);
	}

	private static addFocusCellSection() {
		if (FocusCellSection.instance === null) {
			FocusCellSection.instance = new FocusCellSection();
			app.sectionContainer.addSection(FocusCellSection.instance);
		}

		if (!this.instance.showSection) this.instance.setShowSection(true);

		this.instance.onCellAddressChanged();
	}

	public static hideFocusCellSection() {
		if (FocusCellSection.instance)
			FocusCellSection.instance.setShowSection(false);
	}

	public static showFocusCellSection() {
		if (FocusCellSection.instance)
			FocusCellSection.instance.setShowSection(true);
		else {
			this.addFocusCellSection();
		}
	}

	public onDraw() {
		const cursor = app.calc.cellCursorRectangle;
		const adjusted = CellCursorSection.adjustSizePos([
			cursor.pX1,
			cursor.pY1,
			cursor.pWidth,
			cursor.pHeight,
		]);
		const drawColumn = adjusted[2] > 0;
		const drawRow = adjusted[3] > 0;

		if (!drawColumn && !drawRow) return;

		const style = getComputedStyle(document.documentElement).getPropertyValue(
			'--column-row-highlight',
		);

		this.context.fillStyle = style;
		this.context.strokeStyle = style;

		const colX = adjusted[0] - cursor.pX1;
		const colWidth = adjusted[2];
		const rowY = adjusted[1] - cursor.pY1;
		const rowHeight = adjusted[3];

		this.context.globalAlpha = 0.3;

		if (drawColumn) {
			this.context.fillRect(
				colX,
				-cursor.pY1,
				colWidth,
				this.sectionProperties.maxCol,
			);
		}

		if (drawRow) {
			this.context.fillRect(
				-cursor.pX1,
				rowY,
				this.sectionProperties.maxRow,
				rowHeight,
			);
		}

		this.context.globalAlpha = 1;
		this.context.lineWidth = 2 * app.dpiScale;

		if (drawColumn) {
			this.context.strokeRect(
				colX,
				-cursor.pY1,
				colWidth,
				this.sectionProperties.maxCol,
			);
		}

		if (drawRow) {
			this.context.strokeRect(
				-cursor.pX1,
				rowY,
				this.sectionProperties.maxRow,
				rowHeight,
			);
		}
	}
}
