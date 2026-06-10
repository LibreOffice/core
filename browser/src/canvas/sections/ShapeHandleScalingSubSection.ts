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

class ShapeHandleScalingSubSection extends CanvasSectionObject {
	processingOrder: number = app.CSections.DefaultForDocumentObjects.processingOrder;
	drawingOrder: number = app.CSections.DefaultForDocumentObjects.drawingOrder + 1; // Handle events before the parent section.
	zIndex: number = app.CSections.DefaultForDocumentObjects.zIndex;
	documentObject: boolean = true;

	constructor(parentHandlerSection: ShapeHandlesSection, sectionName: string, size: number[], documentPosition: cool.SimplePoint, ownInfo: any, cropModeEnabled: boolean) {
		super(sectionName);

		this.size = size;
		this.sectionProperties.position = documentPosition.clone();

		this.sectionProperties.parentHandlerSection = parentHandlerSection;
		this.sectionProperties.ownInfo = ownInfo;
		this.sectionProperties.mousePointerType = null;

		this.sectionProperties.initialAngle = null; // Initial angle of the point (handle) to the center in radians.
		this.sectionProperties.distanceToCenter = null; // Distance to center.
		this.sectionProperties.cropModeEnabled = cropModeEnabled;
		this.sectionProperties.cropCursor = 'url(' + app.LOUtil.getURL("images/cursors/crop.svg") + ') 8 8, auto';

		this.setMousePointerType();

		app.events.on('TextCursorVisibility', this.onTextCursorVisibility.bind(this));
	}

	onInitialize(): void {
		this.setPosition(this.sectionProperties.position.pX, this.sectionProperties.position.pY);
	}

	onTextCursorVisibility(event: any): void {
		if (event.detail.visible) {
			this.setShowSection(false);
			this.interactable = false;
		}
		else {
			this.setShowSection(true);
			this.interactable = true;
		}
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
		else
			this.context.canvas.style.cursor = this.sectionProperties.mousePointerType;
	}

	private doWeKeepRatio(e: MouseEvent) {
		if (this.sectionProperties.cropModeEnabled)
			return false;

		let keep = e.ctrlKey && e.shiftKey;

		// For images, the keepRatio shortcut works the opposite way.
		if (app.map.context && app.map.context.context === 'Graphic')
			keep = !keep;

		return keep;
	}

