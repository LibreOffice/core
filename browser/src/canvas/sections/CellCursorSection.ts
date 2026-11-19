/* global Proxy _ */
/*
 * Copyright the Collabora Office contributors.
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

	/**
	 * How long the cursor takes to slide to another cell at the start, in
	 * milliseconds.
	 */
	public static readonly initialSlideTime: number = 100;

	/**
	 * How long the cursor takes to slide to another cell, in milliseconds;
	 * 0 makes it jump.
	 */
	public static slideTime: number = CellCursorSection.initialSlideTime;

	/**
	 * The rectangle that the cursor slides from, or null while the cursor is
	 * not sliding.
	 */
	private slideFrom: cool.SimpleRectangle | null = null;

	/** The time the slide started, as performance.now() gives it. */
	private slideStart: number = 0;

	/** The animation frame that draws the slide next, or null. */
	private slideFrame: number | null = null;

	/**
	 * Get a rectangle on the way between two. Progress runs from 0 to 1, and
	 * the move slows down as it arrives.
	 *
	 * @throws Error when progress is outside 0 to 1.
	 */
	public static interpolate(
		from: cool.SimpleRectangle,
		to: cool.SimpleRectangle,
		progress: number,
	): cool.SimpleRectangle {
		if (Number.isNaN(progress) || progress < 0 || progress > 1)
			throw new Error('Slide progress out of range: ' + progress);

		const eased = 1 - (1 - progress) * (1 - progress);
		const x = from.x1 + (to.x1 - from.x1) * eased;
		const y = from.y1 + (to.y1 - from.y1) * eased;
		const width = from.width + (to.width - from.width) * eased;
		const height = from.height + (to.height - from.height) * eased;
		return new cool.SimpleRectangle(x, y, width, height);
	}

	/**
	 * Get the rectangle that the cursor is drawn at now: on its way while
	 * it slides, otherwise its own. Null when the cursor is not visible.
	 */
	public getDrawnRectangle(): cool.SimpleRectangle | null {
		if (!app.calc.cellCursorVisible) return null;

		const target =
			app.calc.cellCursorRectangle ?? new cool.SimpleRectangle(0, 0, 0, 0);

		const slideTime = CellCursorSection.slideTime;
		if (!this.slideFrom || slideTime <= 0)
			// No slide runs, or the slide was turned off.
			return target.clone();

		const progress = (performance.now() - this.slideStart) / slideTime;
		if (progress >= 1)
			// The slide has run its course.
			return target.clone();

		return CellCursorSection.interpolate(this.slideFrom, target, progress);
	}

	/**
	 * Start a slide from the given rectangle to the rectangle that the cursor
	 * has now, app.calc.cellCursorRectangle. The cursor jumps when the move
	 * does not allow a slide.
	 */
	public startSlideFrom(from: cool.SimpleRectangle): void {
		this.slideFrom = null;
		this.slideStart = performance.now();

		if (!this.canSlideFrom(from))
			return;

		this.slideFrom = from.clone();

		// Keep the fill handle on the cursor from the first frame on.
		this.placeFillHandle();
		this.drawSlide();
	}

	/** End the slide, if one runs; the cursor is drawn on its cell. */
	public endSlide(): void {
		this.slideFrom = null;
	}

	/**
	 * Test whether the cursor can actually slide from the specified rectangle
	 * to the current cursor position.
	 */
	private canSlideFrom(from: cool.SimpleRectangle): boolean {
		const target = app.calc.cellCursorRectangle;
		if (!target || !app.calc.cellCursorVisible)
			// There is no cursor to slide.
			return false;

		if (!LocalCellCursor.isEnabled())
			// The cursor slides only in the local mode.
			return false;

		if (CellCursorSection.slideTime <= 0)
			// The slide is turned off.
			return false;

		if (DomUtil.prefersReducedMotion())
			return false;

		// Both source and target must be in the same visible frame for the
		// sliding to happen.
		const fromInView = app.isRectangleVisibleInTheDisplayedArea(from.toArray());
		const targetInView = app.isRectangleVisibleInTheDisplayedArea(target.toArray());
		return fromInView && targetInView;
	}

	/** Draw the canvas again on every frame until the slide is over. */
	private drawSlide(): void {
		if (this.slideFrame !== null)
			// A frame is already scheduled - bail out.
			return;

		this.slideFrame = requestAnimationFrame(() => {
			this.slideFrame = null;
			const running = performance.now() - this.slideStart < CellCursorSection.slideTime;
			if (!running)
				// The time is up - end the slide animation.
				this.slideFrom = null;

			this.placeFillHandle();
			app.sectionContainer.requestReDraw();

			if (running)
				// Schedule the next frame.
				this.drawSlide();
		});
	}

	/** Put the fill handle on the corner of the cursor as it is drawn. */
	private placeFillHandle(): void {
		const drawn = this.getDrawnRectangle();
		if (!drawn)
			// The cursor is not visible - no need to draw anything.
			return;

		const name = app.CSections.CellFillMarker.name;
		const section = app.sectionContainer.getSectionWithName(name);
		const fillHandle = section as AutoFillBaseSection | null;
		if (!fillHandle)
			// This view has no fill handle.
			return;

		// The handle sits on the bottom-right corner, given in core pixels.
		const corner = [drawn.pX2, drawn.pY2];
		fillHandle.calculatePositionViaCellCursor(corner);
	}

	/// The rectangle of the cursor that is shown, or null when none is.
	protected shownRectangle(): cool.SimpleRectangle | null {
		return this.getDrawnRectangle();
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
