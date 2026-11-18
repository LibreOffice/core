/*
 * Copyright the Collabora Office contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

class PixelGridSection extends CanvasSectionObject {
    interactable: boolean = false;
    anchor: string[] = ['top', 'left'];
    processingOrder: number = app.CSections.Debug.TilePixelGrid.processingOrder;
    drawingOrder: number = app.CSections.Debug.TilePixelGrid.drawingOrder;
    zIndex: number = app.CSections.Debug.TilePixelGrid.zIndex;
    boundToSection: string = 'tiles';

    constructor () { super(app.CSections.Debug.TilePixelGrid.name); }

    onDraw(frameCount?: number, elapsedTime?: number): void {
		var offset = 8;
		var count;
		this.context.lineWidth = 1;
		var currentPos;
		this.context.strokeStyle = '#ff0000';

		currentPos = 0;
		count = Math.round(this.context.canvas.height / offset);
		for (var i = 0; i < count; i++) {
			this.context.beginPath();
			this.context.moveTo(0.5, currentPos + 0.5);
			this.context.lineTo(this.context.canvas.width + 0.5, currentPos + 0.5);
			this.context.stroke();
			currentPos += offset;
		}

		currentPos = 0;
		count = Math.round(this.context.canvas.width / offset);
		for (var i = 0; i < count; i++) {
			this.context.beginPath();
			this.context.moveTo(currentPos + 0.5, 0.5);
			this.context.lineTo(currentPos + 0.5, this.context.canvas.height + 0.5);
			this.context.stroke();
			currentPos += offset;
		}
    }
}

app.definitions.pixelGridSection = PixelGridSection;