	private getNewPosition(handleID: string, rectangle: cool.SimpleRectangle): number[] {
		const isRTL = app.map._docLayer.isCalcRTL();
		const leftHandleX = isRTL ? rectangle.x2 : rectangle.x1;
		const rightHandleX = isRTL ? rectangle.x1 : rectangle.x2;

		if (handleID === '0')
			return [leftHandleX, rectangle.y1];
		else if (handleID === '1')
			return [rectangle.center[0], rectangle.y1];
		else if (handleID === '2')
			return [rightHandleX, rectangle.y1];
		else if (handleID === '3')
			return [leftHandleX, rectangle.center[1]];
		else if (handleID === '4')
			return [rightHandleX, rectangle.center[1]];
		else if (handleID === '5')
			return [leftHandleX, rectangle.y2];
		else if (handleID === '6')
			return [rectangle.center[0], rectangle.y2];
		else // handleID === '7'
			return [rightHandleX, rectangle.y2];
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

			const handleId = this.sectionProperties.ownInfo.id;
			const parentHandlerSection = this.sectionProperties.parentHandlerSection;

			const p = point.clone();
			p.pX = this.mouseToDocX(point);
			p.pY += this.position[1];

			const shapeRecProps = this.calculateNewShapeRectangleProperties(p, e);

			const tempRectangle = cool.SimpleRectangle.fromCorePixels([
				shapeRecProps.center.pX - shapeRecProps.width * 0.5,
				shapeRecProps.center.pY - shapeRecProps.height * 0.5,
				shapeRecProps.width, shapeRecProps.height
			]);

			const newPoint = this.getNewPosition(handleId, tempRectangle);

			if (!this.doWeKeepRatio(e) || ["1", "3", "4", "6"].includes(handleId)) {
				newPoint[0] = Math.round((parentHandlerSection.sectionProperties.closestX ?? this.mouseToDocX(point)) * app.pixelsToTwips);
				newPoint[1] = Math.round((parentHandlerSection.sectionProperties.closestY ?? point.pY + this.position[1]) * app.pixelsToTwips);
			}

			const parameters = {
				HandleNum: { type: 'long', value: handleId },
				NewPosX: { type: 'long', value: newPoint[0] },
				NewPosY: { type: 'long', value: newPoint[1] }
			};

			app.map.sendUnoCommand('.uno:MoveShapeHandle', parameters);
			parentHandlerSection.hideSVG();
		}
	}

	adjustSVGProperties(shapeRecProps: any) {
		if (this.sectionProperties.parentHandlerSection.sectionProperties.svg) {
			const svg = this.sectionProperties.parentHandlerSection.sectionProperties.svg;

			const scaleX = shapeRecProps.width / this.sectionProperties.parentHandlerSection.sectionProperties.shapeRectangleProperties.width;
			const scaleY = shapeRecProps.height / this.sectionProperties.parentHandlerSection.sectionProperties.shapeRectangleProperties.height;

			let diffX = shapeRecProps.center.pX - this.sectionProperties.parentHandlerSection.sectionProperties.shapeRectangleProperties.center.pX;
			let diffY = shapeRecProps.center.pY - this.sectionProperties.parentHandlerSection.sectionProperties.shapeRectangleProperties.center.pY;

			diffX = diffX / app.dpiScale;
			diffY = diffY / app.dpiScale;

			svg.children[0].style.transform = 'translate(' + Math.round(diffX) + 'px, ' + Math.round(diffY) + 'px)' + 'rotate(' + -shapeRecProps.angleRadian + 'rad) scale(' + scaleX + ', ' + scaleY + ') rotate(' + shapeRecProps.angleRadian + 'rad)';

			this.sectionProperties.parentHandlerSection.showSVG();
		}
	}

	// Uses the given "point" parameter and modifies it.
	private calculateRatioPoint(point: cool.SimplePoint, shapeRecProps: any) {
		const isVerticalHandler = ['2', '7'].includes(this.sectionProperties.ownInfo.kind);

		const primaryDelta = isVerticalHandler
			? point.pY - shapeRecProps.center.pY
			: point.pX - shapeRecProps.center.pX;

		const aspectRatio = isVerticalHandler
			? shapeRecProps.width / shapeRecProps.height
			: shapeRecProps.height / shapeRecProps.width;

		// The kind-based direction table assumes LTR doc-X. In RTL, the abs in
		// convertToTileTwipsIfNeeded mirrors the X axis (kinds 1/4/6 sit at
		// doc-max-X, 3/5/8 at doc-min-X), so the secondary axis runs opposite
		// to user intent.
		let secondaryDelta = primaryDelta * aspectRatio;
		if (app.map._docLayer.isCalcRTL())
			secondaryDelta = -secondaryDelta;

		const direction = ['3', '4', '6', '2'].includes(this.sectionProperties.ownInfo.kind) ? -1 : 1;

		if (isVerticalHandler)
			point.pX = shapeRecProps.center.pX + secondaryDelta * direction;
		else
			point.pY = shapeRecProps.center.pY + secondaryDelta * direction;
	}

	calculateNewShapeRectangleProperties(point: cool.SimplePoint, e: MouseEvent) {
		const shapeRecProps: any = structuredClone(this.sectionProperties.parentHandlerSection.sectionProperties.shapeRectangleProperties);
		shapeRecProps.center = this.sectionProperties.parentHandlerSection.sectionProperties.shapeRectangleProperties.center.clone();
		const keepRatio = this.doWeKeepRatio(e);

		if (keepRatio)
			this.calculateRatioPoint(point, shapeRecProps);

		const diff = [point.pX - shapeRecProps.center.pX, -(point.pY - shapeRecProps.center.pY)];
		const length = Math.pow(Math.pow(diff[0], 2) + Math.pow(diff[1], 2), 0.5);
		const pointAngle = Math.atan2(diff[1], diff[0]);
		point.pX = shapeRecProps.center.pX + length * Math.cos(pointAngle - shapeRecProps.angleRadian);
		point.pY = shapeRecProps.center.pY - length * Math.sin(pointAngle - shapeRecProps.angleRadian);

		const rectangle = new cool.SimpleRectangle(
			(shapeRecProps.center.pX - shapeRecProps.width * 0.5) * app.pixelsToTwips,
			(shapeRecProps.center.pY - shapeRecProps.height * 0.5) * app.pixelsToTwips,
			shapeRecProps.width * app.pixelsToTwips,
			shapeRecProps.height * app.pixelsToTwips
		);

		const oldpCenter = rectangle.pCenter;

		// In RTL, convertToTileTwipsIfNeeded's abs flips handle doc-X: kinds
		// 1/4/6 land at doc-max-X (= pX2 side) and 3/5/8 at doc-min-X (pX1).
		// So the edge each handle modifies is swapped relative to LTR.
		const isRTL = app.map._docLayer.isCalcRTL();
		const isMinXHandle = isRTL
			? ['3', '5', '8'].includes(this.sectionProperties.ownInfo.kind)
			: ['1', '4', '6'].includes(this.sectionProperties.ownInfo.kind);
		const isMaxXHandle = isRTL
			? ['1', '4', '6'].includes(this.sectionProperties.ownInfo.kind)
			: ['3', '5', '8'].includes(this.sectionProperties.ownInfo.kind);

		if (isMinXHandle) {
			const pX2 = rectangle.pX2;
			rectangle.pX1 = point.pX;
			rectangle.pX2 = pX2;
		}
		else if (isMaxXHandle)
			rectangle.pX2 = point.pX;

		if (['1', '2', '3'].includes(this.sectionProperties.ownInfo.kind)) {
			const pY2 = rectangle.pY2;
			rectangle.pY1 = point.pY;
			rectangle.pY2 = pY2;
		}
		else if (['6', '7', '8'].includes(this.sectionProperties.ownInfo.kind))
			rectangle.pY2 = point.pY;

		if (keepRatio) {
			if (['4', '5'].includes(this.sectionProperties.ownInfo.kind)) {
				rectangle.pY2 = point.pY;
			} else if (['2', '7'].includes(this.sectionProperties.ownInfo.kind)) {
				if (isRTL) {
					const pX2 = rectangle.pX2;
					rectangle.pX1 = point.pX;
					rectangle.pX2 = pX2;
				} else {
					rectangle.pX2 = point.pX;
				}
			}
		}

		const centerAngle = Math.atan2(oldpCenter[1] - rectangle.pCenter[1], rectangle.pCenter[0] - oldpCenter[0]);
		const centerLength = Math.pow(Math.pow(rectangle.pCenter[1] - oldpCenter[1], 2) + Math.pow(rectangle.pCenter[0] - oldpCenter[0], 2), 0.5);

		const x = centerLength * Math.cos(shapeRecProps.angleRadian + centerAngle);
		const y = centerLength * Math.sin(shapeRecProps.angleRadian + centerAngle);

		shapeRecProps.center.pX += x;
		shapeRecProps.center.pY -= y;
		shapeRecProps.width = rectangle.pWidth;
		shapeRecProps.height = rectangle.pHeight;

		return shapeRecProps;
	}

	// While dragging a handle, we want to simulate handles to their final positions.
	moveHandlesOnDrag(point: cool.SimplePoint, e: MouseEvent) {
		Util.ensureValue(app.activeDocument);

		const p = point.clone();
		p.pX = this.mouseToDocX(point);
		p.pY += this.position[1];

		const shapeRecProps = this.calculateNewShapeRectangleProperties(p, e);

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

		if (!this.sectionProperties.cropModeEnabled)
			this.adjustSVGProperties(shapeRecProps);
	}

	onMouseMove(point: cool.SimplePoint, dragDistance: Array<number>, e: MouseEvent) {
		if (this.containerObject.isDraggingSomething()) {
			this.stopPropagating();
			e.stopPropagation();
			this.sectionProperties.parentHandlerSection.sectionProperties.svg.style.opacity = 0.5;
			this.moveHandlesOnDrag(point, e);

			// Here we are checking a point, so the size 0. dragDistance is also 0 because we already set the new position (moveHandlesOnDrag).
			this.sectionProperties.parentHandlerSection.checkHelperLinesAndSnapPoints([0, 0], this.position, [0, 0]);

			this.containerObject.requestReDraw();
			this.sectionProperties.parentHandlerSection.showSVG();
		}
	}
}
