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
/* See CanvasSectionContainer.ts for explanations. */

class PreviewEditSection extends CanvasSectionObject {
	processingOrder: number = app.CSections.PreviewEdit.processingOrder;
	drawingOrder: number = app.CSections.PreviewEdit.drawingOrder;
	zIndex: number = app.CSections.PreviewEdit.zIndex;
	documentObject: boolean = true;
	boundToSection: string = app.CSections.Tiles.name;

	constructor() {
		super(app.CSections.PreviewEdit.name);
		this.size = app.sectionContainer.getDocumentAnchorSection().size;
		this.sectionProperties.dragStart = null; // Core-pixel point where a marquee drag began.
		this.sectionProperties.dragDelta = null; // Current drag offset in core pixels.
		this.sectionProperties.resizeHandle = null; // Grip name while a resize drag runs.
		this.sectionProperties.cursorSet = false;
		this.sectionProperties.snapGuides = [];
		this.sectionProperties.strokeColor =
			getComputedStyle(document.documentElement).getPropertyValue(
				'--color-primary',
			) || '#0b87e7';
		this.sectionProperties.hatchColor =
			getComputedStyle(document.documentElement).getPropertyValue(
				'--color-primary-darker',
			) || '#095280';
	}

	public static ensure(): void {
		if (!app.sectionContainer.doesSectionExist(app.CSections.PreviewEdit.name))
			app.sectionContainer.addSection(new PreviewEditSection());
	}

	public static remove(): void {
		if (app.sectionContainer.doesSectionExist(app.CSections.PreviewEdit.name))
			app.sectionContainer.removeSection(app.CSections.PreviewEdit.name);
	}

	// Grip squares on the selection marquee, in core pixels: drawn size and hit size.
	private static readonly gripHalf = 4;
	private static readonly gripHitHalf = 7;
	// A resize never shrinks the shape below this, in twips, so it stays clickable.
	private static readonly minShapeTwips = 100;

	private static readonly gripCursors: { [name: string]: string } = {
		nw: 'nwse-resize',
		n: 'ns-resize',
		ne: 'nesw-resize',
		e: 'ew-resize',
		se: 'nwse-resize',
		s: 'ns-resize',
		sw: 'nesw-resize',
		w: 'ew-resize',
	};

	// Snap radius for shape moves, in core pixels.
	private static readonly snapTolerancePx = 8;

	// Grip centers for a marquee rectangle given in viewport core pixels.
	private gripCenters(
		x: number,
		y: number,
		width: number,
		height: number,
	): Array<{ name: string; x: number; y: number }> {
		return [
			{ name: 'nw', x: x, y: y },
			{ name: 'n', x: x + width / 2, y: y },
			{ name: 'ne', x: x + width, y: y },
			{ name: 'e', x: x + width, y: y + height / 2 },
			{ name: 'se', x: x + width, y: y + height },
			{ name: 's', x: x + width / 2, y: y + height },
			{ name: 'sw', x: x, y: y + height },
			{ name: 'w', x: x, y: y + height / 2 },
		];
	}

	// The grip under a section-local point, or null. Uses the marquee of the current
	// selection, not of an in-progress drag: grips are grabbed from rest.
	private gripAt(pointX: number, pointY: number): string | null {
		const selection = app.impress.previewShapeSelection;
		if (!selection) return null;
		const grips = this.gripCenters(
			this.viewportX(selection.pX1),
			this.viewportY(selection.pY1),
			selection.pWidth,
			selection.pHeight,
		);
		for (const grip of grips) {
			if (
				Math.abs(pointX - grip.x) <= PreviewEditSection.gripHitHalf &&
				Math.abs(pointY - grip.y) <= PreviewEditSection.gripHitHalf
			)
				return grip.name;
		}
		return null;
	}

	// The section is created while the document is already on screen, so it places itself
	// right away. Sections placed only by the next scroll stay invisible until then.
	public onInitialize(): void {
		this.onNewDocumentTopLeft();
	}

	public onNewDocumentTopLeft(): void {
		this.setPosition(
			app.activeDocument.activeLayout.viewedRectangle.pX1,
			app.activeDocument.activeLayout.viewedRectangle.pY1,
		);
	}

	// Section-local coordinates are viewport core pixels: the document position minus the
	// scrolled-away part. The state rectangles arrive in document space, so every drawn or
	// hit-tested value goes through this shift first.
	private viewportX(documentX: number): number {
		return documentX - app.activeDocument.activeLayout.viewedRectangle.pX1;
	}

