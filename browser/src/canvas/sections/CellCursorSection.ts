/* global Proxy _ */
/*
 * Copyright the Collabora Online contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

class CellCursorSection extends CanvasSectionObject {
    zIndex: number = app.CSections.CellCursor.zIndex;
    drawingOrder: number = app.CSections.CellCursor.drawingOrder;
    processingOrder: number = app.CSections.CellCursor.processingOrder;
	interactable: boolean = false;

	constructor (color: string, weight: number, viewId: number) {
        super(app.CSections.CellCursor.name);

		this.documentObject = true;

		this.sectionProperties.viewId = viewId;
		this.sectionProperties.weight = weight;
		this.sectionProperties.color = color;
		this.sectionProperties.visualsReady = false;

		RenderManager.appendAfterVisualsReady(() => {
			this.sectionProperties.visualsReady = true;
			this.containerObject?.requestReDraw();
		});
	}

	public getViewId(): number {
		return this.sectionProperties.viewId;
	}

	public setViewId(viewId: number) {
		this.sectionProperties.viewId = viewId;
	}

	// If the split panes are active and the cell cursor overlaps with the split pane, we adjust the size and position.
	public static adjustSizePos(defaultSizePos: number[]): number[] {
		const splitPos = app.map._docLayer._splitPanesContext ? app.map._docLayer._splitPanesContext.getSplitPos() : null;

		if (!splitPos || (splitPos.x === 0 && splitPos.y === 0) || !app.activeDocument || (app.activeDocument.activeLayout.viewedRectangle.pX1 === 0 && app.activeDocument.activeLayout.viewedRectangle.pY1 === 0)) return defaultSizePos;

		if (defaultSizePos[0] < splitPos.x && defaultSizePos[0] + defaultSizePos[2] > splitPos.x)
			defaultSizePos[2] = Math.max(splitPos.x - defaultSizePos[0], defaultSizePos[2] - app.activeDocument.activeLayout.viewedRectangle.pX1);

		if (defaultSizePos[0] >= splitPos.x && app.activeDocument.activeLayout.viewedRectangle.pX1 + splitPos.x > defaultSizePos[0]) {
			defaultSizePos[2] = (defaultSizePos[0] + defaultSizePos[2]) - (app.activeDocument.activeLayout.viewedRectangle.pX1 + splitPos.x);
			defaultSizePos[0] = splitPos.x + app.activeDocument.activeLayout.viewedRectangle.pX1;
		}

		if (defaultSizePos[1] < splitPos.y && defaultSizePos[1] + defaultSizePos[3] > splitPos.y)
			defaultSizePos[3] = Math.max(splitPos.y - defaultSizePos[1], defaultSizePos[3] - app.activeDocument.activeLayout.viewedRectangle.pY1);

		if (defaultSizePos[1] >= splitPos.y && app.activeDocument.activeLayout.viewedRectangle.pY1 + splitPos.y > defaultSizePos[1]) {
			defaultSizePos[3] = (defaultSizePos[1] + defaultSizePos[3]) - (app.activeDocument.activeLayout.viewedRectangle.pY1 + splitPos.y);
			defaultSizePos[1] = splitPos.y + app.activeDocument.activeLayout.viewedRectangle.pY1;
		}

		return defaultSizePos;
	}

	/// The rectangle of the cursor that is shown, or null when none is.
	protected shownRectangle(): cool.SimpleRectangle | null {
		if (!app.calc.cellCursorVisible) return null;
		return (
			app.calc.cellCursorRectangle ?? new cool.SimpleRectangle(0, 0, 0, 0)
		).clone();
	}

	public onDraw() {
		if (!this.sectionProperties.visualsReady) return;

		const cursorRect = this.shownRectangle();
		if (cursorRect) {
			Util.ensureValue(app.activeDocument);

			this.context.save();
			this.context.setTransform(1, 0, 0, 1, 0, 0);

			this.context.lineJoin = 'miter';
			this.context.lineCap = 'butt';
			this.context.lineWidth = 1;

			this.context.strokeStyle = this.sectionProperties.color;

			for (let i: number = 0; i < this.sectionProperties.weight; i++) {
				// Slighyly modify rectangle in each cycle, for thickness.
				cursorRect.pX1 -= 1;
				cursorRect.pY1 -= 1;
				cursorRect.pWidth += 2;
				cursorRect.pHeight += 2;
				this.drawViewRectangle(cursorRect);
			}

			if (window.prefs.getBoolean('darkTheme')) {
				// A white line just inside the coloured border, so the cursor stays visible on a dark grid.
				this.context.strokeStyle = 'white';
				cursorRect.pX1 += 1;
				cursorRect.pY1 += 1;
				cursorRect.pWidth -= 2;
				cursorRect.pHeight -= 2;
				this.drawViewRectangle(cursorRect);
			}

			this.context.restore();
		}
	}
}

/*
	The cell cursor of a spreadsheet active in place in another document. The engine reports it in
	the coordinates of that document, and it shows only while the spreadsheet is active.
*/
class InPlaceCellCursorSection extends CellCursorSection {
	private rectangle: cool.SimpleRectangle | null = null;

	constructor(color: string, weight: number) {
		super(color, weight, 0);
	}

	/// Takes the rectangle of the cursor, in twips, or null for no cursor.
	public setRectangle(rectangle: cool.SimpleRectangle | null): void {
		this.rectangle = rectangle;
		// a section is drawn where it lies, so it lies on the cursor
		if (rectangle) {
			this.size[0] = rectangle.pWidth;
			this.size[1] = rectangle.pHeight;
			this.setPosition(rectangle.pX1, rectangle.pY1);
		}
		this.containerObject?.requestReDraw();
	}

	protected shownRectangle(): cool.SimpleRectangle | null {
		if (!this.rectangle || !GraphicSelection.inPlaceCalc) return null;
		return this.rectangle.clone();
	}
}
