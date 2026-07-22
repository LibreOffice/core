/* -*- js-indent-level: 8 -*- */

/*
 * Copyright the Collabora Online contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/* See CanvasSectionContainer.ts for explanations. */

class AutoFillBaseSection extends CanvasSectionObject {
	processingOrder: number = 0;
	drawingOrder: number = 0;
	zIndex: number = 0;

	map: any;

	_showSection: boolean = true; // Store the internal show/hide section through forced readonly hides...

	constructor(name: string) {
		super(name);
		this.documentObject = true;
		this.map = window.L.Map.THIS;
		this.sectionProperties.docLayer = this.map._docLayer;
		this.sectionProperties.selectedAreaPoint = null;
		this.sectionProperties.cellCursorPoint = null;

		this.sectionProperties.dragStartPosition = null;

		var cursorStyle = getComputedStyle(
			this.sectionProperties.docLayer._cursorDataDiv,
		);
		var cursorColor = cursorStyle.getPropertyValue('border-top-color');
		this.backgroundColor = cursorColor ? cursorColor : this.backgroundColor;
	}

	public onInitialize() {
		if ((<any>window).mode.isDesktop()) {
			this.size = [Math.round(6 * app.dpiScale), Math.round(6 * app.dpiScale)];
		} else {
			this.size = [
				Math.round(16 * app.dpiScale),
				Math.round(16 * app.dpiScale),
			];
		}

		app.events.on(
			'updatepermission',
			this.showHideOnPermissionChange.bind(this),
		);

		app.events.on(
			'cellselectionfragmentchanged',
			this.setMarkerPosition.bind(this),
		);
	}

	private isSelectionFragmented(): boolean {
		return (
			!!app.activeDocument && app.activeDocument.activeView._selectionFragmented
		);
	}

	protected setMarkerPosition() {
		var center: number = 0;
		if (!(<any>window).mode.isDesktop()) {
			Util.ensureValue(app.calc.cellCursorRectangle);
			center = app.calc.cellCursorRectangle.pWidth * 0.5;
		}

		var position: Array<number> = [0, 0];
		this.setShowSection(true);

		if (this.sectionProperties.selectedAreaPoint !== null) {
			if (this.isSelectionFragmented())
				// Non-contiguous selection: autofill is disabled, hide.
				this.setShowSection(false);
			else
				position = [
					this.sectionProperties.selectedAreaPoint[0] - center,
					this.sectionProperties.selectedAreaPoint[1],
				];
		} else if (this.sectionProperties.cellCursorPoint !== null)
			position = [
				this.sectionProperties.cellCursorPoint[0] - center,
				this.sectionProperties.cellCursorPoint[1],
			];
		else this.setShowSection(false);

		this.setPosition(position[0], position[1]);
	}

	private calculatePositionFromPoint(point: Array<number> | null) {
		var calcPoint: Array<number> | null;
		if (point === null) {
			calcPoint = null;
		} else {
			var translation = [
				Math.floor(this.size[0] * 0.5),
				Math.floor(this.size[1] * 0.5),
			];
			calcPoint = [point[0] - translation[0], point[1] - translation[1]];
		}
		return calcPoint;
	}

	// Give bottom right position of selected area, in core pixels. Call with null parameter when auto fill marker is not visible.
	public calculatePositionViaCellSelection(point: Array<number>) {
		this.sectionProperties.selectedAreaPoint =
			this.calculatePositionFromPoint(point);
		this.setMarkerPosition();
	}

	// Give bottom right position of cell cursor, in core pixels. Call with null parameter when auto fill marker is not visible.
	public calculatePositionViaCellCursor(point: Array<number>) {
		this.sectionProperties.cellCursorPoint =
			this.calculatePositionFromPoint(point);
		this.setMarkerPosition();
	}

	// This is for enhancing contrast of the marker with the background
	// similar to what we have for cell cursors.
	private drawWhiteOuterBorders() {
		this.context.strokeStyle = 'white';
		this.context.lineCap = 'square';
		this.context.lineWidth = 1;

		const rectangle = this.boundingRectangle.clone();

		this.context.beginPath();
		this.context.moveTo(rectangle.v1X - 0.5, rectangle.v1Y - 0.5);
		this.context.lineTo(rectangle.v2X - 0.5, rectangle.v2Y - 0.5);
		this.context.moveTo(rectangle.v1X - 0.5, rectangle.v1Y - 0.5);
		this.context.lineTo(rectangle.v3X - 0.5, rectangle.v3Y - 0.5);
		this.context.closePath();
		this.context.stroke();
	}

	showHideOnPermissionChange() {
		this.setShowSection(null);
	}

	setShowSection(show: boolean | null) {
		if (show !== null) this._showSection = show;

		if (app.map._permission === 'readonly') {
			super.setShowSection(false);
		} else {
			super.setShowSection(this._showSection);
		}
	}

	public onDraw() {
		this.context.save();
		this.context.setTransform(1, 0, 0, 1, 0, 0);
		this.drawWhiteOuterBorders();
		this.context.restore();
	}

