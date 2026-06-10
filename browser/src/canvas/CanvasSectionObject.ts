// @ts-strict-ignore
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

// This class will be used internally by CanvasSectionContainer.
class CanvasSectionObject {
	context: CanvasRenderingContext2D;
	myTopLeft: Array<number> = [0, 0];
	containerObject: CanvasSectionContainer = null;
	readonly name: string = null;
	backgroundColor: string = null; // Default is null (container's background color will be used).
	backgroundOpacity: number = 1; // Valid when backgroundColor is valid.
	borderColor: string = null; // Default is null (no borders).
	boundToSection: string = null;
	anchor: Array<string> | Array<Array<string>> = [];
	documentObject: boolean = false; // If true, the section is a document object.
	documentPosition: cool.SimplePoint = new cool.SimplePoint(0, 0); // Used with document objects.
	boundingRectangle: cool.SimpleRectangle = new cool.SimpleRectangle(0, 0, 0, 0);
	// When section is a document object, its position should be the real position inside the document, in core pixels.
	isVisible: boolean = false; // Is section visible on the viewed area of the document? This property is valid for document objects. This is managed by the section container.
	showSection: boolean = true; // Show / hide section.
	position: Array<number> = [0, 0];
	isCollapsed: boolean;
	size: Array<number> = [0, 0];
	origSizeHint: undefined | Array<number>; // This is used to preserve the original size provided on construct.
	expand: Array<string> = [];
	isLocated: boolean; // Location and size of the section computed yet?
	processingOrder: number;
	drawingOrder: number;
	zIndex: number;
	interactable: boolean = true;
	isAnimating: boolean = false;
	isAlwaysVisible: boolean = false;
	windowSection: boolean = false;
	sectionProperties: any = {};
	boundsList: Array<CanvasSectionObject> = []; // The sections those this section can propagate events to. Updated by container.

	constructor(name: string) {
		this.name = name;

		if (this.documentObject) {
			this.documentPosition = cool.SimplePoint.fromCorePixels([...this.position]);
			this.boundingRectangle = cool.SimpleRectangle.fromCorePixels([...this.position, ...this.size]);
		}
	}

	onInitialize(): void { return; }
	onCursorPositionChanged(newPosition: cool.SimpleRectangle): void { return; }
	onCellAddressChanged(): void { return; }
	onMouseMove(point: cool.SimplePoint, dragDistance: Array<number>, e: MouseEvent): void { return; }
	onMouseDown(point: cool.SimplePoint, e: MouseEvent): void { return; }
	onMouseUp(point: cool.SimplePoint, e: MouseEvent): void { return; }

	setShowSection(show: boolean): void {
		this.showSection = show;

		if (this.onSectionShowStatusChange)
			this.onSectionShowStatusChange();

		if (this.containerObject) { // Is section added to container.
			this.isVisible = this.containerObject.isDocumentObjectVisible(this);
			this.onDocumentObjectVisibilityChange();

			if (this.containerObject.testing)
				this.containerObject.createUpdateSingleDivElement(this);
		}
	}

	onSectionShowStatusChange(): void { return; } /// Called when setShowSection is called.

	isSectionShown(): boolean {
		return this.showSection;
	}

	onDocumentObjectVisibilityChange(): void { return; }
	onMouseEnter(point: cool.SimplePoint, e: MouseEvent): void { return; }
	onMouseLeave(point: cool.SimplePoint, e: MouseEvent): void { return; }
	onClick(point: cool.SimplePoint, e: MouseEvent): void { return; }
	onDoubleClick(point: cool.SimplePoint, e: MouseEvent): void { return; }
	onContextMenu(point: cool.SimplePoint, e: MouseEvent): void { return; }
	onMouseWheel(point: cool.SimplePoint, delta: Array<number>, e: WheelEvent): void { return; }
	onMultiTouchStart(e: TouchEvent): void { return; }
	onMultiTouchMove(point: cool.SimplePoint, dragDistance: number, e: TouchEvent): void { return; }
	onMultiTouchEnd(e: TouchEvent): void { return; }
	onDrop(point: cool.SimplePoint, e: DragEvent): void { return; }
	onResize(): void { return; }
	onDraw(frameCount?: number, elapsedTime?: number): void { return; }
	onDrawArea(area?: cool.Bounds, paneTopLeft?: cool.Point, canvasContext?: CanvasRenderingContext2D): void { return; } // area is the area to be painted using canvasContext.
	onAnimate(frameCount: number, elapsedTime: number): void { return; }
	onAnimationEnded(frameCount: number, elapsedTime: number): void { return; } // frameCount, elapsedTime. Sections that will use animation, have to have this function defined.
	onNewDocumentTopLeft(): void { return; }
	onRemove(): void { return; } // This Function is called right before section is removed.
	getHTMLObject(): HTMLElement { return; } // Implemented in HTMLObjectSection.

