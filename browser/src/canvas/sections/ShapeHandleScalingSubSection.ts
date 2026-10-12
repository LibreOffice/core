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

/*
	This class is for the sub sections (handles) of ShapeHandlesSection.
	Shape is rendered on the core side. Only the handles are drawn here and modification commands are sent to the core side.
*/

class ShapeHandleScalingSubSection extends ShapeHandleSubSection {
	constructor(parentHandlerSection: ShapeHandlesSection, sectionName: string, size: number[], documentPosition: cool.SimplePoint, ownInfo: any, cropModeEnabled: boolean) {
		super(parentHandlerSection, sectionName, size, documentPosition, ownInfo);

		this.sectionProperties.mousePointerType = null;

		this.sectionProperties.initialAngle = null; // Initial angle of the point (handle) to the center in radians.
		this.sectionProperties.distanceToCenter = null; // Distance to center.
		this.sectionProperties.cropModeEnabled = cropModeEnabled;
		this.sectionProperties.cropCursor = 'url(' + app.LOUtil.getURL("images/cursors/crop.svg") + ') 8 8, auto';

		this.setMousePointerType();

		app.events.on('TextCursorVisibility', this.onTextCursorVisibility.bind(this));
	}

	/*
		Whether this handle is drawn larger in this moment: it is the one the keyboard works on and
		the blink is at its larger half. The handle stays on the page throughout and changes only
		its size, so what blinks is which handle is meant, not whether there is one.
	*/
	private isTheActiveHandle(): boolean {
		return (
			GraphicSelection.travel()?.showsAsActive(
				this.sectionProperties.ownInfo?.name,
			) ?? false
		);
	}

	onDraw(frameCount?: number, elapsedTime?: number): void {
		this.context.save();
		this.context.setTransform(1, 0, 0, 1, 0, 0);

		this.context.fillStyle = 'white';
		this.context.strokeStyle = 'black';
		this.context.beginPath();

		const multiplier = app.map._docLayer.isCalcRTL() ? -1 : 1;

		if (this.sectionProperties.cropModeEnabled)
			this.drawCropHandles();
		else if (this.isTheActiveHandle()) {
			// A third again in each direction, around the middle of where it would be.
			const grow = this.size[0] / 3;
			this.context.rect(
				this.documentPosition.vX - grow * multiplier,
				this.documentPosition.vY - grow,
				(this.size[0] + 2 * grow) * multiplier,
				this.size[1] + 2 * grow);
		}
		else
			this.context.rect(this.documentPosition.vX, this.documentPosition.vY, this.size[0] * multiplier, this.size[1]);

		this.context.closePath();
		this.context.fill();
		this.context.stroke();

		this.context.restore();
	}

	drawCropCornerHandle() {
		const markerWidth = this.size[0];
		const halfMarkerWidth = markerWidth * 0.5;
		const shapeAngle = this.sectionProperties.parentHandlerSection.sectionProperties.shapeRectangleProperties.angleRadian;
		let x = halfMarkerWidth, y = halfMarkerWidth;
		this.context.translate(x, y);
		this.context.rotate(shapeAngle * -1);
		this.context.translate(-x, -y);
		this.context.moveTo(x, y);
		x += markerWidth;
		this.context.lineTo(x, y);
		y += halfMarkerWidth;
		this.context.lineTo(x, y);
		x -= halfMarkerWidth;
		this.context.lineTo(x, y);
		y += halfMarkerWidth;
		this.context.lineTo(x, y);
		x -= halfMarkerWidth;
		this.context.lineTo(x, y);
	}

	drawCropSideHandle() {
		const markerWidth = this.size[0];
		const halfMarkerWidth = markerWidth * 0.5;
		const shapeAngle = this.sectionProperties.parentHandlerSection.sectionProperties.shapeRectangleProperties.angleRadian;
		let x = halfMarkerWidth, y = halfMarkerWidth;
		this.context.translate(x, y);
		this.context.rotate(shapeAngle * -1);
		this.context.translate(-x, -y);
		this.context.moveTo(x, y);
		x += markerWidth;
		this.context.lineTo(x, y);
		y += halfMarkerWidth;
		this.context.lineTo(x, y);
		x -= markerWidth;
		this.context.lineTo(x, y);
	}

