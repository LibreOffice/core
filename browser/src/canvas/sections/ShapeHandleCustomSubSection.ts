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

/*
	This class is for custom handlers of shapes.
*/

class ShapeHandleCustomSubSection extends ShapeHandleSubSection {
	constructor (parentHandlerSection: ShapeHandlesSection, sectionName: string, size: number[], documentPosition: cool.SimplePoint, ownInfo: any) {
		super(parentHandlerSection, sectionName, size, documentPosition, ownInfo);

		this.sectionProperties.mousePointerType = 'url(' + app.LOUtil.getURL('images/cursors/grab.svg') + ') 12 12, grab';
	}

	onDraw(frameCount?: number, elapsedTime?: number): void {
		// Larger while the handle the keyboard works on is at the larger half of its blink. It
		// stays on the page throughout and changes only its size.
		const active = GraphicSelection.travel()?.showsAsActive(
			this.sectionProperties.ownInfo?.name,
		);

		const rtlShift = app.map._docLayer.isCalcRTL() ? this.size[0] : 0;
		this.context.fillStyle = 'yellow';
		this.context.strokeStyle = 'black';
		this.context.beginPath();
		// The handle the keyboard works on is drawn a third larger.
		const radius = this.size[0] * (active ? 0.5 * 1.33 : 0.5);
		this.context.arc(this.size[0] * 0.5 - rtlShift, this.size[1] * 0.5, radius, 0, Math.PI * 2);
		this.context.closePath();
		this.context.fill();
		this.context.stroke();
	}

	onMouseEnter(point: cool.SimplePoint, e: MouseEvent) {
		this.context.canvas.style.cursor = this.sectionProperties.mousePointerType;
	}

	onMouseUp(point: cool.SimplePoint, e: MouseEvent): void {
		if (this.containerObject.isDraggingSomething()) {
			const isRTL = app.map._docLayer.isCalcRTL();
			const docX = isRTL ? this.position[0] - point.pX : this.position[0] + point.pX;
			const parameters = {
				...ShapeHandlesSection.handleParameters(this.sectionProperties.ownInfo),
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
