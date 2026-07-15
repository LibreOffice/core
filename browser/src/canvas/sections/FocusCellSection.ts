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
	position: number[] = [0, 0];

	constructor() {
		super(app.CSections.FocusCell.name);

		this.sectionProperties.columnRectangle = null;
		this.sectionProperties.rowRectangle = null;
		this.sectionProperties.maxCol = 268435455;
		this.sectionProperties.maxRow = 20971124;
		this.isAlwaysVisible = true;
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
		Util.ensureValue(app.activeDocument);
		Util.ensureValue(app.calc.splitCoordinate);

		this.context.save();
		this.context.setTransform(1, 0, 0, 1, 0, 0);

		const style = getComputedStyle(document.documentElement).getPropertyValue(
			'--column-row-highlight',
		);

		this.context.fillStyle = style;
		this.context.strokeStyle = style;
		this.context.globalAlpha = 0.3;

		let cursor = app.calc.cellCursorRectangle?.clone();
		if (!cursor) return;
		cursor.pX1 =
			app.calc.splitCoordinate.pX !== 0
				? 0
				: app.activeDocument.activeLayout.viewedRectangle.pX1;
		cursor.pX2 = app.activeDocument.activeLayout.viewedRectangle.pX2;
		this.drawViewRectangle(cursor, true);

		cursor = app.calc.cellCursorRectangle?.clone();
		if (!cursor) return;
		cursor.pY1 =
			app.calc.splitCoordinate.pY !== 0
				? 0
				: app.activeDocument.activeLayout.viewedRectangle.pY1;
		cursor.pY2 = app.activeDocument.activeLayout.viewedRectangle.pY2;
		this.drawViewRectangle(cursor, true);

		this.context.restore();
	}
}
