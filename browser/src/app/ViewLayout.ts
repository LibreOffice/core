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

class ScrollProperties {
	yOffset: number = 0;
	verticalScrollLength: number = 0;
	verticalScrollSize: number = 0; // Clamped to minimum, used for drawing.
	verticalScrollSizeForScrolling: number = 0; // Unclamped, used for scroll calculations.
	minimumVerticalScrollSize: number = 80 * app.roundedDpiScale;
	verticalScrollRatio: number = 0;
	startY: number = 0; // Start position of the vertical scroll bar on canvas.
	viewY: number = 0; // Corresponding view position of startY.
	verticalScrollStep: number = 0; // Quick scroll step.

	xOffset: number = 0;
	horizontalScrollLength: number = 0;
	horizontalScrollSize: number = 0; // Clamped to minimum, used for drawing.
	horizontalScrollSizeForScrolling: number = 0; // Unclamped, used for scroll calculations.
	minimumHorizontalScrollSize: number = 80 * app.roundedDpiScale;
	horizontalScrollRatio: number = 0;
	startX: number = 0;
	viewX: number = 0; // Corresponding view position of startX.
	horizontalScrollStep: number = 0;

	usableThickness: number = 20 * app.roundedDpiScale;
	horizontalScrollRightOffset: number = 20 /*usableThickness*/; // To prevent overlapping of the scroll bars.
	scrollBarThickness: number = 6 * app.roundedDpiScale;
	edgeOffset: number = 0;

	moveBy: number[] | null = null; // Pending move event (pX, pY).
}

// FIXME: should be abstract to split Writer and other layouts
// so we can have abstract methods and be warned about missing bits
class ViewLayoutBase {
	public readonly type: string = 'ViewLayoutBase';

	private lastViewedRectangle: cool.SimpleRectangle; // Previously viewed rectangle.

	protected clientVisibleAreaCommand: string = ''; // Last visible area command. Checked to avoid sending the same command multiple times.
	protected _viewedRectangle: cool.SimpleRectangle; // Currently viewed rectangle.
	protected _viewSize: cool.SimplePoint; // Scrollable area.
	protected _documentAnchorPosition: number[]; // The position of document section on the canvas. Always canvas (core) pixels, no need for SimplePoint class.
	public scrollProperties: ScrollProperties = new ScrollProperties();
	protected currentCoordList: Array<TileCoordData> = [];

	constructor() {
		this._viewedRectangle = new cool.SimpleRectangle(0, 0, 0, 0);
		this.lastViewedRectangle = new cool.SimpleRectangle(0, 0, 0, 0);
		this._viewSize = new cool.SimplePoint(0, 0);
		this._documentAnchorPosition = [0, 0];
	}

	/*
		View layout may choose to render the tiles in different coordinates.
		In that case, the tiles' coordinates will differ from the file's coordinates.
		There are x, pX and cX. View layout also adds vX.
			x : Document coordinate system
			pX: Corresponding pixel coordinate system on canvas. x and pX have a fixed ratio.
			cX: Corresponding CSS coordinate system on canvas. x and cX have a fixed ratio.
			vX: View coordinate system. Current view layout decides on the mapping between x (document coordinate) and vX (view coordinate).
				View coordinate system also uses canvas pixels as unit (like pX).

		Below functions are used to convert between those coordinate systems.
		This is the base class. Does nothing special but provide the interface.
	*/
	public documentToViewX(point: cool.SimplePoint): number {
		return (
			point.pX - this._viewedRectangle.pX1 + this._documentAnchorPosition[0]
		);
	}

	public adjustViewZoomLevel() {
		if (app.map._docLayer)
			app.map._docLayer._fitWidthZoom(undefined, undefined, true);
	}

	public documentToViewY(point: cool.SimplePoint): number {
		return (
			point.pY - this._viewedRectangle.pY1 + this._documentAnchorPosition[1]
		);
	}

	// point: Unmodified canvas coordinate, no scroll included.
	public canvasToDocumentPoint(point: cool.SimplePoint): cool.SimplePoint {
		const result = point.clone();

		result.pX =
			point.pX + this._viewedRectangle.pX1 - this._documentAnchorPosition[0];
		result.pY =
			point.pY + this._viewedRectangle.pY1 - this._documentAnchorPosition[1];

		return result;
	}

	public resetClientVisibleArea(): void {
		this.lastViewedRectangle = new cool.SimpleRectangle(0, 0, 0, 0);
	}