	private viewportY(documentY: number): number {
		return documentY - app.activeDocument.activeLayout.viewedRectangle.pY1;
	}

	// Snap a moved rectangle to the entered preview's edges and center lines. Both axes
	// snap independently; the nearest candidate within the tolerance wins. The matched
	// positions are recorded in snapGuides, one entry per axis that matched.
	private snapMoveRectangle(
		rectangle: cool.SimpleRectangle,
	): cool.SimpleRectangle {
		this.sectionProperties.snapGuides = [];
		const mode = app.impress.previewEditMode;
		if (!mode.active || !mode.rectangle) return rectangle;
		const preview = mode.rectangle;
		const tolerance = Math.round(
			PreviewEditSection.snapTolerancePx * app.pixelsToTwips,
		);

		const snapAxis = (
			edges: Array<{ value: number; target: number }>,
		): { offset: number; line: number } | null => {
			let best: { offset: number; line: number } | null = null;
			for (const edge of edges) {
				const offset = edge.target - edge.value;
				if (
					Math.abs(offset) <= tolerance &&
					(!best || Math.abs(offset) < Math.abs(best.offset))
				)
					best = { offset: offset, line: edge.target };
			}
			return best;
		};

		const horizontal = snapAxis([
			{ value: rectangle.x1, target: preview.x1 },
			{ value: rectangle.x2, target: preview.x2 },
			{
				value: rectangle.x1 + rectangle.width / 2,
				target: preview.x1 + preview.width / 2,
			},
		]);
		const vertical = snapAxis([
			{ value: rectangle.y1, target: preview.y1 },
			{ value: rectangle.y2, target: preview.y2 },
			{
				value: rectangle.y1 + rectangle.height / 2,
				target: preview.y1 + preview.height / 2,
			},
		]);

		let x1 = rectangle.x1;
		let y1 = rectangle.y1;
		if (horizontal) {
			x1 += horizontal.offset;
			this.sectionProperties.snapGuides.push({
				vertical: true,
				position: horizontal.line,
			});
		}
		if (vertical) {
			y1 += vertical.offset;
			this.sectionProperties.snapGuides.push({
				vertical: false,
				position: vertical.line,
			});
		}
		return new cool.SimpleRectangle(x1, y1, rectangle.width, rectangle.height);
	}

	// The selection rectangle with the in-progress drag applied, in canvas twips. A move
	// shifts the whole rectangle; a resize moves the edges the grip owns, keeping a minimum
	// size so the rectangle never inverts.
	private currentDragRectangle(): cool.SimpleRectangle | null {
		const selection = app.impress.previewShapeSelection;
		if (!selection) return null;
		const delta = this.sectionProperties.dragDelta;
		if (!delta) return selection.clone();
		const ratio = app.pixelsToTwips;
		const deltaX = Math.round(delta[0] * ratio);
		const deltaY = Math.round(delta[1] * ratio);
		const handle = this.sectionProperties.resizeHandle;
		if (!handle)
			return this.snapMoveRectangle(
				new cool.SimpleRectangle(
					selection.x1 + deltaX,
					selection.y1 + deltaY,
					selection.width,
					selection.height,
				),
			);

		const minSize = PreviewEditSection.minShapeTwips;
		let x1 = selection.x1;
		let y1 = selection.y1;
		let x2 = selection.x2;
		let y2 = selection.y2;
		if (handle.includes('w')) x1 = Math.min(x1 + deltaX, x2 - minSize);
		if (handle.includes('e')) x2 = Math.max(x2 + deltaX, x1 + minSize);
		if (handle === 'n' || handle === 'nw' || handle === 'ne')
			y1 = Math.min(y1 + deltaY, y2 - minSize);
		if (handle === 's' || handle === 'sw' || handle === 'se')
			y2 = Math.max(y2 + deltaY, y1 + minSize);
		return new cool.SimpleRectangle(x1, y1, x2 - x1, y2 - y1);
	}