	private getCenterRegardingDocument(): cool.SimplePoint {
		const p2 = new cool.SimplePoint(0, 0);
		p2.pX += this.position[0] + this.size[0] * 0.5;
		p2.pY += this.position[1] + this.size[1] * 0.5;
		return p2;
	}

	// On mobile, setMarkerPosition shifts the marker left by half the cell
	// width so it sits visually under the cell. Core's autofill hit-test is at
	// the cell's bottom-right corner, so undo that shift before posting events.
	private adjustForMobileCenterOffset(p: cool.SimplePoint): void {
		if (!(<any>window).mode.isDesktop()) {
			Util.ensureValue(app.calc.cellCursorRectangle);
			p.pX += app.calc.cellCursorRectangle.pWidth * 0.5;
		}
	}

	private getMouseControl(): MouseControl {
		Util.ensureValue(app.activeDocument);
		Util.ensureValue(app.activeDocument.mouseControl);
		return app.activeDocument.mouseControl;
	}

	// Whether the fill area this marker drags is currently shown. Subclasses that
	// drive a different fill area override this.
	protected isFillAreaVisible(): boolean {
		return !!this.sectionProperties.docLayer._cellAutoFillAreaPixels;
	}

	// MouseControl decides between panning the view and posting to the engine from
	// e.type, and it pans on a touch drag. An autofill drag must post (fill), so
	// present the event to MouseControl as the matching mouse type. Only the type
	// changes; the modifier keys stay intact for readModifier.
	private asMouseEvent(e: MouseEvent): MouseEvent {
		const touchToMouse: { [key: string]: string } = {
			touchstart: 'mousedown',
			touchmove: 'mousemove',
			touchend: 'mouseup',
		};
		const mouseType = touchToMouse[e.type];
		if (mouseType)
			Object.defineProperty(e, 'type', {
				value: mouseType,
				configurable: true,
			});
		return e;
	}

	// Convert a marker-local point into MouseControl's coordinate frame.
	// Marker-local plus myTopLeft is the canvas-pixel coordinate. MouseControl is
	// bound to the tiles section and expects its point in that section's local
	// frame, which is offset from canvas by the document anchor (for example the
	// ruler height in Writer), so remove the anchor as well.
	protected toMouseControlPoint(point: cool.SimplePoint): cool.SimplePoint {
		const docAnchor = app.sectionContainer.getDocumentAnchor();
		let canvasX = point.pX + this.myTopLeft[0];
		const canvasY = point.pY + this.myTopLeft[1];

		// On mobile the marker is drawn half a cell to the left of the handle, so
		// shift the point back to the cell corner the engine hit-tests for.
		if (!(<any>window).mode.isDesktop()) {
			Util.ensureValue(app.calc.cellCursorRectangle);
			canvasX += app.calc.cellCursorRectangle.pWidth * 0.5;
		}

		return cool.SimplePoint.fromCorePixels([
			canvasX - docAnchor[0],
			canvasY - docAnchor[1],
		]);
	}

	// Forward the drag to MouseControl, which posts the mouse events to the engine
	// and runs the shared edge autoscroll and the re-send of the drag position
	// while the view scrolls, so the fill keeps following the pointer.
	public onMouseMove(
		point: cool.SimplePoint,
		dragDistance: Array<number>,
		e: MouseEvent,
	) {
		if (dragDistance === null || !this.isFillAreaVisible()) return; // No dragging or no event handling or the fill marker is not visible.

		this.getMouseControl().onMouseMove(
			this.toMouseControlPoint(point),
			dragDistance,
			this.asMouseEvent(e),
		);
	}

	public onMouseUp(point: cool.SimplePoint, e: MouseEvent) {
		this.getMouseControl().onMouseUp(
			this.toMouseControlPoint(point),
			this.asMouseEvent(e),
		);
	}

	public onMouseDown(point: cool.SimplePoint, e: MouseEvent) {
		// Anchor the drag at the marker centre, the cell's bottom-right corner, so
		// the engine starts an autofill rather than a cell selection.
		const center = cool.SimplePoint.fromCorePixels([
			this.size[0] * 0.5,
			this.size[1] * 0.5,
		]);
		this.getMouseControl().onMouseDown(
			this.toMouseControlPoint(center),
			this.asMouseEvent(e),
		);
	}

	public onMouseEnter() {
		this.context.canvas.style.cursor = 'crosshair';
	}

	public onNewDocumentTopLeft() {
		this.setMarkerPosition();
	}

	public onDoubleClick(point: cool.SimplePoint, e: MouseEvent) {
		const pos = this.getCenterRegardingDocument();
		this.adjustForMobileCenterOffset(pos);
		this.sectionProperties.docLayer._postMouseEvent(
			'buttondown',
			pos.x,
			pos.y,
			2,
			1,
			0,
		);
		this.sectionProperties.docLayer._postMouseEvent(
			'buttonup',
			pos.x,
			pos.y,
			2,
			1,
			0,
		);
	}
}
