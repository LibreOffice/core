/*
 * Copyright the Collabora Office contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

namespace cool {
	/// Draws polygon outlines around redline text when a redline tooltip is shown on hover.
	export class TooltipAnchorSection extends CanvasSectionObject {
		processingOrder: number = app.CSections.TooltipAnchorLeft.processingOrder;
		drawingOrder: number = app.CSections.TooltipAnchorLeft.drawingOrder;
		zIndex: number = app.CSections.TooltipAnchorLeft.zIndex;
		interactable: boolean = false;
		documentObject: boolean = true;

		constructor(name: string, mode: number) {
			super(name);

			this.sectionProperties.polygon = null;
			this.sectionProperties.polygonColor = 'black';
			this.sectionProperties.mode = mode;
		}

		public override onResize(): void {
			if (!this.sectionProperties.polygon) return;

			this.setTwipRectangles(this.sectionProperties.lastRectangles);
		}

		public override onNewDocumentTopLeft(): void {
			if (!this.sectionProperties.polygon) return;

			this.setTwipRectangles(this.sectionProperties.lastRectangles);
		}

		public override onDraw(): void {
			if (!this.sectionProperties.polygon) return;

			this.drawPolygon();
		}

		/// Draws anchor rectangles, used by UIManager.
		public drawAnchorRectangles(
			anchorRectangles: string[],
			redlineType: string,
		): void {
			const rectangles = this.getRectangles(anchorRectangles);
			this.sectionProperties.lastRectangles = rectangles;
			if (rectangles.length === 0) {
				this.sectionProperties.polygon = null;
				app.sectionContainer.requestReDraw();
				return;
			}

			this.sectionProperties.polygon = cool.rectanglesToPolygon(
				rectangles,
				app.twipsToPixels,
			);

			// Set the polygon color to roughly match the auto text color.
			const mode = this.sectionProperties.mode;
			if (redlineType === 'Delete' && mode === TileMode.LeftSide)
				this.sectionProperties.polygonColor = 'red';
			else if (redlineType === 'Insert' && mode === TileMode.RightSide)
				this.sectionProperties.polygonColor = 'green';
			else this.sectionProperties.polygonColor = 'gray';

			this.setTwipRectangles(rectangles);
			app.sectionContainer.requestReDraw();
		}

		/// Hides anchor rectangles, used by UIManager.
		public hideAnchorRectangles(): void {
			this.sectionProperties.polygon = null;
			app.sectionContainer.requestReDraw();
		}

		/// Sets position and size from twip rectangles, then applies the TileMode.
		private setTwipRectangles(rectangles: Array<number[]>): void {
			this.setPositionAndSizeFromTwipRectangles(rectangles);
			this.documentPosition.mode = this.sectionProperties.mode;
			this.myTopLeft[0] = this.documentPosition.vX;
			this.myTopLeft[1] = this.documentPosition.vY;
			if (this.containerObject.testing)
				this.containerObject.createUpdateSingleDivElement(this);
		}

		/// Parses the rectangles string[] into an Array<number[]>.
		private getRectangles(rects: string[]): Array<number[]> {
			const rectangles: Array<number[]> = [];
			for (const rect of rects) {
				const parts = rect.split(', ');
				if (parts.length >= 4) {
					rectangles.push([
						parseInt(parts[0]),
						parseInt(parts[1]),
						parseInt(parts[2]),
						parseInt(parts[3]),
					]);
				}
			}
			return rectangles;
		}
	}
}

app.definitions.TooltipAnchorSection = cool.TooltipAnchorSection;
