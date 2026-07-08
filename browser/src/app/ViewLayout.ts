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
	// Where the vertical scroll bar railway is drawn: the whole document
	// anchor, unlike yOffset/verticalScrollLength, which stop at the edge of
	// a frozen pane so the thumb does not travel into it.
	verticalScrollRailwayOffset: number = 0;
	verticalScrollRailwayLength: number = 0;
	verticalScrollSize: number = 0; // Clamped to minimum, used for drawing.
	verticalScrollSizeForScrolling: number = 0; // Unclamped, used for scroll calculations.
	minimumVerticalScrollSize: number = 80 * app.roundedDpiScale;
	verticalScrollRatio: number = 0;
	startY: number = 0; // Start position of the vertical scroll bar on canvas.
	viewY: number = 0; // Corresponding view position of startY.
	verticalScrollStep: number = 0; // Quick scroll step.

	xOffset: number = 0;
	horizontalScrollLength: number = 0;
	// Where the horizontal scroll bar railway is drawn: the whole document
	// anchor, unlike xOffset/horizontalScrollLength, which stop at the edge
	// of a frozen pane so the thumb does not travel into it.
	horizontalScrollRailwayOffset: number = 0;
	horizontalScrollRailwayLength: number = 0;
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

	protected lastViewedRectangle: cool.SimpleRectangle; // Previously viewed rectangle.

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

	protected getVisibleAreaBounds() {
		let visibleArea = app.map.getPixelBounds();
		visibleArea = new cool.Bounds(
			app.map._docLayer._pixelsToTwips(visibleArea.min),
			app.map._docLayer._pixelsToTwips(visibleArea.max),
		);
		return visibleArea;
	}

	public sendClientVisibleArea(forceUpdate: boolean = false): void {
		if (!app.map._docLoaded) return;
		// During a zoom animation app.twipsToPixels and the viewedRectangle are
		// driven through intermediate frame values; do not push those to core.
		// The real visible area is sent once the animation settles.
		if (app.sectionContainer.isInZoomAnimation()) return;

		var splitPos = app.map._docLayer._splitPanesContext
			? app.map._docLayer._splitPanesContext.getSplitPos()
			: new cool.Point(0, 0);

		const visibleArea = this.getVisibleAreaBounds();

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
			Math.round(size.y);

		if (
			this.clientVisibleAreaCommand !== newClientVisibleAreaCommand ||
			forceUpdate
		) {
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

	// Refresh cached geometry that depends on the current viewed rectangle.
	public refreshViewArea(): void {
		return;
	}

	// Lightweight viewedRectangle update used per zoom frame. Unlike the setter
	// it does not run onNewDocumentTopLeft, requestReDraw or touch
	// lastViewedRectangle: the zoom RAF already redraws, and the document
	// sections read this rectangle directly via documentToViewX/Y.
	public setZoomFrameViewedRectangle(rectangle: cool.SimpleRectangle): void {
		this._viewedRectangle = rectangle;
	}

	public get viewSize() {
		return this._viewSize;
	}

	public set viewSize(size: cool.SimplePoint) {
		this._viewSize = size;
	}

	// The visible frame the document is rendered into: the document anchor
	// (tiles) section's on-canvas area. Unlike viewSize, which is the whole
	// scrollable extent and can be larger than the document, this is the
	// viewport. Returned as a SimplePoint so callers read core (pX/pY) or CSS
	// (cX/cY) pixels without a manual dpiScale conversion.
	public get frameSize(): cool.SimplePoint {
		const anchor = this.getDocumentAnchorSection();
		if (!anchor) return new cool.SimplePoint(0, 0);
		return cool.SimplePoint.fromCorePixels([anchor.size[0], anchor.size[1]]);
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

	// New-structure (off-map) layouts drive their own viewed rectangle and zoom
	// through ZoomControl instead of the leaflet map, and override this to return
	// true (see ViewLayoutNewBase). ViewLayoutBase itself is no longer
	// instantiated directly - every document layout now extends ViewLayoutNewBase
	// - so this base value is only a default.
	public usesZoomControl(): boolean {
		return false;
	}

	// ---- Zoom (driven by ZoomControl) ----------------------------------
	// Shared, map-free zoom for the new-structure layouts. ZoomControl calls
	// these; the data (scale, viewed rectangle, tile requests) lives here so the
	// map is not involved. This is the single-scrollable-window model (Calc,
	// single-page). Stacked-page layouts (ViewLayoutMultiPage/FileBased) whose
	// viewed rectangle is computed from page geometry override the positioning
	// (setViewRectangleFromPointAndScale) and applyZoom as needed.

	// Rebuild the viewed rectangle for a new zoom: centre it on the given
	// document-space point and size it to the current frame at the given scale.
	// point is in twips, scale is the twips-to-core-pixel factor for the new
	// zoom.
	public setViewRectangleFromPointAndScale(
		point: cool.SimplePoint,
		scale: number,
	): void {
		if (!scale) return;
		const frame = this.frameSize;
		const widthTwips = Math.round(frame.pX / scale);
		const heightTwips = Math.round(frame.pY / scale);

		// Centre on the point, but clamp the top-left to the scrollable range
		// [0, viewSize - frame] just like scroll() does. Without this an anchor
		// near the top/left edge produces a negative top-left, which shifts the
		// content and leaves a blank gap on that side.
		const maxX = Math.max(0, this._viewSize.x - widthTwips);
		const maxY = Math.max(0, this._viewSize.y - heightTwips);
		const x = Math.min(maxX, Math.max(0, Math.round(point.x - widthTwips / 2)));
		const y = Math.min(
			maxY,
			Math.max(0, Math.round(point.y - heightTwips / 2)),
		);

		this.viewedRectangle = new cool.SimpleRectangle(
			x,
			y,
			widthTwips,
			heightTwips,
		);
	}

	// The document-space point (twips) a zoom should keep fixed on screen.
	// Defaults to the current viewport centre; callers (cursor/cell/pointer)
	// may pass their own anchor.
	public zoomAnchorPoint(): cool.SimplePoint {
		const c = this._viewedRectangle.center;
		return new cool.SimplePoint(c[0], c[1]);
	}

	// Start a zoom-frame animation pivoted on anchorTwips. Snapshots the start
	// state (preZoomAnimation) and switches the tiles section into zoom-frame
	// drawing. The animation is then driven frame by frame by ZoomControl
	// through the section container's animation loop - NOT the tile painter's
	// own requestAnimationFrame. The scale for each frame comes from stepZoom.
	public beginZoom(anchorTwips: cool.SimplePoint): void {
		const docLayer: any = app.map._docLayer; // _painter is not on the typed interface.
		const painter = docLayer._painter;

		// The anchor is already in core pixels (SimplePoint.pX/pY) - no intern/CRS
		// round-trip needed. This is both the pinch pivot and the frame centre.
		const centerCorePx = new cool.Point(anchorTwips.pX, anchorTwips.pY);

		// Snapshot the starting scale / view bounds and hide the cursor.
		docLayer.preZoomAnimation(null, centerCorePx);

		painter._newCenter = centerCorePx;
		painter._inZoomAnim = true;

		// Publish the scale the tiles section uses for the zoom frame, decoupled
		// from the painter's _zoomFrameScale: the base twips->px it started from
		// and the effective (animated) one. drawZoomFrame derives its bitmap
		// ratio from these; stepZoom updates the effective value each frame.
		const tiles: any = app.sectionContainer.getSectionWithName(
			app.CSections.Tiles.name,
		);
		if (tiles) {
			tiles.sectionProperties.zoomBaseTwipsToPixels = app.twipsToPixels;
			tiles.sectionProperties.effectiveTwipsToPixels = app.twipsToPixels;
		}

		app.sectionContainer.setInZoomAnimation(true);
	}

	// One animation frame: `scale` is the factor relative to the zoom start.
	// TilesSection.drawZoomFrame reads it while the container redraws this frame.
	public stepZoom(scale: number): void {
		// The effective twips->px the tiles section renders this frame at. The
		// zoom-frame consumers (tiles, grid, headers, overlay) read the ratio
		// from this via tsManager.zoomFrameScale().
		const tiles: any = app.sectionContainer.getSectionWithName(
			app.CSections.Tiles.name,
		);
		if (tiles && tiles.sectionProperties.zoomBaseTwipsToPixels)
			tiles.sectionProperties.effectiveTwipsToPixels =
				tiles.sectionProperties.zoomBaseTwipsToPixels * scale;
	}

	// Finish the animation: leave zoom-frame mode, commit the final zoom
	// map-free and restore the cursor.
	public endZoom(targetZoom: number, anchorTwips: cool.SimplePoint): void {
		const docLayer: any = app.map._docLayer;
		const painter = docLayer._painter;

		painter._inZoomAnim = false;

		const tiles: any = app.sectionContainer.getSectionWithName(
			app.CSections.Tiles.name,
		);
		if (tiles) tiles.sectionProperties.effectiveTwipsToPixels = undefined;

		app.sectionContainer.setInZoomAnimation(false);

		this.applyZoom(targetZoom, anchorTwips);
		docLayer.postZoomAnimation();
	}

	// Commit the final integral zoom: set the new scale, rebuild the viewed
	// rectangle around anchorTwips at that scale, and refresh tiles/headers.
	// Used by both the animated end and the non-animated path. Map-free: the
	// scale is applied via _updateTileTwips (not _resetView) and the doc type's
	// 'zoomend' listeners do the rest of the post-zoom work.
	public applyZoom(targetZoom: number, anchorTwips: cool.SimplePoint): void {
		// The zoom internals used here (_zoom, _clientZoom, _tileZoom,
		// _updateTileTwips) are not on the typed interfaces; cast for now while
		// the map is still being removed from the zoom path.
		const map: any = app.map;
		const docLayer: any = map._docLayer;
		const zoom = map._limitZoom(targetZoom);
		const zoomChanged = map._zoom !== zoom;

		map._zoom = zoom;
		map._clientZoom = zoom; // fallback used when setZoom(0) is called later.
		docLayer._tileZoom = Math.round(zoom);

		// Recompute app.twipsToPixels for the new zoom (no map pane involved) and
		// re-sync the canvas, then rebuild the viewed rectangle around the anchor
		// at that scale.
		docLayer._updateTileTwips();
		this.setViewRectangleFromPointAndScale(anchorTwips, app.twipsToPixels);

		// The doc type's 'zoomend' listeners do the post-zoom work (for Calc:
		// _onZoomRowColumns -> client zoom, _restrictDocumentSize, view
		// data/tiles; plus the status-bar percentage and cursor follow). They run
		// only when the scale really moved: a resize recomputes the fit zoom of a
		// presentation on every container change and usually lands on the value
		// that is already set, and post-zoom work on an unchanged scale disturbs
		// the open user interface.
		if (zoomChanged) {
			map.fire('zoomend');
			map.fire('zoomlevelschange');
		}
	}

	private calculateHorizontalScrollLength(
		documentAnchor: CanvasSectionObject,
	): void {
		const canvasWidth: number = documentAnchor.size[0];
		this.scrollProperties.xOffset = documentAnchor.myTopLeft[0];

		this.scrollProperties.horizontalScrollRailwayOffset =
			documentAnchor.myTopLeft[0];
		this.scrollProperties.horizontalScrollRailwayLength =
			canvasWidth - this.scrollProperties.horizontalScrollRightOffset;

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

		this.scrollProperties.verticalScrollRailwayOffset =
			documentAnchor.myTopLeft[1];
		this.scrollProperties.verticalScrollRailwayLength = result;

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

	protected refreshCurrentCoordList() {
		this.currentCoordList.length = 0;
		const zoom = Math.round(app.map.getZoom());

		const columnCount = Math.ceil(
			this._viewedRectangle.pWidth / RenderManager.tileSize,
		);
		const rowCount = Math.ceil(
			this._viewedRectangle.pHeight / RenderManager.tileSize,
		);
		const startX =
			Math.floor(this._viewedRectangle.pX1 / RenderManager.tileSize) *
			RenderManager.tileSize;
		const startY =
			Math.floor(this._viewedRectangle.pY1 / RenderManager.tileSize) *
			RenderManager.tileSize;

		// The coordinates name tiles of the part the view currently shows:
		// part 0 for a text document, the selected sheet for a spreadsheet,
		// and the shown page's unique id for a presentation or drawing.
		const part = app.map._docLayer.getSelectedPart();

		for (let i = 0; i <= columnCount; i++) {
			for (let j = 0; j <= rowCount; j++) {
				const coords = new TileCoordData(
					startX + i * RenderManager.tileSize,
					startY + j * RenderManager.tileSize,
					zoom,
					part,
				);

				if (RenderManager.isValidTile(coords))
					this.currentCoordList.push(coords);
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
		`ignoreScrollbarLength` relaxes the scrollbar-length constraints while
		scrolling the document to make some space for the comments.
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

	public scroll(
		pX: number,
		pY: number,
		userIsScrolling: boolean = false,
	): void {
		// While a zoom is waiting for its new tiles the canvas is not cleared (to
		// avoid white flicker), so scrolling in that window leaves smears.
		// Force a clear in that case; the flag is reset once the zoom finishes.
		if (
			app.sectionContainer.isZoomChanged() &&
			!app.sectionContainer.isInZoomAnimation()
		)
			app.sectionContainer.setScrollingBeforeZoomSettled(true);
		if (userIsScrolling) this.unselectCommentOnScroll();
		this.refreshScrollProperties();
		const documentAnchor = this.getDocumentAnchorSection();

		if (pX !== 0 && this.canScrollHorizontal(documentAnchor))
			this.scrollHorizontal(pX);

		if (pY !== 0 && this.canScrollVertical(documentAnchor))
			this.scrollVertical(pY);

		this.refreshCurrentCoordList();
		app.sectionContainer.requestReDraw();
	}

	public scrollTo(
		pX: number,
		pY: number,
		userIsScrolling: boolean = false,
	): void {
		this.refreshScrollProperties();

		this.scrollProperties.moveBy = null;

		pX -= this.viewedRectangle.pX1;
		pY -= this.viewedRectangle.pY1;

		this.scroll(pX, pY, userIsScrolling);
	}

	public setOverviewPageVisArea(point: cool.SimplePoint): void {
		this.scrollTo(point.pX, point.pY);
	}
}
