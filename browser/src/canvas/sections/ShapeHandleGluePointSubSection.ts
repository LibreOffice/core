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

class ShapeHandleGluePointSubSection extends CanvasSectionObject {
    processingOrder: number = app.CSections.DefaultForDocumentObjects.processingOrder;
	drawingOrder: number = app.CSections.DefaultForDocumentObjects.drawingOrder + 1; // Handle events before the parent section.
	zIndex: number = app.CSections.DefaultForDocumentObjects.zIndex;
    documentObject: boolean = true;

	constructor (parentHandlerSection: ShapeHandlesSection, sectionName: string, size: number[], documentPosition: cool.SimplePoint, ownInfo: any) {
        super(sectionName);

        this.size = size;
        this.position = [documentPosition.pX, documentPosition.pY];

		this.sectionProperties.parentHandlerSection = parentHandlerSection;
		this.sectionProperties.ownInfo = ownInfo;
	}

    onDraw(frameCount?: number, elapsedTime?: number): void {
        this.context.fillStyle = '#EE3E3E';
        this.context.beginPath();
        this.context.arc(0, 0, this.size[0] * 0.5, 0, Math.PI * 2);
        this.context.fill();
    }
}

app.definitions.shapeHandleGluePointSubSection = ShapeHandleGluePointSubSection;