	public sendClientVisibleArea(forceUpdate: boolean = false): void {
		if (!app.map._docLoaded) return;

		var splitPos = app.map._docLayer._splitPanesContext
			? app.map._docLayer._splitPanesContext.getSplitPos()
			: new cool.Point(0, 0);

		var visibleArea = app.map.getPixelBounds();
		visibleArea = new cool.Bounds(
			app.map._docLayer._pixelsToTwips(visibleArea.min),
			app.map._docLayer._pixelsToTwips(visibleArea.max),
		);
		splitPos = app.map._docLayer._corePixelsToTwips(splitPos);
		var size = visibleArea.getSize();
		var visibleTopLeft = visibleArea.min;
		var newClientVisibleAreaCommand =
			'clientvisiblearea x=' +
			Math.round(visibleTopLeft.x) +
			' y=' +
			Math.round(visibleTopLeft.y) +
			' width=' +
			Math.round(size.x) +
			' height=' +
			Math.round(size.y) +
			' splitx=' +
			Math.round(splitPos.x) +
			' splity=' +
			Math.round(splitPos.y);

		if (
			this.clientVisibleAreaCommand !== newClientVisibleAreaCommand ||
			forceUpdate
		) {
			// Only update on some change
			if (app.map._docLayer._ySplitter) {
				app.map._docLayer._ySplitter.onPositionChange();
			}
			if (app.map._docLayer._xSplitter) {
				app.map._docLayer._xSplitter.onPositionChange();
			}
			// Visible area is dirty, update it on the server
			app.socket.sendMessage(newClientVisibleAreaCommand);
			if (app.map.contextToolbar) app.map.contextToolbar.hideContextToolbar(); // hide context toolbar when scroll/window resize etc...
			if (!app.map._fatal && app.idleHandler._active && app.socket.connected())
				this.clientVisibleAreaCommand = newClientVisibleAreaCommand;
		}
	}

	public getLastPanDirection(): Array<number> {
		var dx: number = this._viewedRectangle.pX1 - this.lastViewedRectangle.pX1;
		var dy: number = this._viewedRectangle.pY1 - this.lastViewedRectangle.pY1;
		return [Math.sign(dx), Math.sign(dy)];
	}

	public get viewedRectangle() {
		return this._viewedRectangle;
	}

	public set viewedRectangle(rectangle: cool.SimpleRectangle) {
		// maintain a view of where we're panning to.
		if (!this._viewedRectangle.equals(rectangle.toArray()))
			this.lastViewedRectangle = this._viewedRectangle.clone();

		this._viewedRectangle = rectangle;

		app.sectionContainer.onNewDocumentTopLeft();
		app.sectionContainer.requestReDraw();
	}

	public get viewSize() {
		return this._viewSize;
	}

	public set viewSize(size: cool.SimplePoint) {
		this._viewSize = size;
	}

	public get documentAnchorPosition() {
		return this._documentAnchorPosition.slice();
	}

	public set documentAnchorPosition(newPosition: number[]) {
		this._documentAnchorPosition = newPosition;
	}

	protected getDocumentAnchorSection(): CanvasSectionObject {
		return app.sectionContainer.getDocumentAnchorSection();
	}

	/// used in Views which can show pages (Writer) to determine left alignment
	// FIXME: confusing name, should be abstract
	public getDocumentScrollOffset(): number {
		app.console.error('not implemented');
		return 0;
	}

	private calculateHorizontalScrollLength(
		documentAnchor: CanvasSectionObject,
	): void {
		const canvasWidth: number = documentAnchor.size[0];
		this.scrollProperties.xOffset = documentAnchor.myTopLeft[0];

		if (app.map._docLayer._docType === 'spreadsheet') {
			var splitPanesContext: any = app.map.getSplitPanesContext();
			var splitPos = { x: 0, y: 0 };
			if (splitPanesContext) {
				splitPos = splitPanesContext.getSplitPos().clone();
				splitPos.x = Math.round(splitPos.x * app.dpiScale);
			}

			this.scrollProperties.xOffset += splitPos.x;
			this.scrollProperties.horizontalScrollLength =
				canvasWidth -
				splitPos.x -
				this.scrollProperties.horizontalScrollRightOffset;
		} else {
			this.scrollProperties.horizontalScrollLength =
				canvasWidth - this.scrollProperties.horizontalScrollRightOffset;
		}
	}

	private calculateVerticalScrollLength(
		documentAnchor: CanvasSectionObject,
	): void {
		const result: number = documentAnchor.size[1];
		this.scrollProperties.yOffset = documentAnchor.myTopLeft[1];

		if (app.map._docLayer._docType !== 'spreadsheet') {
			this.scrollProperties.verticalScrollLength = result;
		} else {
			const splitPanesContext: any = app.map.getSplitPanesContext();
			let splitPos = { x: 0, y: 0 };
			if (splitPanesContext) {
				splitPos = splitPanesContext.getSplitPos().clone();
				splitPos.y = Math.round(splitPos.y * app.dpiScale);
			}

			this.scrollProperties.yOffset += splitPos.y;
			this.scrollProperties.verticalScrollLength = result - splitPos.y;
		}
	}

