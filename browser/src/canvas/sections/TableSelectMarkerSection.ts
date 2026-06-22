/*
 * Copyright the Collabora Online contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

class TableSelectMarkerSection extends HTMLObjectSection {
	constructor(
		name: string,
		objectWidth: number,
		objectHeight: number,
		documentPosition: cool.SimplePoint,
		extraClass: string,
		markerType: string,
		showSection: boolean = true,
	) {
		super(
			name,
			objectWidth,
			objectHeight,
			documentPosition,
			extraClass,
			showSection,
		);

		this.sectionProperties.markerType = markerType;
		this.sectionProperties.contextMenuActivated = false;
	}

	public onMouseEnter(point: cool.SimplePoint, e: MouseEvent): void {
		this.getHTMLObject()?.classList.add('hovered');
		this.context.canvas.style.cursor = 'pointer';
	}

	public onMouseLeave(point: cool.SimplePoint, e: MouseEvent): void {
		this.getHTMLObject()?.classList.remove('hovered');
	}

	public onMouseDown(point: cool.SimplePoint, e: MouseEvent): void {
		this.stopPropagating(e);
	}

	public onMouseUp(point: cool.SimplePoint, e: MouseEvent): void {
		this.stopPropagating(e);

		if (this.sectionProperties.contextMenuActivated === true) {
			this.sectionProperties.contextMenuActivated = false;
			return;
		}

		// Hammer.js doesn't fire onClick event after touchEnd event.
		// CanvasSectionContainer fires the onClick event. But since Hammer.js is used for map, it disables the onClick for SectionContainer.
		// We will use this event as click event on touch devices, until we remove Hammer.js (then this code will be removed from here).
		if (
			(!this.containerObject.isDraggingSomething() &&
				(<any>window).mode.isSmallScreenDevice()) ||
			(<any>window).mode.isTablet()
		) {
			this.onClick(point, e);
		}
	}

	onContextMenu(point: cool.SimplePoint, e: MouseEvent): void {
		Util.ensureValue(app.activeDocument);
		let x: number;
		let y: number;
		if (this.sectionProperties.markerType === 'column') {
			x = (this.position[0] + this.size[0] * 0.5) * app.pixelsToTwips;
			y =
				app.activeDocument.tableMiddleware.getTableTopY() * app.pixelsToTwips +
				5;
		} else {
			x =
				app.activeDocument.tableMiddleware.getTableLeftX() * app.pixelsToTwips +
				5;
			y = (this.position[1] + this.size[1] * 0.5) * app.pixelsToTwips;
		}
		app.map._docLayer._postMouseEvent('buttondown', x, y, 1, 4, 0);
		this.sectionProperties.contextMenuActivated = true;
	}

	public onClick(point: cool.SimplePoint, e: MouseEvent): void {
		this.stopPropagating(e);
		Util.ensureValue(app.activeDocument);
		if (this.sectionProperties.markerType === 'column') {
			const x1 = Math.round(
				(this.position[0] + this.size[0] * 0.5) * app.pixelsToTwips,
			);
			const y1 = Math.round(
				app.activeDocument.tableMiddleware.getTableTopY() * app.pixelsToTwips,
			);
			const x2 = x1;
			const y2 = Math.round(
				app.activeDocument.tableMiddleware.getTableBottomY() *
					app.pixelsToTwips,
			);

			app.map._docLayer._postSelectTextEvent('start', x1, y1 + 5);
			app.map._docLayer._postSelectTextEvent('end', x2, y2 - 5);
		} else {
			const x1 = Math.round(
				app.activeDocument.tableMiddleware.getTableLeftX() * app.pixelsToTwips,
			);
			const y1 = Math.round(
				(this.position[1] + this.size[1] * 0.5) * app.pixelsToTwips,
			);
			const x2 = Math.round(
				app.activeDocument.tableMiddleware.getTableRightX() * app.pixelsToTwips,
			);
			const y2 = y1;

			app.map._docLayer._postSelectTextEvent('start', x1 + 5, y1);
			app.map._docLayer._postSelectTextEvent('end', x2 - 5, y2);
		}
	}
}
