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

	// The object this handle sits on, numbered from zero. Each selection message brings new
	// handle information, so the number is read fresh.
	private getOwnObjectNumber(): number {
		return this.sectionProperties.ownInfo.objectNumber ?? 0;
	}

	/*
		Inner rectangle, rotation angle and center of the object this handle sits on. A selection
		can hold several objects, each with its own eight handles.
	*/
	private getOwnShapeRectangleProperties() {
		return this.sectionProperties.parentHandlerSection.sectionProperties
			.shapeRectangleProperties[this.getOwnObjectNumber()];
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
		const shapeAngle = this.getOwnShapeRectangleProperties().angleRadian;
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
		const shapeAngle = this.getOwnShapeRectangleProperties().angleRadian;
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

	/*
		Where a drop at the given point leads: the object's new rectangle, the handle the move is
		committed on, and the twips position that handle lands on. The preview and the drop both
		read it, so the preview shows the geometry the drop produces.
	*/
	private dropOfDrag(point: cool.SimplePoint, e: MouseEvent): any {
		const ownInfo = this.sectionProperties.ownInfo;
		/*
			Which corner or edge of its own object the handle sits on, counted from zero. The kind
			numbers these from "1" for the upper left one to "8" for the lower right, while the id
			counts every handle the object made.
		*/
		const handleId = String(Number(ownInfo.kind) - 1);

		const p = point.clone();
		p.pX = this.mouseToDocX(point);
		p.pY += this.position[1];

		const keepRatio = HandleScaling.keepsRatio(e, this.sectionProperties.cropModeEnabled);
		const shapeRecProps = HandleScaling.shapeAfterDrag(
			p, this.getOwnShapeRectangleProperties(), ownInfo.kind, keepRatio);

		const rectangle = cool.SimpleRectangle.fromCorePixels([
			shapeRecProps.center.pX - shapeRecProps.width * 0.5,
			shapeRecProps.center.pY - shapeRecProps.height * 0.5,
			shapeRecProps.width, shapeRecProps.height
		]);

		const committedHandleId = HandleScaling.committedHandle(handleId, keepRatio);
		const position = HandleScaling.positionOfHandle(committedHandleId, rectangle);

		if (!keepRatio) {
			const snapped = this.sectionProperties.parentHandlerSection.snappedAt();
			position[0] = Math.round((snapped[0] ?? this.mouseToDocX(point)) * app.pixelsToTwips);
			position[1] = Math.round((snapped[1] ?? point.pY + this.position[1]) * app.pixelsToTwips);
		}

		return { committedHandleId: committedHandleId, rectangle: rectangle, position: position };
	}

	onMouseUp(point: cool.SimplePoint, e: MouseEvent): void {
		if (this.containerObject.isDraggingSomething()) {
			Util.ensureValue(app.activeDocument);
			this.stopPropagating();
			e.stopPropagation();

			const ownInfo = this.sectionProperties.ownInfo;
			const parentHandlerSection = this.sectionProperties.parentHandlerSection;
			const drop = this.dropOfDrag(point, e);
			const committedHandleId = drop.committedHandleId;
			const newPoint = drop.position;

			if (GraphicSelection.inPlaceActive) {
				parentHandlerSection.sectionProperties.inPlaceResizeRectangle = null;
				GraphicSelection.sendInPlaceArea(drop.rectangle);
				parentHandlerSection.hideSVG();
				return;
			}

			if (GraphicSelection.hasDarkOverlay()) {
				parentHandlerSection.sectionProperties.inPlaceResizeRectangle = null;
				GraphicSelection.sendElementArea(drop.rectangle);
				parentHandlerSection.hideSVG();
				return;
			}

			const parameters = {
				...ShapeHandlesSection.handleParameters(
					ownInfo.name
						? { name: String(Number(committedHandleId) + 1) + '.0.0' }
						: parentHandlerSection.committedScalingHandle(
							ownInfo, String(Number(committedHandleId) + 1))),
				NewPosX: { type: 'long', value: newPoint[0] },
				NewPosY: { type: 'long', value: newPoint[1] }
			};

			app.map.sendUnoCommand('.uno:MoveShapeHandle', parameters);
			parentHandlerSection.hideSVG();
			parentHandlerSection.clearShapeDragPreview();
		}
	}

	// While dragging a handle, we want to simulate handles to their final positions.
	moveHandlesOnDrag(point: cool.SimplePoint, e: MouseEvent) {
		Util.ensureValue(app.activeDocument);

		const p = point.clone();
		p.pX = this.mouseToDocX(point);
		p.pY += this.position[1];

		const objectNumber = this.getOwnObjectNumber();
		const shapeRecProps = HandleScaling.shapeAfterDrag(
			p, this.getOwnShapeRectangleProperties(),
			this.sectionProperties.ownInfo.kind,
			HandleScaling.keepsRatio(e, this.sectionProperties.cropModeEnabled));

		this.sectionProperties.parentHandlerSection.calculateInitialAnglesOfShapeHandlers(shapeRecProps, objectNumber);

		const halfWidth = this.sectionProperties.parentHandlerSection.sectionProperties.handleWidth * 0.5;
		const halfHeight = this.sectionProperties.parentHandlerSection.sectionProperties.handleHeight * 0.5;
		const subSections = this.sectionProperties.parentHandlerSection.sectionProperties.subSections;

		let x = 0, y = 0;
		let pointAngle = 0;

		for (let i = 0; i < subSections.length; i++) {
			const subSection = subSections[i];

			// Only the scaling handles carry a distance and an angle to the shape's center for
			// this loop to follow. A rotation handle gets its position another way, so it is
			// skipped here.
			if (!['1', '2', '3', '4', '5', '6', '7', '8']
				.includes(subSection.sectionProperties.ownInfo.kind))
				continue;

			// The other selected objects keep their handles where they are. Core resizes them
			// too, and their new positions arrive with the next selection message.
			if ((subSection.sectionProperties.ownInfo.objectNumber ?? 0) !== objectNumber)
				continue;

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

		if (!this.sectionProperties.cropModeEnabled
			&& !this.sectionProperties.parentHandlerSection.usesCoreDragPreview())
			this.sectionProperties.parentHandlerSection.scalePreviewTo(shapeRecProps, objectNumber);
	}

	onMouseMove(point: cool.SimplePoint, dragDistance: Array<number>, e: MouseEvent) {
		if (this.containerObject.isDraggingSomething()) {
			this.stopPropagating();
			e.stopPropagation();

			if (!this.sectionProperties.cropModeEnabled && GraphicSelection.extraInfo?.isResizable === false)
				return;

			const parentHandlerSection = this.sectionProperties.parentHandlerSection;
			const parentProperties = parentHandlerSection.sectionProperties;
			const useCorePreview = parentHandlerSection.usesCoreDragPreview();

			const parentSVG = parentProperties.svg;
			if (!useCorePreview && parentSVG) parentSVG.style.opacity = 0.5;

			const snapPosition = [
				this.mouseToDocX(point) - parentProperties.handleWidth * 0.5,
				point.pY + this.position[1] - parentProperties.handleHeight * 0.5
			];

			this.moveHandlesOnDrag(point, e);

			// Here we are checking a point, so the size 0. dragDistance is also 0 because we already set the new position (moveHandlesOnDrag).
			parentHandlerSection.checkHelperLinesAndSnapPoints([0, 0], snapPosition, [0, 0]);

			this.containerObject.requestReDraw();

			if (useCorePreview) {
				// The snap points are ready by now, so this asks core to preview the position a
				// drop here would give.
				const dropPosition = this.dropOfDrag(point, e).position;
				parentHandlerSection.requestShapeDragPreview(
					this.sectionProperties.ownInfo,
					new cool.SimplePoint(dropPosition[0], dropPosition[1]));
			}
			else
				parentHandlerSection.showSVG();
		}
	}
}