	setDrawingOrder(drawingOrder: number): void {
		this.drawingOrder = drawingOrder;
		this.containerObject.updateBoundSectionLists();
		this.containerObject.reNewAllSections();
	}

	setZIndex(zIndex: number): void {
		this.zIndex = zIndex;
		this.containerObject.updateBoundSectionLists();
		this.containerObject.reNewAllSections();
	}

	bindToSection(sectionName: string): void {
		this.boundToSection = sectionName;
		this.containerObject.updateBoundSectionLists();
		this.containerObject.reNewAllSections();
	}

	stopPropagating(e: MouseEvent = null): void {
		this.containerObject.lowestPropagatedBoundSection = this.name;

		// We shouldn't need e when we remove map element.
		if (e) { // This addition doesn't effect current uses of this function, since they don't send e here.
			if (e.preventDefault)
				e.preventDefault();

			if (e.stopImmediatePropagation)
				e.stopImmediatePropagation();

			(e as any).preventedDefault = true; // Tap events are first handled by touchGesture. We need to let it know if we handled the event.
		}
	}

	startAnimating(options: any): boolean {
		return this.containerObject.startAnimating(this.name, options);
	}

	resetAnimation(): void {
		this.containerObject.resetAnimation(this.name);
	}

	getTestDiv(): HTMLDivElement {
		var element: HTMLDivElement = <HTMLDivElement>document.getElementById('test-div-' + this.name);
		if (element)
			return element;

		return null;
	}

	setSize(w: number, h: number): void  {
		if (!this.containerObject)
			return;

		w = Math.round(w);
		h = Math.round(h);

		if (this.size[0] === w && this.size[1] === h)
			return;

		this.size = [w, h];
	}

	// Document objects only.
	setPosition(x: number, y: number): void {
		if (this.documentObject !== true || !this.containerObject)
			return;

		x = Math.round(x);
		y = Math.round(y);

		// Setting the position.
		this.position[0] = x;
		this.position[1] = y;
		this.documentPosition = cool.SimplePoint.fromCorePixels([x, y]);
		this.boundingRectangle = cool.SimpleRectangle.fromCorePixels([...[x, y], ...this.size]);

		this.myTopLeft[0] = this.documentPosition.vX;
		this.myTopLeft[1] = this.documentPosition.vY;

		// Visibility check.
		const isVisible = this.containerObject.isDocumentObjectVisible(this);
		if (isVisible !== this.isVisible) {
			this.isVisible = isVisible;
			this.onDocumentObjectVisibilityChange();
		}

		if (this.containerObject.testing)
			this.containerObject.createUpdateSingleDivElement(this);
	}

	/*
		Allow locally to influence if this object is hit by the given point.
		This can be used e.g. to have CanvasSectionObjects with 'holes',
		e.g. a frame around something and you only want the frame to be hittable
	*/
	isHit(point: number[]): boolean {
		const addition = (this.documentObject === true && app.map._docLayer.isCalcRTL()) ? -this.size[0] : 0;

		// return result of inside local range (position, size) check
		return (
			(point[0] >= this.myTopLeft[0] + addition && point[0] <= this.myTopLeft[0] + addition + this.size[0]) &&
			(point[1] >= this.myTopLeft[1] && point[1] <= this.myTopLeft[1] + this.size[1])
		);
	}

	goToSection() {
		const point = new cool.SimplePoint(this.position[0] * app.pixelsToTwips, this.position[1] * app.pixelsToTwips);
		var isNewCursorVisible = app.isPointVisibleInTheDisplayedArea(point.toArray());
		if (!isNewCursorVisible)
			app.map._docLayer.scrollToPos(point);
	}