	// The entered preview carries a diagonally hatched band just outside its frame, the
	// same visual language the office suite uses for entering a group or an embedded
	// object in place. A selected shape keeps the plain dashed marquee, so the two
	// states read differently at a glance.
	private drawEnteredFrame(rectangle: cool.SimpleRectangle): void {
		const context = this.context;
		const band = 6;
		const step = 5;
		const x = this.viewportX(rectangle.pX1);
		const y = this.viewportY(rectangle.pY1);
		const outerX = x - band;
		const outerY = y - band;
		const outerWidth = rectangle.pWidth + 2 * band;
		const outerHeight = rectangle.pHeight + 2 * band;

		context.save();
		context.beginPath();
		context.rect(outerX, outerY, outerWidth, outerHeight);
		context.rect(x, y, rectangle.pWidth, rectangle.pHeight);
		context.clip('evenodd');

		context.strokeStyle = this.sectionProperties.hatchColor;
		context.lineWidth = 1.5;
		context.beginPath();
		for (let offset = -outerHeight; offset < outerWidth; offset += step) {
			context.moveTo(outerX + offset, outerY);
			context.lineTo(outerX + offset + outerHeight, outerY + outerHeight);
		}
		context.stroke();
		context.restore();

		context.beginPath();
		context.strokeStyle = this.sectionProperties.hatchColor;
		context.lineWidth = 1;
		context.strokeRect(x, y, rectangle.pWidth, rectangle.pHeight);
		context.closePath();
	}

	public onDraw(): void {
		const mode = app.impress.previewEditMode;
		if (!mode.active || !mode.rectangle) return;

		this.drawEnteredFrame(mode.rectangle);
		this.context.strokeStyle = this.sectionProperties.strokeColor;
		this.context.lineWidth = 2;

		// Marquee on the selected shape, shifted by the local drag offset while dragging.
		const dragRectangle = this.currentDragRectangle();
		if (dragRectangle) {
			const x = this.viewportX(dragRectangle.pX1);
			const y = this.viewportY(dragRectangle.pY1);
			this.context.beginPath();
			this.context.setLineDash([4, 3]);
			this.context.strokeRect(
				x,
				y,
				dragRectangle.pWidth,
				dragRectangle.pHeight,
			);
			this.context.setLineDash([]);
			this.context.closePath();

			// Grips stay hidden while the shape body is moved; during a resize they show
			// on the live rectangle so the grabbed edge is visible.
			const moving =
				this.sectionProperties.dragDelta &&
				!this.sectionProperties.resizeHandle;
			if (!moving) {
				this.context.fillStyle = this.sectionProperties.strokeColor;
				const half = PreviewEditSection.gripHalf;
				for (const grip of this.gripCenters(
					x,
					y,
					dragRectangle.pWidth,
					dragRectangle.pHeight,
				))
					this.context.fillRect(
						grip.x - half,
						grip.y - half,
						2 * half,
						2 * half,
					);
			}
		}

		if (
			this.sectionProperties.dragDelta &&
			this.sectionProperties.snapGuides.length &&
			mode.rectangle
		) {
			const preview = mode.rectangle;
			this.context.beginPath();
			this.context.setLineDash([6, 4]);
			this.context.strokeStyle = this.sectionProperties.hatchColor;
			this.context.lineWidth = 1;
			for (const guide of this.sectionProperties.snapGuides) {
				const guidePixels = Math.round(guide.position * app.twipsToPixels);
				if (guide.vertical) {
					this.context.moveTo(
						this.viewportX(guidePixels),
						this.viewportY(preview.pY1),
					);
					this.context.lineTo(
						this.viewportX(guidePixels),
						this.viewportY(preview.pY2),
					);
				} else {
					this.context.moveTo(
						this.viewportX(preview.pX1),
						this.viewportY(guidePixels),
					);
					this.context.lineTo(
						this.viewportX(preview.pX2),
						this.viewportY(guidePixels),
					);
				}
			}
			this.context.stroke();
			this.context.setLineDash([]);
			this.context.closePath();
		}
	}

	public onMouseDown(point: cool.SimplePoint, e: MouseEvent): void {
		if (e.type === 'touchstart') return;
		const selection = app.impress.previewShapeSelection;
		if (!selection) return;
		const grip = this.gripAt(point.pX, point.pY);
		if (grip) {
			this.sectionProperties.resizeHandle = grip;
			this.sectionProperties.dragStart = [point.pX, point.pY];
			this.stopPropagating();
			e.stopPropagation();
			return;
		}
		if (
			point.pX >= this.viewportX(selection.pX1) &&
			point.pX <= this.viewportX(selection.pX2) &&
			point.pY >= this.viewportY(selection.pY1) &&
			point.pY <= this.viewportY(selection.pY2)
		) {
			this.sectionProperties.dragStart = [point.pX, point.pY];
			this.stopPropagating();
			e.stopPropagation();
		}
	}

