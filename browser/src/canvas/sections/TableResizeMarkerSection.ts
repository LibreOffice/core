/*
 * Copyright the Collabora Office contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

class TableResizeMarkerSection extends HTMLObjectSection {
	constructor(
		name: string,
		objectWidth: number,
		objectHeight: number,
		documentPosition: cool.SimplePoint,
		extraClass: string,
		markerType: string,
		index: number,
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
		this.sectionProperties.index = index;
		this.sectionProperties.leftMost = 0;
		this.sectionProperties.rightMost = 0;
		this.sectionProperties.topMost = 0;
		this.sectionProperties.bottomMost = 0;
		this.sectionProperties.dragStartPosition = null;
		this.sectionProperties.initialPosition = this.position.slice();
		this.sectionProperties.hoverCursor =
			markerType === 'column' ? 'col-resize' : 'row-resize';
	}

	private calculateLeftMostAndRightMostAvailableX() {
		Util.ensureValue(app.activeDocument);
		const previous =
			this.sectionProperties.index === 0
				? null
				: app.activeDocument.tableMiddleware.tableResizeColumnMarkers[
						this.sectionProperties.index - 1
					];
		const next =
			this.sectionProperties.index ===
			app.activeDocument.tableMiddleware.tableResizeColumnMarkers.length - 1
				? null
				: app.activeDocument.tableMiddleware.tableResizeColumnMarkers[
						this.sectionProperties.index + 1
					];

		if (!previous) {
			// First column marker.
			this.sectionProperties.leftMost = this.position[0] - 1000;
		} else {
			this.sectionProperties.leftMost =
				previous.position[0] +
				app.activeDocument.tableMiddleware.resizeMarkerMaxApproximation;
		}

		if (!next) {
			// Last column marker.
			this.sectionProperties.rightMost = this.position[0] + 1000;
		} else {
			this.sectionProperties.rightMost =
				next.position[0] -
				app.activeDocument.tableMiddleware.resizeMarkerMaxApproximation;
		}
	}

	private calculateTopMostAndBottomMostAvailableY() {
		Util.ensureValue(app.activeDocument);
		const previous =
			this.sectionProperties.index === 0
				? null
				: app.activeDocument.tableMiddleware.tableResizeRowMarkers[
						this.sectionProperties.index - 1
					];
		const next =
			this.sectionProperties.index ===
			app.activeDocument.tableMiddleware.tableResizeRowMarkers.length - 1
				? null
				: app.activeDocument.tableMiddleware.tableResizeRowMarkers[
						this.sectionProperties.index + 1
					];

		if (!previous) {
			// First row marker.
			this.sectionProperties.topMost =
				app.activeDocument.tableMiddleware.getTableTopY() +
				app.activeDocument.tableMiddleware.resizeMarkerMaxApproximation;
		} else {
			this.sectionProperties.topMost =
				previous.position[1] +
				app.activeDocument.tableMiddleware.resizeMarkerMaxApproximation;
		}

		if (!next) {
			// Last row marker.
			this.sectionProperties.bottomMost = this.position[1] + 1000;
		} else {
			this.sectionProperties.bottomMost =
				next.position[1] -
				app.activeDocument.tableMiddleware.resizeMarkerMaxApproximation;
		}
	}

	public onMouseEnter(point: cool.SimplePoint, e: MouseEvent): void {
		// Calculate on mouse enter so we don't need to recaulculate on every mouse move.
		if (this.sectionProperties.markerType === 'column')
			this.calculateLeftMostAndRightMostAvailableX();
		else this.calculateTopMostAndBottomMostAvailableY();

		this.getHTMLObject().classList.add('hovered');

		this.context.canvas.style.cursor = this.sectionProperties.hoverCursor;
	}

	public onMouseLeave(point: cool.SimplePoint, e: MouseEvent): void {
		this.sectionProperties.dragStartPosition = null;
		this.getHTMLObject().classList.remove('hovered');
	}

	public onMouseDown(point: cool.SimplePoint, e: MouseEvent): void {
		this.sectionProperties.dragStartPosition = point;
		if (
			(<any>window).mode.isSmallScreenDevice() ||
			(<any>window).mode.isTablet()
		) {
			this.calculateLeftMostAndRightMostAvailableX();
			this.calculateTopMostAndBottomMostAvailableY();
		}
	}

	private getParametersForRow() {
		let index: number, type: string;
		Util.ensureValue(app.activeDocument);
		if (
			this.sectionProperties.index ===
			app.activeDocument.tableMiddleware.tableResizeRowMarkers.length - 1
		) {
			type = 'row-right';
			index = 0;
		} else {
			type = 'row-middle';
			index = this.sectionProperties.index;
		}

		return { index: index, type: type };
	}

	private getParametersForColumn() {
		let index: number, type: string;
		Util.ensureValue(app.activeDocument);
		if (this.sectionProperties.index === 0) {
			type = 'column-left';
			index = 0;
		} else if (
			this.sectionProperties.index ===
			app.activeDocument.tableMiddleware.tableResizeColumnMarkers.length - 1
		) {
			type = 'column-right';
			index = 0;
		} else {
			type = 'column-middle';
			index = this.sectionProperties.index - 1;
		}

		return { index: index, type: type };
	}

	private onDragEnd(markerType: string) {
		let offset;

		if (markerType === 'column') {
			offset =
				Math.round(
					this.position[0] - this.sectionProperties.initialPosition[0],
				) * app.pixelsToTwips;
		} else {
			offset =
				Math.round(
					this.position[1] - this.sectionProperties.initialPosition[1],
				) * app.pixelsToTwips;
		}

		let parameters;
		if (markerType === 'column') parameters = this.getParametersForColumn();
		else parameters = this.getParametersForRow();

		const commandArguments = {
			BorderType: {
				type: 'string',
				value: parameters.type,
			},
			Index: {
				type: 'uint16',
				value: parameters.index,
			},
			Offset: {
				type: 'int32',
				value: offset,
			},
		};

		app.map.sendUnoCommand(
			'.uno:TableChangeCurrentBorderPosition',
			commandArguments,
		);
	}

	public onMouseUp(point: cool.SimplePoint, e: MouseEvent): void {
		this.sectionProperties.dragStartPosition = null;

		if (
			this.containerObject.isDraggingSomething() &&
			this.containerObject.targetSection === this.name
		) {
			this.onDragEnd(this.sectionProperties.markerType);
		}
	}

	private columnDrag(point: cool.SimplePoint) {
		const dragDistance = [];
		dragDistance.push(point.pX - this.sectionProperties.dragStartPosition.pX);
		dragDistance.push(point.pY - this.sectionProperties.dragStartPosition.pY);

		const finalPosition = [
			dragDistance[0] + this.position[0],
			this.position[1],
		];
		if (
			finalPosition[0] > this.sectionProperties.leftMost &&
			finalPosition[0] < this.sectionProperties.rightMost
		) {
			this.setPosition(finalPosition[0], finalPosition[1]);
			this.containerObject.requestReDraw();
		}
	}

	private rowDrag(point: cool.SimplePoint) {
		const dragDistance = [];
		dragDistance.push(point.pX - this.sectionProperties.dragStartPosition.pX);
		dragDistance.push(point.pY - this.sectionProperties.dragStartPosition.pY);

		const finalPosition = [
			this.position[0],
			this.position[1] + dragDistance[1],
		];
		if (
			finalPosition[1] > this.sectionProperties.topMost &&
			finalPosition[1] < this.sectionProperties.bottomMost
		) {
			this.setPosition(finalPosition[0], finalPosition[1]);
			this.containerObject.requestReDraw();
		}
	}

	public onMouseMove(
		point: cool.SimplePoint,
		dragDistance: Array<number>,
		e: MouseEvent,
	): void {
		if (this.containerObject.isDraggingSomething()) {
			// We only allow horizontal movement for column markers and vertical for row markers.

			if (this.sectionProperties.markerType === 'column')
				this.columnDrag(point);
			else this.rowDrag(point);

			this.adjustHTMLObjectPosition();
		}
	}

	onDraw(frameCount?: number, elapsedTime?: number): void {
		if (
			this.containerObject.isDraggingSomething() &&
			this.containerObject.targetSection === this.name
		) {
			Util.ensureValue(app.activeDocument);
			if (this.sectionProperties.markerType === 'column') {
				const bottomy =
					app.activeDocument.tableMiddleware.getTableBottomY() -
					this.position[1];
				this.context.strokeStyle = '#3388FF';
				this.context.lineWidth = 2;
				this.context.beginPath();
				this.context.moveTo(this.size[0] / 2, this.size[1]);
				this.context.lineTo(this.size[0] / 2, bottomy);
				this.context.stroke();
				this.context.closePath();
			} else {
				const rightX =
					app.activeDocument.tableMiddleware.getTableRightX() -
					this.position[0];
				this.context.strokeStyle = '#3388FF';
				this.context.lineWidth = 2;
				this.context.beginPath();
				this.context.moveTo(this.size[0], this.size[1] / 2);
				this.context.lineTo(this.size[0] + rightX, this.size[1] / 2);
				this.context.stroke();
				this.context.closePath();
			}
		}
	}
}