	drawCropHandles() {
		const markerWidth = this.size[0];
		this.context.save();
		// Shift the local-origin markers to this handle's view position.
		this.context.translate(this.documentPosition.vX, this.documentPosition.vY);
		switch (this.sectionProperties.ownInfo.kind) {
			case '1':
				this.drawCropCornerHandle();
				break;
			case '2':
				this.drawCropSideHandle();
				break;
			case '3':
				this.context.rotate(Math.PI / 2);
				this.context.translate(0, -markerWidth);
				this.drawCropCornerHandle();
				break;
			case '4':
				this.context.rotate(-Math.PI / 2);
				this.context.translate(-markerWidth, 0);
				this.drawCropSideHandle();
				break;
			case '5':
				this.context.rotate(Math.PI / 2);
				this.context.translate(0, -markerWidth);
				this.drawCropSideHandle();
				break;
			case '6':
				this.context.rotate(-Math.PI / 2);
				this.context.translate(-markerWidth, 0);
				this.drawCropCornerHandle();
				break;
			case '7':
				this.context.rotate(Math.PI);
				this.context.translate(-markerWidth, -markerWidth);
				this.drawCropSideHandle();
				break;
			case '8':
				this.context.rotate(Math.PI);
				this.context.translate(-markerWidth, -markerWidth);
				this.drawCropCornerHandle();
				break;
		}
		this.context.restore();
	}

	setMousePointerType() {
		if (this.sectionProperties.ownInfo.kind === '1')
			this.sectionProperties.mousePointerType = 'nwse-resize';
		else if (this.sectionProperties.ownInfo.kind === '2')
			this.sectionProperties.mousePointerType = 'ns-resize';
		else if (this.sectionProperties.ownInfo.kind === '3')
			this.sectionProperties.mousePointerType = 'nesw-resize';
		else if (this.sectionProperties.ownInfo.kind === '4')
			this.sectionProperties.mousePointerType = 'ew-resize';
		else if (this.sectionProperties.ownInfo.kind === '5')
			this.sectionProperties.mousePointerType = 'ew-resize';
		else if (this.sectionProperties.ownInfo.kind === '6')
			this.sectionProperties.mousePointerType = 'nesw-resize';
		else if (this.sectionProperties.ownInfo.kind === '7')
			this.sectionProperties.mousePointerType = 'ns-resize';
		else if (this.sectionProperties.ownInfo.kind === '8')
			this.sectionProperties.mousePointerType = 'nwse-resize';
	}

	onMouseEnter(point: cool.SimplePoint, e: MouseEvent) {
		if (this.sectionProperties.cropModeEnabled)
			this.context.canvas.style.cursor = this.sectionProperties.cropCursor;
		else if (GraphicSelection.extraInfo?.isResizable === false)
			this.context.canvas.style.cursor = 'not-allowed';
		else
			this.context.canvas.style.cursor = this.sectionProperties.mousePointerType;
	}

	// In Calc RTL the canvas is mirrored, so the section-local mouse offset
	// runs opposite to the doc-X axis. Convert the section-local point.pX
	// into the doc-pixel X of the mouse cursor, the same coordinate system
	// that this.position[0] and shapeRectangleProperties live in.
	private mouseToDocX(point: cool.SimplePoint): number {
		return app.map._docLayer.isCalcRTL()
			? this.position[0] - point.pX
			: this.position[0] + point.pX;
	}

	onMouseUp(point: cool.SimplePoint, e: MouseEvent): void {
		if (this.containerObject.isDraggingSomething()) {
			Util.ensureValue(app.activeDocument);
			this.stopPropagating();
			e.stopPropagation();

			const ownInfo = this.sectionProperties.ownInfo;
			// The handles are counted from zero here, where a handle that names itself counts the
			// kinds from one, so the maths below works on the number either way.
			const handleId = ownInfo.name ? Number(ownInfo.kind) - 1 : ownInfo.id;
			const parentHandlerSection = this.sectionProperties.parentHandlerSection;

			const p = point.clone();
			p.pX = this.mouseToDocX(point);
			p.pY += this.position[1];

			const keepRatio = HandleScaling.keepsRatio(e, this.sectionProperties.cropModeEnabled);
			const shapeRecProps = HandleScaling.shapeAfterDrag(
				p, parentHandlerSection.sectionProperties.shapeRectangleProperties,
				ownInfo.kind, keepRatio);

			const tempRectangle = cool.SimpleRectangle.fromCorePixels([
				shapeRecProps.center.pX - shapeRecProps.width * 0.5,
				shapeRecProps.center.pY - shapeRecProps.height * 0.5,
				shapeRecProps.width, shapeRecProps.height
			]);

			if (GraphicSelection.inPlaceActive) {
				parentHandlerSection.sectionProperties.inPlaceResizeRectangle = null;
				GraphicSelection.sendInPlaceArea(tempRectangle);
				parentHandlerSection.hideSVG();
				return;
			}

			if (GraphicSelection.hasDarkOverlay()) {
				parentHandlerSection.sectionProperties.inPlaceResizeRectangle = null;
				GraphicSelection.sendElementArea(tempRectangle);
				parentHandlerSection.hideSVG();
				return;
			}

			const committedHandleId = HandleScaling.committedHandle(handleId, keepRatio);
			const newPoint = HandleScaling.positionOfHandle(committedHandleId, tempRectangle);

			if (!keepRatio) {
				const snapped = parentHandlerSection.snappedAt();
				newPoint[0] = Math.round((snapped[0] ?? this.mouseToDocX(point)) * app.pixelsToTwips);
				newPoint[1] = Math.round((snapped[1] ?? point.pY + this.position[1]) * app.pixelsToTwips);
			}

			const parameters = {
				...ShapeHandlesSection.handleParameters(
					ownInfo.name
						? { name: String(Number(committedHandleId) + 1) + '.0.0' }
						: { id: committedHandleId }),
				NewPosX: { type: 'long', value: newPoint[0] },
				NewPosY: { type: 'long', value: newPoint[1] }
			};

			app.map.sendUnoCommand('.uno:MoveShapeHandle', parameters);
			parentHandlerSection.hideSVG();
		}
	}

