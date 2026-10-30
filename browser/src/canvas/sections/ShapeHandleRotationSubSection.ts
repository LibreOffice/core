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

class ShapeHandleRotationSubSection extends ShapeHandleSubSection {
	// The rotate icon, loaded once and kept until the theme asks for another one.
	private static icon: HTMLImageElement | null = null;
	private static iconURL: string | null = null;

	/*
		The rotate icon for the current theme, light or dark. A new one has not loaded yet, so
		the canvas is asked to draw again when loading ends, whether it worked or failed.
	*/
	private static getIcon(): HTMLImageElement {
		const url = app.LOUtil.getImageURL('shape-rotate-handle.svg');
		const kept = ShapeHandleRotationSubSection.icon;

		if (kept !== null && ShapeHandleRotationSubSection.iconURL === url)
			return kept;

		const icon = new Image();
		const redraw = function () { app.sectionContainer.requestReDraw(); };
		icon.src = url;
		icon.onload = redraw;
		icon.onerror = redraw;

		ShapeHandleRotationSubSection.icon = icon;
		ShapeHandleRotationSubSection.iconURL = url;

		return icon;
	}

	constructor (parentHandlerSection: ShapeHandlesSection, sectionName: string, size: number[], documentPosition: cool.SimplePoint, ownInfo: any) {
		super(parentHandlerSection, sectionName, size, documentPosition, ownInfo);

		this.sectionProperties.lastDraggingDistance = null;
		/*
			A turning arrow tells the user what a drag on this handle does. Its hotspot is the
			middle of the arrow, and a browser that will not take the picture shows the hand.
		*/
		this.sectionProperties.cursorStyle =
			'url(' + app.LOUtil.getURL('images/cursors/rotate.svg') + ') 12 12, pointer';

		app.events.on('TextCursorVisibility', this.onTextCursorVisibility.bind(this));
	}

	calculateAngle(center: cool.SimplePoint, target: cool.SimplePoint): number {
		let angle = Math.atan2(target.y - center.y, target.x - center.x);

		while (angle < 0)
			angle += 2 * Math.PI;

		while (angle > Math.PI * 2)
			angle -= Math.PI * 2;

		angle = (angle * 180) / Math.PI;
		angle *= 100; // Core side multiplies degrees with 100.

		return angle;
	}

	onMouseEnter(point: cool.SimplePoint, e: MouseEvent): void {
		this.context.canvas.style.cursor = this.sectionProperties.cursorStyle;
	}

	/*
		A plain circle the size of the handle, drawn while the rotate icon is still loading and
		for as long as it cannot be loaded at all. Dragging works with or without the icon, so
		the handle always shows something the user can grab.
	*/
	private drawCircle(addition: number): void {
		this.context.fillStyle = 'white';
		this.context.strokeStyle = 'black';
		this.context.beginPath();
		this.context.arc(this.size[0] * 0.5 + addition, this.size[1] * 0.5, this.size[0] * 0.5, 0, Math.PI * 2);
		this.context.closePath();
		this.context.fill();
		this.context.stroke();
	}

	onDraw(frameCount?: number, elapsedTime?: number): void {
		const addition = (this.documentObject === true && app.map._docLayer.isCalcRTL()) ? -this.size[0] : 0;
		const icon = ShapeHandleRotationSubSection.getIcon();

		if (icon.complete && icon.naturalWidth > 0)
			this.context.drawImage(icon, addition, 0, this.size[0], this.size[1]);
		else
			this.drawCircle(addition);
	}

	// This is called after dragging the rotation handler. It re-calculates initial angle with the handler's new position.
	getAngleDifference(): number {
		const dragDistanceInTwips = [this.sectionProperties.lastDraggingDistance[0] * app.pixelsToTwips, this.sectionProperties.lastDraggingDistance[1] * app.pixelsToTwips];
		const draggedToPoint = new cool.SimplePoint(dragDistanceInTwips[0], dragDistanceInTwips[1]);

		/*
			The angle is measured to the middle of the handle, which is the point the mouse
			takes hold of. A handle is placed by its upper left corner, so half of its size
			goes onto the point it started from and onto the point the drag moved it to.
		*/
		draggedToPoint.pX += this.position[0] + this.size[0] * 0.5;
		draggedToPoint.pY += this.position[1] + this.size[1] * 0.5;

		Util.ensureValue(GraphicSelection.rectangle);
		const selectionCenter = new cool.SimplePoint(GraphicSelection.rectangle.center[0], GraphicSelection.rectangle.center[1]);

		const initialPoint = this.sectionProperties.ownInfo.initialPosition.clone();
		initialPoint.pX += this.size[0] * 0.5;
		initialPoint.pY += this.size[1] * 0.5;

		const initialAngle = this.calculateAngle(selectionCenter, initialPoint);
		const newAngle = this.calculateAngle(selectionCenter, draggedToPoint);

		return initialAngle - newAngle;
	}

	onMouseUp(point: cool.SimplePoint, e: MouseEvent): void {
		if (this.containerObject.isDraggingSomething()) {
			if (this.sectionProperties.lastDraggingDistance) {
				Util.ensureValue(GraphicSelection.rectangle);
				const center = GraphicSelection.rectangle.center;

				const commandParameters = {
					'TransformRotationDeltaAngle': {
						'type': 'long',
						'value': this.getAngleDifference()
					},
					'TransformRotationX': {
						'type': 'long',
						'value': center[0]
					},
					'TransformRotationY': {
						'type': 'long',
						'value': center[1]
					}
				};

				app.map.sendUnoCommand('.uno:TransformDialog ', commandParameters);
			}
			this.sectionProperties.parentHandlerSection.hideSVG();
		}
	}

	onMouseMove(position: cool.SimplePoint, distance: number[]) {
		if (!this.containerObject.isDraggingSomething()) {
			// Keep the rotate pointer while the mouse is over the handle, because other
			// code may have replaced it.
			if (this.context.canvas.style.cursor !== this.sectionProperties.cursorStyle)
				this.context.canvas.style.cursor = this.sectionProperties.cursorStyle;
			return;
		}

		this.sectionProperties.lastDraggingDistance = distance;

		if (this.sectionProperties.parentHandlerSection.sectionProperties.svg) {
			this.sectionProperties.parentHandlerSection.sectionProperties.svg.style.opacity = 0.5;
			const angleDifference = -this.getAngleDifference() / 100;
			this.sectionProperties.parentHandlerSection.sectionProperties.svg.style.transform = 'rotate(' + angleDifference + 'deg)';
			this.containerObject.requestReDraw();
			this.sectionProperties.parentHandlerSection.showSVG();
		}
	}
}

app.definitions.shapeHandleRotationSubSection = ShapeHandleRotationSubSection;