	// All below functions should be included in their respective section definitions (or other classes), not here.
	isCalcRTL(): boolean { return; }
	setViewResolved(on: boolean): void { return; }
	setView(on: boolean): void { return; }
	scrollVerticalWithOffset(offset: number): void { return; }
	remove(id: string): void { return; }
	deleteThis(): void { return; }
	getActiveEdit(): any { return; }
	isMobileCommentActive(): boolean { return false; }
	getMobileCommentModalId(): string { return ''; }
	rejectAllTrackedCommentChanges(): void { return; }
	removeHighlighters(): void { return; }
	showUsernamePopUp(): void { return; }
	_selectColumn (colNumber: number, modifier: number): void { return; }
	_selectRow (row: number, modifier: number): void { return; }
	insertColumnBefore (index: number): void { return; }
	insertRowAbove (index: number): void { return; }
	deleteColumn (index: number): void { return; }
	deleteRow (index: number): void { return; }
	resetStrokeStyle(): void { return; }
	hasAnyComments(): boolean { return false; }

	/// Updates sectionProperties.polygonColor based on the current dark/light theme.
	protected changeBorderStyle(): void {
		const polygonColor = (<any>window).prefs.getBoolean('darkTheme') ? 'white' : 'black';
		if (this.sectionProperties.polygonColor !== polygonColor)
			this.sectionProperties.polygonColor = polygonColor;
	}

	/// Strokes the polygon stored in sectionProperties.polygon using sectionProperties.polygonColor.
	protected drawPolygon(): void {
		this.context.strokeStyle = this.sectionProperties.polygonColor;
		this.context.beginPath();
		this.context.moveTo(this.sectionProperties.polygon[0] - this.position[0], this.sectionProperties.polygon[0 + 1] - this.position[1]);
		for (let i = 0; i < this.sectionProperties.polygon.length - 1; i++) {
			this.context.lineTo(this.sectionProperties.polygon[i] - this.position[0], this.sectionProperties.polygon[i + 1] - this.position[1]);
			i += 1;
		}
		this.context.closePath();
		this.context.stroke();
	}

	/// Sets position and size from an array of twip rectangles [x, y, width, height].
	protected setPositionAndSizeFromTwipRectangles(rectangles: Array<number[]>): void {
		var xMin: number = Infinity, yMin: number = Infinity, xMax: number = 0, yMax: number = 0;
		for (var i = 0; i < rectangles.length; i++) {
			if (rectangles[i][0] < xMin)
				xMin = rectangles[i][0];

			if (rectangles[i][1] < yMin)
				yMin = rectangles[i][1];

			if (rectangles[i][0] + rectangles[i][2] > xMax)
				xMax = rectangles[i][0] + rectangles[i][2];

			if (rectangles[i][1] + rectangles[i][3] > yMax)
				yMax = rectangles[i][1] + rectangles[i][3];
		}

		xMin = Math.round(xMin * app.twipsToPixels);
		yMin = Math.round(yMin * app.twipsToPixels);
		xMax = Math.round(xMax * app.twipsToPixels);
		yMax = Math.round(yMax * app.twipsToPixels);

		this.setPosition(xMin, yMin);
		this.size = [xMax - xMin, yMax - yMin];
		if (this.size[0] < 5)
			this.size[0] = 5;
	}

	public getLineWidth(): number {
		if (app.dpiScale > 1.0) {
			return app.roundedDpiScale;
		} else {
			return app.dpiScale;
		}
	}

	public getLineOffset(): number {
		if (app.dpiScale > 1.0) {
			return app.roundedDpiScale % 2 === 0 ? 0 : 0.5;
		} else {
			return 0.5;
		}
	}

	public drawViewRectangle(rectangle: cool.SimpleRectangle, fill = false) {
		if (fill) {
			this.context.beginPath();
			this.context.moveTo(rectangle.v1X, rectangle.v1Y);
			this.context.lineTo(rectangle.v2X, rectangle.v2Y);
			this.context.lineTo(rectangle.v4X, rectangle.v4Y);
			this.context.lineTo(rectangle.v3X, rectangle.v3Y);
			this.context.closePath();
			this.context.fill();
		} else {
			this.context.beginPath();
			this.context.moveTo(rectangle.v1X - 0.5, rectangle.v1Y - 0.5);
			this.context.lineTo(rectangle.v2X - 0.5, rectangle.v2Y - 0.5);
			this.context.lineTo(rectangle.v4X - 0.5, rectangle.v4Y - 0.5);
			this.context.lineTo(rectangle.v3X - 0.5, rectangle.v3Y - 0.5);
			this.context.closePath();
			this.context.stroke();
		}
	}
}
