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

class SplitterLinesSection extends CanvasSectionObject {
	interactable: boolean = false;
	processingOrder: number = app.CSections.Splitter.processingOrder;
	drawingOrder: number = app.CSections.Splitter.drawingOrder;
	zIndex: number = app.CSections.Splitter.zIndex;
	documentObject: boolean = true;

	constructor() {
		super(app.CSections.Splitter.name);
	}

	onInitialize(): void {
		const splittersDataDiv = document.createElement('div');
		splittersDataDiv.className = 'splitters-data';

		const canvasContainer = document.getElementById('canvas-container');
		if (canvasContainer) canvasContainer.appendChild(splittersDataDiv);

		this.size = [1000000, 1000000];
		this.position = [0, 0];

		this.sectionProperties = {
			fillColor: 'rgba(0, 0, 0, 0)',
			fillOpacity: 0,
			thickness: 0,
		};

		this.readCSSProperties();
	}

	readCSSProperties() {
		const splittersDataDiv = document.querySelector('.splitters-data');

		if (splittersDataDiv) {
			this.sectionProperties.fillColor =
				getComputedStyle(splittersDataDiv).getPropertyValue('color');

			this.sectionProperties.fillOpacity =
				getComputedStyle(splittersDataDiv).getPropertyValue('opacity');

			this.sectionProperties.thickness =
				parseFloat(
					getComputedStyle(splittersDataDiv)
						.getPropertyValue('border-top-width')
						.replace('px', ''),
				) * app.dpiScale;
		}
	}

	private setBoxGradient(splitPos: any, isVertical: boolean) {
		let selectionBackgroundGradient = null;

		Util.ensureValue(app.map._docLayer.sheetGeometry);
		// last row geometry data will be a good for setting default raw height
		const spanlist = app.map._docLayer.sheetGeometry
			.getRowsGeometry()
			.getVisibleSizes()._spanlist;
		const rowData = spanlist[spanlist.length - 1];

		// Create a linear gradient based on the extracted color stops
		// get raw data from sheet geometry. use index = 1
		const deafultRowSize = rowData.data.sizecore;

		// Gradient width should be half of the default row height.
		const gradientWidth: number = Math.ceil(deafultRowSize / 2);

		// Adjust horizontal position for RTL mode.
		splitPos.x = this.isCalcRTL() ? this.size[0] - splitPos.x : splitPos.x;

		// Create a linear gradient based on the extracted color stops
		selectionBackgroundGradient = this.createSplitLineGradient(
			splitPos,
			gradientWidth,
			isVertical,
		);

		return { selectionBackgroundGradient, gradientWidth };
	}

	private createSplitLineGradient(
		splitPos: any,
		gradientWidth: number,
		isVertSplitter: boolean,
	) {
		const sPx = splitPos.x * app.dpiScale;
		const sPy = splitPos.y * app.dpiScale;

		let linearGradient = null;
		const colorStops = [
			{ colorCode: this.sectionProperties.fillColor, offset: 0 },
			{ colorCode: 'rgba(240, 240, 240, 0)', offset: 1 },
		];

		if (isVertSplitter) {
			linearGradient = this.context.createLinearGradient(
				0,
				sPy,
				0,
				sPy + gradientWidth,
			);
		} else {
			let x0 = sPx;
			let x1 = sPx + gradientWidth;
			if (this.isCalcRTL()) {
				x0 = sPx - gradientWidth;
				x1 = sPx;
			}
			linearGradient = this.context.createLinearGradient(x0, 0, x1, 0);
		}

		// Add color stops to the gradient
		for (let i = 0; i < colorStops.length; i++) {
			// set offset with colorcode & handle special case for horizontal line in RTL mode
			const offset =
				!isVertSplitter && this.isCalcRTL()
					? colorStops[colorStops.length - i - 1].offset
					: colorStops[i].offset;
			linearGradient.addColorStop(offset, colorStops[i].colorCode);
		}

		return linearGradient;
	}

	private GetColumnHeaderHeight(): number {
		if (
			this.containerObject.getSectionWithName(app.CSections.ColumnHeader.name)
		) {
			return this.containerObject.getSectionWithName(
				app.CSections.ColumnHeader.name,
			).size[1];
		} else return 0;
	}

	private GetRowHeaderWidth(): number {
		if (this.containerObject.getSectionWithName(app.CSections.RowHeader.name)) {
			return this.containerObject.getSectionWithName(
				app.CSections.RowHeader.name,
			).size[0];
		} else return 0;
	}

	onDraw(): void {
		const splitPanesContext = app.map._docLayer.getSplitPanesContext();

		if (splitPanesContext) {
			var splitPos = splitPanesContext.getSplitPos();

			if (splitPos.x || splitPos.y) {
				this.context.globalAlpha = this.sectionProperties.fillOpacity;
				this.context.lineWidth = this.sectionProperties.thickness;

				if (splitPos.x) {
					let width, style;
					Util.ensureValue(app.activeDocument);
					if (app.activeDocument.activeLayout.viewedRectangle.pX1 === 0) {
						width = this.sectionProperties.thickness;
						style = this.sectionProperties.fillColor;
					} else {
						const temp = this.setBoxGradient(splitPos.clone(), false);
						width = temp.gradientWidth;
						style = temp.selectionBackgroundGradient;
					}
					this.context.fillStyle = style;

					this.context.fillRect(
						splitPos.x * app.dpiScale,
						-this.GetColumnHeaderHeight(),
						width,
						this.size[1],
					);
				}

				if (splitPos.y) {
					let width, style;
					Util.ensureValue(app.activeDocument);
					if (app.activeDocument.activeLayout.viewedRectangle.pY1 === 0) {
						width = this.sectionProperties.thickness;
						style = this.sectionProperties.fillColor;
					} else {
						const temp = this.setBoxGradient(splitPos.clone(), true);
						width = temp.gradientWidth;
						style = temp.selectionBackgroundGradient;
					}

					this.context.fillStyle = style;

					this.context.fillRect(
						-this.GetRowHeaderWidth(),
						splitPos.y * app.dpiScale,
						this.size[0],
						width,
					);
				}
			}
		}

		this.context.globalAlpha = 1;
	}
}