	protected calculateTheScrollSizes() {
		// Sizes of the scroll bars.
		this.scrollProperties.verticalScrollSize = Math.round(
			Math.pow(this.scrollProperties.verticalScrollLength, 2) /
				this.viewSize.pY,
		);
		this.scrollProperties.horizontalScrollSize = Math.round(
			Math.pow(this.scrollProperties.horizontalScrollLength, 2) /
				this.viewSize.pX,
		);

		// Store unclamped values for scroll calculations.
		this.scrollProperties.verticalScrollSizeForScrolling =
			this.scrollProperties.verticalScrollSize;
		this.scrollProperties.horizontalScrollSizeForScrolling =
			this.scrollProperties.horizontalScrollSize;

		// Clamp to minimum for drawing.
		if (
			this.scrollProperties.horizontalScrollSize <
			this.scrollProperties.minimumHorizontalScrollSize
		)
			this.scrollProperties.horizontalScrollSize =
				this.scrollProperties.minimumHorizontalScrollSize;

		if (
			this.scrollProperties.verticalScrollSize <
			this.scrollProperties.minimumVerticalScrollSize
		)
			this.scrollProperties.verticalScrollSize =
				this.scrollProperties.minimumVerticalScrollSize;
	}

	public refreshScrollProperties(): any {
		const documentAnchor = this.getDocumentAnchorSection();

		// The length of the railway that the scroll bar moves on up & down or left & right.
		this.calculateVerticalScrollLength(documentAnchor);
		this.calculateHorizontalScrollLength(documentAnchor);

		// Sizes of the scroll bars.
		this.calculateTheScrollSizes();

		// 1px scrolling = xpx document height / width.
		this.scrollProperties.horizontalScrollRatio =
			(this.viewSize.pX - documentAnchor.size[0]) /
			(this.scrollProperties.horizontalScrollLength -
				this.scrollProperties.horizontalScrollSize);
		this.scrollProperties.verticalScrollRatio =
			(this.viewSize.pY - documentAnchor.size[1]) /
			(this.scrollProperties.verticalScrollLength -
				this.scrollProperties.verticalScrollSize);

		// The start position of scroll bars on canvas.
		this.scrollProperties.startX =
			this.viewedRectangle.pX1 / this.scrollProperties.horizontalScrollRatio +
			this.scrollProperties.xOffset;

		this.scrollProperties.startY =
			this.viewedRectangle.pY1 / this.scrollProperties.verticalScrollRatio +
			this.scrollProperties.yOffset;

		// Properties for quick scrolling.
		this.scrollProperties.verticalScrollStep = documentAnchor.size[1] / 2;
		this.scrollProperties.horizontalScrollStep = documentAnchor.size[0] / 2;
	}

	// This function doesn't do well. It seems we don't need this function.
	// Tiles middleware seems to manage redraw requests when tiles are ready.
	public areViewTilesReady(): boolean {
		let allReady = true;

		for (let i = 0; i < this.currentCoordList.length; i++) {
			const tempTile = TileManager.get(this.currentCoordList[i]);

			if (!tempTile || !tempTile.isReady()) allReady = false;
		}

		return allReady;
	}

	protected refreshCurrentCoordList() {
		this.currentCoordList.length = 0;
		const zoom = Math.round(app.map.getZoom());

		const columnCount = Math.ceil(
			this._viewedRectangle.pWidth / TileManager.tileSize,
		);
		const rowCount = Math.ceil(
			this._viewedRectangle.pHeight / TileManager.tileSize,
		);
		const startX =
			Math.floor(this._viewedRectangle.pX1 / TileManager.tileSize) *
			TileManager.tileSize;
		const startY =
			Math.floor(this._viewedRectangle.pY1 / TileManager.tileSize) *
			TileManager.tileSize;

		for (let i = 0; i <= columnCount; i++) {
			for (let j = 0; j <= rowCount; j++) {
				const coords = new TileCoordData(
					startX + i * TileManager.tileSize,
					startY + j * TileManager.tileSize,
					zoom,
					0,
				);

				if (TileManager.isValidTile(coords)) this.currentCoordList.push(coords);
			}
		}
	}

	public getCurrentCoordList(): Array<TileCoordData> {
		return this.currentCoordList;
	}

	// virtual function implemented by the children
	public viewHasEnoughSpaceToShowFullWidthComments() {
		return false;
	}