	public onMouseMove(
		point: cool.SimplePoint,
		dragDistance: Array<number>,
		e: MouseEvent,
	): void {
		if (e.type === 'touchmove') return;
		if (!this.sectionProperties.dragStart) {
			this.updateHoverCursor(point);
			return;
		}
		if (
			!this.containerObject.isDraggingSomething() ||
			this.containerObject.targetSection !== this.name
		)
			return;
		this.sectionProperties.dragDelta = [
			point.pX - this.sectionProperties.dragStart[0],
			point.pY - this.sectionProperties.dragStart[1],
		];
		this.stopPropagating();
		e.stopPropagation();
		this.containerObject.requestReDraw();
	}

	// Resize cursors over the grips, a move cursor over the marquee body. Claiming the
	// cursor stops the current mousemove from propagating past this section.
	private updateHoverCursor(point: cool.SimplePoint): void {
		if (this.containerObject.isDraggingSomething()) return;
		const selection = app.impress.previewShapeSelection;
		const grip = selection ? this.gripAt(point.pX, point.pY) : null;
		let cursor = '';
		if (grip) cursor = PreviewEditSection.gripCursors[grip];
		else if (
			selection &&
			point.pX >= this.viewportX(selection.pX1) &&
			point.pX <= this.viewportX(selection.pX2) &&
			point.pY >= this.viewportY(selection.pY1) &&
			point.pY <= this.viewportY(selection.pY2)
		)
			cursor = 'move';
		if (cursor) {
			this.context.canvas.style.cursor = cursor;
			this.sectionProperties.cursorSet = true;
			this.stopPropagating();
		} else if (this.sectionProperties.cursorSet) {
			this.context.canvas.style.cursor = '';
			this.sectionProperties.cursorSet = false;
		}
	}

	public onMouseUp(point: cool.SimplePoint, e: MouseEvent): void {
		if (e.type === 'touchend') return;
		if (!this.sectionProperties.dragStart) return;
		const dropped = this.currentDragRectangle();
		const wasResize = !!this.sectionProperties.resizeHandle;
		const hadDelta =
			this.sectionProperties.dragDelta &&
			(this.sectionProperties.dragDelta[0] !== 0 ||
				this.sectionProperties.dragDelta[1] !== 0);
		this.sectionProperties.dragStart = null;
		this.sectionProperties.dragDelta = null;
		this.sectionProperties.resizeHandle = null;
		this.sectionProperties.snapGuides = [];
		if (
			!this.containerObject.isDraggingSomething() ||
			this.containerObject.targetSection !== this.name
		)
			return;
		if (!dropped || !hadDelta) return;

		if (wasResize) {
			// A resize always sends the plain move command, regardless of where the
			// dragged edge lands relative to the entered preview's border.
			const slideRectangle =
				PreviewEditTransform.canvasRectangleToSlide(dropped);
			app.map.sendUnoCommand('.uno:MovePreviewShape', {
				X: { type: 'long', value: String(slideRectangle.x1) },
				Y: { type: 'long', value: String(slideRectangle.y1) },
				Width: { type: 'long', value: String(slideRectangle.width) },
				Height: { type: 'long', value: String(slideRectangle.height) },
			});
		} else {
			const centerX = dropped.x1 + dropped.width / 2;
			const centerY = dropped.y1 + dropped.height / 2;
			if (PreviewEditTransform.isInsideEnteredPreview(centerX, centerY)) {
				const slideRectangle =
					PreviewEditTransform.canvasRectangleToSlide(dropped);
				app.map.sendUnoCommand('.uno:MovePreviewShape', {
					X: { type: 'long', value: String(slideRectangle.x1) },
					Y: { type: 'long', value: String(slideRectangle.y1) },
					Width: { type: 'long', value: String(slideRectangle.width) },
					Height: { type: 'long', value: String(slideRectangle.height) },
				});
			} else {
				app.map.sendUnoCommand('.uno:MovePreviewShapeOut', {
					X: { type: 'long', value: String(dropped.x1) },
					Y: { type: 'long', value: String(dropped.y1) },
					Width: { type: 'long', value: String(dropped.width) },
					Height: { type: 'long', value: String(dropped.height) },
				});
			}
		}
		this.stopPropagating();
		e.stopPropagation();
	}
}
