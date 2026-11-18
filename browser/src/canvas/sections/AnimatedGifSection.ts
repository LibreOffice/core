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
 * AnimatedGifSection draws the animated frames of one image (for example an
 * animated GIF) over the document, on top of the static first frame that the
 * tiles carry.
 */

class AnimatedGifSection extends CanvasSectionObject {
	processingOrder: number = app.CSections.AnimatedGif.processingOrder;
	drawingOrder: number = app.CSections.AnimatedGif.drawingOrder;
	zIndex: number = app.CSections.AnimatedGif.zIndex;
	documentObject: boolean = true;
	interactable: boolean = false;
	mirror: number = 0;

	constructor(sectionName: string, twipRectangle: number[], url: string) {
		super(sectionName);
		this.sectionProperties.twipRectangle = twipRectangle;
		this.sectionProperties.url = url;
		this.sectionProperties.source = new GifFrameSource(url, () => {
			app.sectionContainer.requestReDraw();
		});
	}

	public getUrl(): string {
		return this.sectionProperties.url;
	}

	onInitialize(): void {
		this.updateTwipRectangle(this.sectionProperties.twipRectangle);
		this.sectionProperties.source.load();
	}

	// Sets the object rectangle in twips and converts it to the current pixel
	// position and size. Called on creation and whenever the zoom changes, since
	// the twip-to-pixel scale changes with zoom.
	public updateTwipRectangle(twipRectangle: number[]): void {
		this.sectionProperties.twipRectangle = twipRectangle;
		this.setPositionAndSizeFromTwipRectangles([twipRectangle]);
	}

	onDraw(): void {
		// The zoom animation scales the tiles, which hold the first frame, but
		// this section keeps the rectangle of the zoom start until the zoom
		// ends. So only the tiles show the image meanwhile.
		if (this.containerObject.isInZoomAnimation()) return;

		const frame = this.sectionProperties.source.getCurrentFrame();
		if (!frame) return;

		const [width, height] = this.size;

		// Drawing about the centre keeps the flipped frame on the same area.
		this.context.save();
		this.context.translate(width / 2, height / 2);
		this.context.scale(this.mirror & 1 ? -1 : 1, this.mirror & 2 ? -1 : 1);
		this.context.drawImage(frame, -width / 2, -height / 2, width, height);
		this.context.restore();
	}

	onRemove(): void {
		this.sectionProperties.source.dispose();
	}
}