	// virtual function implemented by the children
	public unselectCommentOnScroll() {
		return;
	}

	private addToMoveBy(pX: number, pY: number) {
		if (this.scrollProperties.moveBy !== null) {
			// Add offset to the pending move event.
			if (pX !== 0) {
				this.scrollProperties.moveBy[0] += pX;
			}
			if (pY !== 0) {
				this.scrollProperties.moveBy[1] += pY;
			}
		} else {
			// Create a new pending move event.
			this.scrollProperties.moveBy = [pX, pY];
		}
	}
	/*
		`ignoreScrollbarLength` constraints while scrolling the document to make some space for the comments.
		see `ViewLayoutWriter.adjustDocumentMarginsForComments`
	*/
	protected scrollHorizontal(
		pX: number,
		ignoreScrollbarLength: boolean = false,
	): void {
		const scrollProps: ScrollProperties = this.scrollProperties;
		const psX = pX / scrollProps.horizontalScrollRatio;
		if (document.documentElement.dir === 'rtl') pX = -pX;

		if (!ignoreScrollbarLength) {
			let control = scrollProps.moveBy ? scrollProps.moveBy[0] : 0; // Add pending offset.
			control /= scrollProps.horizontalScrollRatio; // Convert to scroll bar position diff.

			const endPosition =
				scrollProps.startX - scrollProps.xOffset + control + psX;
			if (pX > 0) {
				if (
					endPosition + scrollProps.horizontalScrollSize >
					scrollProps.horizontalScrollLength
				)
					pX =
						(scrollProps.horizontalScrollLength -
							scrollProps.horizontalScrollSize -
							scrollProps.startX +
							scrollProps.xOffset -
							control) *
						scrollProps.horizontalScrollRatio;

				if (pX < 0) pX = 0;
			} else {
				if (endPosition < 0)
					pX =
						(scrollProps.startX - scrollProps.xOffset + control) *
						-1 *
						scrollProps.horizontalScrollRatio;

				if (pX > 0) pX = 0;
			}
		}

		this.addToMoveBy(pX, 0);
	}

	// For scrolling with screen offset.
	// This function shouldn't care about the document content, size etc.
	// All this cares is the current scroll position and the scroll length.
	// For making a portion of the document visible, use other methods.
	protected scrollVertical(pY: number): void {
		const scrollProps: ScrollProperties = this.scrollProperties;

		let control = scrollProps.moveBy ? scrollProps.moveBy[1] : 0; // Add pending offset.
		control /= scrollProps.verticalScrollRatio; // Convert to scroll bar position diff.

		const psY = pY / scrollProps.verticalScrollRatio;

		const endPosition =
			scrollProps.startY - scrollProps.yOffset + control + psY;

		if (pY > 0) {
			if (
				endPosition + scrollProps.verticalScrollSize >
				scrollProps.verticalScrollLength
			)
				pY =
					(scrollProps.verticalScrollLength -
						scrollProps.verticalScrollSize -
						scrollProps.startY +
						scrollProps.yOffset -
						control) *
					scrollProps.verticalScrollRatio;

			if (pY < 0) pY = 0;
		} else {
			if (endPosition < 0)
				pY =
					(scrollProps.startY - scrollProps.yOffset + control) *
					-1 *
					scrollProps.verticalScrollRatio;

			if (pY > 0) pY = 0;
		}

		this.addToMoveBy(0, pY);
	}

	public canScrollHorizontal(documentAnchor: CanvasSectionObject): boolean {
		return this.viewSize.pX > documentAnchor.size[0];
	}

	public canScrollVertical(documentAnchor: CanvasSectionObject): boolean {
		return this.viewSize.pY > documentAnchor.size[1];
	}

	public scroll(pX: number, pY: number): void {
		this.refreshScrollProperties();
		const documentAnchor = this.getDocumentAnchorSection();

		if (pX !== 0 && this.canScrollHorizontal(documentAnchor))
			this.scrollHorizontal(pX);

		if (pY !== 0 && this.canScrollVertical(documentAnchor))
			this.scrollVertical(pY);

		this.refreshCurrentCoordList();
		this.unselectCommentOnScroll();
		app.sectionContainer.requestReDraw();
	}

	public scrollTo(pX: number, pY: number): void {
		this.refreshScrollProperties();

		this.scrollProperties.moveBy = null;

		pX -= this.viewedRectangle.pX1;
		pY -= this.viewedRectangle.pY1;

		this.scroll(pX, pY);
	}

	public setOverviewPageVisArea(point: cool.SimplePoint): void {
		this.scrollTo(point.pX, point.pY);
	}
}
