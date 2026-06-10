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

class CellSelectionHandle extends CanvasSectionObject {
	showSection: boolean = false;
	processingOrder: number = app.CSections.DefaultForDocumentObjects.processingOrder;
	drawingOrder: number = app.CSections.DefaultForDocumentObjects.drawingOrder;
	zIndex: number = app.CSections.DefaultForDocumentObjects.zIndex;
	documentObject: boolean = true;

	constructor (name: string) {
        super(name);

		this.sectionProperties.circleRadius = 10 * app.dpiScale;
		this.size = [this.sectionProperties.circleRadius * 2, this.sectionProperties.circleRadius * 2];
	}

	private onDragEnd(point: cool.SimplePoint) {
		app.map.focus();
		app.map.fire('scrollvelocity', {vx: 0, vy: 0});

		point.pX += this.position[0];
		point.pY += this.position[1];

		this.sharedOnDragAndEnd(point);
		CellSelectionMarkers.update();
		app.map.scrollingIsHandled = false;
	}

	private sharedOnDragAndEnd(point: cool.SimplePoint) {
		const type = this.name === 'cell_selection_handle_start' ? 'start' : 'end';
		app.map._docLayer._postSelectTextEvent(type, point.x, point.y);
	}

	private onDrag(point: cool.SimplePoint) {
		point.pX += this.position[0];
		point.pY += this.position[1];

		app.map.fire('handleautoscroll', { pos: { x: point.cX, y: point.cY }, map: app.map });

		this.sharedOnDragAndEnd(point);
	}

	public onDraw() {
		this.context.strokeStyle = (<any>window).prefs.getBoolean('darkTheme') ? 'white' : 'black';
		this.context.lineWidth = 2;

		this.context.beginPath();
		this.context.arc(this.sectionProperties.circleRadius, this.sectionProperties.circleRadius, this.sectionProperties.circleRadius, 0, 2 * Math.PI);
		this.context.stroke();
	}

	onMouseMove(point: cool.SimplePoint, dragDistance: number[], e: MouseEvent): void {
		e.stopPropagation();
		if (this.containerObject.isDraggingSomething()) {
			app.map.scrollingIsHandled = true;
			this.stopPropagating();
			this.onDrag(point);
		}
	}

	onMouseDown(point: cool.SimplePoint, e: MouseEvent): void {
		e.stopPropagation();
		this.stopPropagating();
	}

	onMouseUp(point: cool.SimplePoint, e: MouseEvent): void {
		e.stopPropagation();
		if (this.containerObject.isDraggingSomething()) {
			this.stopPropagating();
			this.onDragEnd(point);
		}
	}
}
