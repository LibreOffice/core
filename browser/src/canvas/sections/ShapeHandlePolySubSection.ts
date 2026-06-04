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

class ShapeHandlePolySubSection extends ShapeHandleCustomSubSection {
	// Object type of connectors. Matches SdrObjKind::Edge in core.
	private static readonly connectorObjectType: number = 24;

	constructor (parentHandlerSection: ShapeHandlesSection, sectionName: string, size: number[], documentPosition: cool.SimplePoint, ownInfo: any) {
        super(parentHandlerSection, sectionName, size, documentPosition, ownInfo);
		this.sectionProperties.mousePointerType = 'move';
	}

	private isConnectorHandle(): boolean {
		return GraphicSelection.extraInfo?.type === ShapeHandlePolySubSection.connectorObjectType;
	}

	onMouseMove(point: cool.SimplePoint, dragDistance: Array<number>, e: MouseEvent) {
		if (this.containerObject.isDraggingSomething()) {
			this.stopPropagating();
			e.stopPropagation();

			if (this.isConnectorHandle()) {
				// Ask core for the line the connector would get if the
				// handle was dropped here, and draw that as the preview.
				const mousePoint = point.clone();
				mousePoint.pX += this.position[0];
				mousePoint.pY += this.position[1];
				this.sectionProperties.parentHandlerSection.requestShapeDragPreview(this.sectionProperties.ownInfo.id, mousePoint);
			}
		}
	}

	onMouseUp(point: cool.SimplePoint, e: MouseEvent): void {
		const wasDragging = this.containerObject.isDraggingSomething();
		super.onMouseUp(point, e);

		if (wasDragging)
			this.sectionProperties.parentHandlerSection.clearShapeDragPreview();
	}
}

app.definitions.shapeHandlePolySubSection = ShapeHandlePolySubSection;
