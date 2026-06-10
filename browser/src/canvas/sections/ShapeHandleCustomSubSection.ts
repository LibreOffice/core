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
	This class is for custom handlers of shapes.
*/

class ShapeHandleCustomSubSection extends CanvasSectionObject {
    processingOrder: number = app.CSections.DefaultForDocumentObjects.processingOrder;
	drawingOrder: number = app.CSections.DefaultForDocumentObjects.drawingOrder + 1; // Handle events before the parent section.
	zIndex: number = app.CSections.DefaultForDocumentObjects.zIndex;
    documentObject: boolean = true;

	constructor (parentHandlerSection: ShapeHandlesSection, sectionName: string, size: number[], documentPosition: cool.SimplePoint, ownInfo: any) {
        super(sectionName);

        this.size = size;

		this.sectionProperties.position = documentPosition.clone();
		this.sectionProperties.parentHandlerSection = parentHandlerSection;
		this.sectionProperties.ownInfo = ownInfo;

		this.sectionProperties.mousePointerType = 'url(' + app.LOUtil.getURL('images/cursors/grab.svg') + ') 12 12, grab';
	}

	onDraw(frameCount?: number, elapsedTime?: number): void {
		const rtlShift = app.map._docLayer.isCalcRTL() ? this.size[0] : 0;
		this.context.fillStyle = 'yellow';
		this.context.strokeStyle = 'black';
		this.context.beginPath();
		this.context.arc(this.size[0] * 0.5 - rtlShift, this.size[1] * 0.5, this.size[0] * 0.5, 0, Math.PI * 2);
		this.context.closePath();
		this.context.fill();
		this.context.stroke();
	}

	onInitialize(): void {
		this.setPosition(this.sectionProperties.position.pX, this.sectionProperties.position.pY);
	}

	onMouseEnter(point: cool.SimplePoint, e: MouseEvent) {
		this.context.canvas.style.cursor = this.sectionProperties.mousePointerType;
	}

	onMouseUp(point: cool.SimplePoint, e: MouseEvent): void {
		if (this.containerObject.isDraggingSomething()) {
			const isRTL = app.map._docLayer.isCalcRTL();
			const docX = isRTL ? this.position[0] - point.pX : this.position[0] + point.pX;
			const parameters = {
				HandleNum: { type: 'long', value: this.sectionProperties.ownInfo.id },
				NewPosX: { type: 'long', value: Math.round(docX * app.pixelsToTwips) },
				NewPosY: { type: 'long', value: Math.round((point.pY + this.position[1]) * app.pixelsToTwips) }
			};

			app.map.sendUnoCommand('.uno:MoveShapeHandle', parameters);

			this.stopPropagating();
			e.stopPropagation();
		}
	}

	onMouseMove(point: cool.SimplePoint, dragDistance: Array<number>, e: MouseEvent) {
		if (this.containerObject.isDraggingSomething()) {
			this.stopPropagating();
			e.stopPropagation();
		}
	}
}

app.definitions.shapeHandleCustomSubSection = ShapeHandleCustomSubSection;
