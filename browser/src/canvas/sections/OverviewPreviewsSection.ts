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

// Hover and selection feedback for the slide previews on the overview (canvas) page while no
// preview is entered for editing. The previews are ordinary document content painted by the
// tiles, so this overlay is what tells the user they are clickable.
class OverviewPreviewsSection extends CanvasSectionObject {
	processingOrder: number = app.CSections.OverviewPreviews.processingOrder;
	drawingOrder: number = app.CSections.OverviewPreviews.drawingOrder;
	zIndex: number = app.CSections.OverviewPreviews.zIndex;
	documentObject: boolean = true;
	boundToSection: string = app.CSections.Tiles.name;

	constructor() {
		super(app.CSections.OverviewPreviews.name);
		this.size = app.sectionContainer.getDocumentAnchorSection().size;
		this.sectionProperties.hoveredPart = -1;
		this.sectionProperties.cursorSet = false;
		this.sectionProperties.strokeColor =
			getComputedStyle(document.documentElement).getPropertyValue(
				'--color-primary',
			) || '#0b87e7';
	}

	public static ensure(): void {
		if (
			!app.sectionContainer.doesSectionExist(
				app.CSections.OverviewPreviews.name,
			)
		)
			app.sectionContainer.addSection(new OverviewPreviewsSection());
	}

	public static remove(): void {
		if (
			app.sectionContainer.doesSectionExist(app.CSections.OverviewPreviews.name)
		)
			app.sectionContainer.removeSection(app.CSections.OverviewPreviews.name);
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

	private viewportX(documentX: number): number {
		return documentX - app.activeDocument.activeLayout.viewedRectangle.pX1;
	}

	private viewportY(documentY: number): number {
		return documentY - app.activeDocument.activeLayout.viewedRectangle.pY1;
	}

	// The overlay acts only on the overview page itself, outside preview edit mode.
	private isApplicable(): boolean {
		return (
			app.map._docLayer._docType === 'presentation' &&
			app.impress.hasOverviewPage &&
			app.map._docLayer._selectedPart === 0 &&
			!app.impress.previewEditMode.active &&
			!!app.impress.previewRects
		);
	}

	// The preview under a section-local core-pixel point, or null.
	private previewAt(
		x: number,
		y: number,
	): { part: number; rectangle: any } | null {
		if (!this.isApplicable()) return null;
		for (const preview of app.impress.previewRects) {
			const rect = preview.rectangle;
			if (
				x >= this.viewportX(rect.pX1) &&
				x <= this.viewportX(rect.pX2) &&
				y >= this.viewportY(rect.pY1) &&
				y <= this.viewportY(rect.pY2)
			)
				return preview;
		}
		return null;
	}

	// True only for a point over a preview. Every other point misses, leaving this
	// section out of the hit test there.
	public isHit(point: number[]): boolean {
		return (
			this.previewAt(
				point[0] - this.myTopLeft[0],
				point[1] - this.myTopLeft[1],
			) !== null
		);
	}

	private setHoveredPart(part: number): void {
		if (this.sectionProperties.hoveredPart === part) return;
		this.sectionProperties.hoveredPart = part;
		this.containerObject.requestReDraw();
	}

	private resetCursor(): void {
		if (this.sectionProperties.cursorSet) {
			this.context.canvas.style.cursor = '';
			this.sectionProperties.cursorSet = false;
		}
	}

	public onMouseMove(
		point: cool.SimplePoint,
		dragDistance: Array<number>,
		e: MouseEvent,
	): void {
		// During a drag the mouse pointer belongs to the drag itself, so this section
		// gives up its cursor claim and the dragged preview keeps the move cursor.
		if (this.containerObject.isDraggingSomething()) {
			this.resetCursor();
			return;
		}
		const preview = this.previewAt(point.pX, point.pY);
		this.setHoveredPart(preview ? preview.part : -1);
		if (preview) {
			// A preview moves with a drag whether or not it is selected, and the mouse
			// pointer messages from the document say the same, so the move cursor is
			// the one steady answer over a preview.
			this.context.canvas.style.cursor = 'move';
			this.sectionProperties.cursorSet = true;
		} else this.resetCursor();
	}

	public onMouseLeave(): void {
		this.setHoveredPart(-1);
		this.resetCursor();
	}

	public onDraw(): void {
		if (!this.isApplicable()) return;
		for (const preview of app.impress.previewRects) {
			const rect = preview.rectangle;
			const x = this.viewportX(rect.pX1);
			const y = this.viewportY(rect.pY1);
			if (preview.part === app.impress.selectedPreviewPart) {
				this.context.beginPath();
				this.context.strokeStyle = this.sectionProperties.strokeColor;
				this.context.lineWidth = 2;
				this.context.strokeRect(x, y, rect.pWidth, rect.pHeight);
				this.context.globalAlpha = 0.08;
				this.context.fillStyle = this.sectionProperties.strokeColor;
				this.context.fillRect(x, y, rect.pWidth, rect.pHeight);
				this.context.globalAlpha = 1;
				this.context.closePath();
			} else if (preview.part === this.sectionProperties.hoveredPart) {
				this.context.beginPath();
				this.context.strokeStyle = this.sectionProperties.strokeColor;
				this.context.lineWidth = 1;
				this.context.strokeRect(x, y, rect.pWidth, rect.pHeight);
				this.context.closePath();
			}
		}
	}
}