	// While dragging a handle, we want to simulate handles to their final positions.
	moveHandlesOnDrag(point: cool.SimplePoint, e: MouseEvent) {
		Util.ensureValue(app.activeDocument);

		const p = point.clone();
		p.pX = this.mouseToDocX(point);
		p.pY += this.position[1];

		const shapeRecProps = HandleScaling.shapeAfterDrag(
			p, this.sectionProperties.parentHandlerSection.sectionProperties.shapeRectangleProperties,
			this.sectionProperties.ownInfo.kind,
			HandleScaling.keepsRatio(e, this.sectionProperties.cropModeEnabled));

		this.sectionProperties.parentHandlerSection.calculateInitialAnglesOfShapeHandlers(shapeRecProps);

		const halfWidth = this.sectionProperties.parentHandlerSection.sectionProperties.handleWidth * 0.5;
		const halfHeight = this.sectionProperties.parentHandlerSection.sectionProperties.handleHeight * 0.5;
		const subSections = this.sectionProperties.parentHandlerSection.sectionProperties.subSections;

		let x = 0, y = 0;
		let pointAngle = 0;

		for (let i = 0; i < subSections.length; i++) {
			const subSection = subSections[i];

			pointAngle = subSection.sectionProperties.initialAngle + shapeRecProps.angleRadian;
			x = shapeRecProps.center.pX + subSection.sectionProperties.distanceToCenter * Math.cos(pointAngle);
			y = shapeRecProps.center.pY - subSection.sectionProperties.distanceToCenter * Math.sin(pointAngle);
			subSection.setPosition(x - halfWidth, y - halfHeight);
		}

		if (GraphicSelection.inPlaceActive || GraphicSelection.hasDarkOverlay()) {
			const width = Math.abs(shapeRecProps.width);
			const height = Math.abs(shapeRecProps.height);
			this.sectionProperties.parentHandlerSection.sectionProperties.inPlaceResizeRectangle = cool.SimpleRectangle.fromCorePixels([
				shapeRecProps.center.pX - width * 0.5,
				shapeRecProps.center.pY - height * 0.5,
				width, height
			]);
		}

		if (!this.sectionProperties.cropModeEnabled)
			this.sectionProperties.parentHandlerSection.scalePreviewTo(shapeRecProps);
	}

	onMouseMove(point: cool.SimplePoint, dragDistance: Array<number>, e: MouseEvent) {
		if (this.containerObject.isDraggingSomething()) {
			this.stopPropagating();
			e.stopPropagation();

			if (!this.sectionProperties.cropModeEnabled && GraphicSelection.extraInfo?.isResizable === false)
				return;

			const parentSVG =
				this.sectionProperties.parentHandlerSection.sectionProperties.svg;
			if (parentSVG) parentSVG.style.opacity = 0.5;

			const parentProperties = this.sectionProperties.parentHandlerSection.sectionProperties;
			const snapPosition = [
				this.mouseToDocX(point) - parentProperties.handleWidth * 0.5,
				point.pY + this.position[1] - parentProperties.handleHeight * 0.5
			];

			this.moveHandlesOnDrag(point, e);

			// Here we are checking a point, so the size 0. dragDistance is also 0 because we already set the new position (moveHandlesOnDrag).
			this.sectionProperties.parentHandlerSection.checkHelperLinesAndSnapPoints([0, 0], snapPosition, [0, 0]);

			this.containerObject.requestReDraw();
			this.sectionProperties.parentHandlerSection.showSVG();
		}
	}
}
