// @ts-strict-ignore
/* -*- js-indent-level: 8 -*- */

/*
 * Copyright the Collabora Office contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/* See CanvasSectionContainer.ts for explanations. */

class SelectionRectangle extends CanvasSectionObject {
	processingOrder: number = app.CSections.SelectionRectangle.processingOrder;
	drawingOrder: number = app.CSections.SelectionRectangle.drawingOrder;
	zIndex: number = app.CSections.SelectionRectangle.zIndex;
	documentObject: boolean = true;
	boundToSection: string = app.CSections.Tiles.name;

	// Same gesture is used to move the screen on touch devices. So we don't draw a selection rectangle on touch events.
	constructor() {
		super(app.CSections.SelectionRectangle.name);
		this.size = app.sectionContainer.getDocumentAnchorSection().size;

		this.sectionProperties.positionOnMouseDown = null;
		this.sectionProperties.selectionSize = null;
		this.sectionProperties.active = true;
	}

	public setActive(active: boolean) {
		this.sectionProperties.active = active;
	}

	public onMouseMove(
		point: cool.SimplePoint,
		dragDistance: Array<number>,
		e: MouseEvent,
	) {
		if (e.type === 'touchmove') return;

		if (
			this.containerObject.isDraggingSomething() &&
			this.containerObject.targetSection === this.name &&
			this.sectionProperties.positionOnMouseDown
		) {
			this.sectionProperties.selectionSize = [
				point.pX - this.sectionProperties.positionOnMouseDown.pX,
				point.pY - this.sectionProperties.positionOnMouseDown.pY,
			];
		}
	}

	public onMouseUp(point: cool.SimplePoint, e: MouseEvent): void {
		if (e.type === 'touchend') return;

		this.sectionProperties.positionOnMouseDown = null;
		this.sectionProperties.selectionSize = null;
	}

	public onMouseDown(point: cool.SimplePoint, e: MouseEvent): void {
		if (e.type === 'touchstart') return;

		this.sectionProperties.positionOnMouseDown = point;
	}

	public onNewDocumentTopLeft(): void {
		this.setPosition(
			app.activeDocument.activeLayout.viewedRectangle.pX1,
			app.activeDocument.activeLayout.viewedRectangle.pY1,
		);
	}

	public onDraw(frameCount?: number, elapsedTime?: number): void {
		if (
			this.sectionProperties.positionOnMouseDown &&
			this.sectionProperties.selectionSize &&
			this.sectionProperties.active
		) {
			this.context.beginPath();
			this.context.strokeStyle = 'dark grey';
			this.context.setLineDash([3, 3]);
			this.context.strokeRect(
				this.sectionProperties.positionOnMouseDown.pX,
				this.sectionProperties.positionOnMouseDown.pY,
				this.sectionProperties.selectionSize[0],
				this.sectionProperties.selectionSize[1],
			);
			this.context.setLineDash([]);
			this.context.closePath();
		}
	}
}
