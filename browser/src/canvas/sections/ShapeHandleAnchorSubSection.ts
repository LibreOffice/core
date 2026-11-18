// @ts-strict-ignore
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
	This class is for the sub sections (handles) of ShapeHandlesSection.
	Shape is rendered on the core side. Only the handles are drawn here and modification commands are sent to the core side.
*/

class ShapeHandleAnchorSubSection extends HTMLObjectSection {
	static tableAnchorIconSize = [20, 20]; // CSS pixels.

	constructor (parentHandlerSection: ShapeHandlesSection | null, sectionName: string, size: number[], documentPosition: cool.SimplePoint, ownInfo: any) {
        super(sectionName, size[0], size[1], documentPosition, 'anchor-marker');

        this.sectionProperties.parentHandlerSection = parentHandlerSection;
		this.sectionProperties.ownInfo = ownInfo;
	}

	onMouseEnter() {
		this.backgroundColor = 'grey';
		this.context.canvas.style.cursor = 'url(' + app.LOUtil.getURL('images/cursors/grab.svg') + ') 12 12, grab';
		this.containerObject.requestReDraw();
	}

	onMouseLeave() {
		this.backgroundColor = null;
		this.containerObject.requestReDraw();
	}

	tableMouseUp(point: cool.SimplePoint, e: MouseEvent) {
		const parameters = {
			'TransformPosX': {
				'type': 'long',
				'value': Math.round((point.pX + this.position[0]) * app.pixelsToTwips)
			},
			'TransformPosY': {
				'type': 'long',
				'value': Math.round((point.pY + this.position[1]) * app.pixelsToTwips)
			}
		};

		app.map.sendUnoCommand('.uno:TransformDialog', parameters);
	}

	shapeMouseUp(point: cool.SimplePoint, e: MouseEvent) {
		const parameters = {
			...ShapeHandlesSection.handleParameters(this.sectionProperties.ownInfo),
			'NewPosX': {
				'type': 'long',
				'value': Math.round((point.pX + this.position[0]) * app.pixelsToTwips)
			},
			'NewPosY': {
				'type': 'long',
				'value': Math.round((point.pY + this.position[1]) * app.pixelsToTwips)
			}
		};

		app.map.sendUnoCommand('.uno:MoveShapeHandle', parameters);
	}

	onMouseUp(point: cool.SimplePoint, e: MouseEvent): void {
		if (this.containerObject.isDraggingSomething()) {
			// Tables don't have parent sections. This is used for separating table anchors from other anchors.
			if (this.sectionProperties.parentHandlerSection) {
				this.shapeMouseUp(point, e);
			}
			else {
				this.tableMouseUp(point, e);
			}

			this.sectionProperties.parentHandlerSection.hideSVG();
		}
	}

	onMouseMove(point: cool.SimplePoint, dragDistance: Array<number>, e: MouseEvent) {
		if (this.containerObject.isDraggingSomething()) {
			// Show preview in its final position.
			let svg;
			let initialPosition;
			if (this.sectionProperties.parentHandlerSection) {
				this.sectionProperties.parentHandlerSection.showSVG();
				svg = this.sectionProperties.parentHandlerSection.sectionProperties.svg;
				initialPosition = this.sectionProperties.parentHandlerSection.sectionProperties.svgPosition;
			}
			else {
				// Table..
				svg = document.getElementById('canvas-container').querySelector('svg');
				svg.style.display = '';
				if (!this.sectionProperties.initialPosition) {
					this.sectionProperties.initialPosition = [parseFloat(svg.style.left.replace('px', '')) * app.dpiScale, parseFloat(svg.style.top.replace('px', '')) * app.dpiScale];
				}
				initialPosition = this.sectionProperties.initialPosition;
			}
			svg.style.left = (dragDistance[0] + initialPosition[0]) / app.dpiScale + 'px';
			svg.style.top = (dragDistance[1] + initialPosition[1]) / app.dpiScale + 'px';
		}
	}
}
