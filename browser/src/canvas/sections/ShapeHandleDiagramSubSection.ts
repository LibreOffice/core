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

class ShapeHandleDiagramSubSection extends CanvasSectionObject {
	processingOrder: number =
		app.CSections.DefaultForDocumentObjects.processingOrder;
	drawingOrder: number =
		app.CSections.DefaultForDocumentObjects.drawingOrder + 1; // Handle events before the parent section.
	zIndex: number = app.CSections.DefaultForDocumentObjects.zIndex;
	documentObject: boolean = true;
	filledCombine: Path2D = new Path2D();
	filledCombineUsed: boolean = false;
	fillColor: string = '';
	fillColorDark: string = '';
	fillColorLight: string = '';

	constructor(
		parentHandlerSection: ShapeHandlesSection,
		sectionName: string,
		size: number[],
		documentPosition: cool.SimplePoint,
		ownInfo: any,
	) {
		super(sectionName);

		this.size = size;

		this.sectionProperties.position = documentPosition.clone();
		this.sectionProperties.parentHandlerSection = parentHandlerSection;
		this.sectionProperties.ownInfo = ownInfo;
		this.sectionProperties.cursorStyle = 'pointer';

		app.events.on(
			'TextCursorVisibility',
			this.onTextCursorVisibility.bind(this),
		);
	}

	onTextCursorVisibility(event: any): void {
		if (event.detail.visible) {
			this.setShowSection(false);
		} else {
			this.setShowSection(true);
		}
	}

	onInitialize(): void {
		this.setPosition(
			this.sectionProperties.position.pX,
			this.sectionProperties.position.pY,
		);
	}

	onMouseEnter(point: cool.SimplePoint, e: MouseEvent): void {
		this.context.canvas.style.cursor = this.sectionProperties.cursorStyle;
	}

	private createRoundRect(
		path: Path2D,
		x: number,
		y: number,
		w: number,
		h: number,
		r: number,
	): void {
		path.moveTo(x, y + r);
		path.arcTo(x, y, x + r, y, r);
		path.lineTo(x + w - r, y);
		path.arcTo(x + w, y, x + w, y + r, r);
		path.lineTo(x + w, y + h - r);
		path.arcTo(x + w, y + h, x + w - r, y + h, r);
		path.lineTo(x + r, y + h);
		path.arcTo(x, y + h, x, y + h - r, r);
		path.lineTo(x, y + r);
	}

	onDraw(frameCount?: number, elapsedTime?: number): void {
		const origLineWidth: number = this.context.lineWidth;
		const halfWidthPx: number =
			this.sectionProperties.ownInfo.halfWidth * app.twipsToPixels;
		const halfWidthPy: number =
			this.sectionProperties.ownInfo.halfHeight * app.twipsToPixels;

		// create outerBound
		var outerBound = new Path2D();
		this.createRoundRect(
			outerBound,
			0,
			0,
			this.size[0],
			this.size[1],
			halfWidthPx,
		);

		// create innerBound
		var innerBound = new Path2D();
		const innerRelative: number = 0.5;
		this.createRoundRect(
			innerBound,
			halfWidthPx * innerRelative,
			halfWidthPy * innerRelative,
			this.size[0] - halfWidthPx * (2.0 * innerRelative),
			this.size[1] - halfWidthPy * (2.0 * innerRelative),
			halfWidthPx * (1.0 - innerRelative),
		);

		// reset filledCombine & create combination for fill
		this.filledCombine = new Path2D();
		this.filledCombineUsed = true;
		this.filledCombine.addPath(outerBound);
		this.filledCombine.addPath(innerBound);

		if (!this.fillColor) {
			const baseElem = document.getElementsByTagName('body')[0];
			const elem = window.L.DomUtil.create(
				'div',
				'spreadsheet-header-row',
				baseElem,
			);

			// create colors
			this.fillColor = window.L.DomUtil.getStyle(elem, 'background-color');
			const blendFactor: number = 0.1;

			let mix: any = colorParser(this.fillColor);
			mix.add(new RGBColor(blendFactor, blendFactor, blendFactor));
			this.fillColorLight = mix.toString();

			mix = colorParser(this.fillColor);
			mix.add(new RGBColor(-blendFactor, -blendFactor, -blendFactor));
			this.fillColorDark = mix.toString();

			window.L.DomUtil.remove(elem);
		}

		// draw fill
		const selectionBackgroundGradient = this.context.createLinearGradient(
			0,
			0,
			this.size[0],
			this.size[1],
		);
		selectionBackgroundGradient.addColorStop(0, this.fillColorDark);
		selectionBackgroundGradient.addColorStop(0.5, this.fillColor);
		selectionBackgroundGradient.addColorStop(1, this.fillColorLight);
		this.context.fillStyle = selectionBackgroundGradient;
		this.filledCombine.closePath();
		this.context.fill(this.filledCombine, 'evenodd');

		// draw outer
		this.context.strokeStyle = this.fillColorDark;
		this.context.lineWidth = 2;
		this.context.stroke(outerBound);

		// draw inner
		this.context.stroke(innerBound);

		this.context.lineWidth = origLineWidth;
	}

	onClick(point: cool.SimplePoint, e: MouseEvent): void {
		app.map.sendUnoCommand('.uno:EditDiagram');
	}

	isHit(point: number[]): boolean {
		if (!super.isHit(point)) {
			// use baseclass method to see if we are outside local range (position, size)
			return false;
		}

		if (this.filledCombineUsed) {
			// if we have the geometry from paint, use it for HitTest
			return this.context.isPointInPath(
				this.filledCombine,
				point[0] - this.myTopLeft[0],
				point[1] - this.myTopLeft[1],
				'evenodd',
			);
		}

		// if not, check for outer bounds defined by offsets from outside
		const halfWidthPx: number =
			this.sectionProperties.ownInfo.halfWidth * app.twipsToPixels;
		const halfWidthPy: number =
			this.sectionProperties.ownInfo.halfHeight * app.twipsToPixels;

		if (
			point[0] <= this.myTopLeft[0] + halfWidthPx ||
			point[0] >= this.myTopLeft[0] + this.size[0] - halfWidthPx ||
			point[1] <= this.myTopLeft[1] + halfWidthPy ||
			point[1] >= this.myTopLeft[1] + this.size[1] - halfWidthPy
		) {
			return true;
		}

		return false;
	}
}

app.definitions.shapeHandleDiagramSubSection = ShapeHandleDiagramSubSection;
