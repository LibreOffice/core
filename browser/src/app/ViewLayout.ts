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
}

// FIXME: should be abstract to split Writer and other layouts
// so we can have abstract methods and be warned about missing bits
class ViewLayoutBase {
	public readonly type: string = 'ViewLayoutBase';

	protected lastViewedRectangle: cool.SimpleRectangle; // Previously viewed rectangle.

	protected clientVisibleAreaCommand: string = ''; // Last visible area command. Checked to avoid sending the same command multiple times.
	protected _viewedRectangle: cool.SimpleRectangle; // Currently viewed rectangle.
	protected _viewSize: cool.SimplePoint; // Scrollable area (document extent plus any comment overflow).
	// Scrollable extent WITHOUT comment overflow - the layout's own document/page
	// extent. Stacked-page layouts capture it when they recompute their layout;
	// side-space and page-positioning read this stable value so a comment-inflated
	// viewSize never feeds back into them. See getBaseViewSize.
	protected _baseViewSize: cool.SimplePoint;
	protected _documentAnchorPosition: number[]; // The position of document section on the canvas. Always canvas (core) pixels, no need for SimplePoint class.
	public scrollProperties: ScrollProperties = new ScrollProperties();
	protected currentCoordList: Array<TileCoordData> = [];
	// app.events is a bridge over DOM events, so the resize handler is kept bound
	// here: off has to be given the same function reference that on registered.
	private readonly boundOnResize = () => this.onResize();

	constructor() {
		this._viewedRectangle = new cool.SimpleRectangle(0, 0, 0, 0);
		this.lastViewedRectangle = new cool.SimpleRectangle(0, 0, 0, 0);
		this._viewSize = new cool.SimplePoint(0, 0);
		this._baseViewSize = new cool.SimplePoint(0, 0);
		this._documentAnchorPosition = [0, 0];

		// The stacked-page subclasses (MultiPage/FileBased/CompareChanges) install
		// their own zoomend handler and override onResize to rebuild page geometry
		// and request tiles. The plain single-window layout (Impress/Draw edit) has
		// none, so without these the viewed rectangle (which carries the centering
		// offset) is never rebuilt on load/zoom/resize: the slide would not centre
		// and would go blank after a zoom. rebuildSingleWindowView is guarded by
		// usesSingleWindowView() so the zoomend path is a no-op for the subclasses.
		// Each handler is registered for this layout alone, so dispose removes exactly
		// this instance's subscriptions.
		app.map.on('zoomend', this.rebuildSingleWindowView, this);
		app.events.on('resize', this.boundOnResize);
		app.layoutingService.appendLayoutingTask(() =>
			this.rebuildSingleWindowView(),
		);
	}

	// Drops the subscriptions this layout made, so that only the layout which is
	// currently active rebuilds its geometry, sends a visible area to the server
	// and asks for tiles on a resize or a zoom. A subclass that subscribes to
	// more events overrides this and calls it.
	public dispose(): void {
		app.map.off('zoomend', this.rebuildSingleWindowView, this);
		app.events.off('resize', this.boundOnResize);
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

	// The message text for the current visible area, in whole twips.
	protected clientVisibleAreaMessage(): string {
		const rectangle = this.viewedRectangle;

		return (
			'clientvisiblearea x=' +
			Math.round(rectangle.x1) +
			' y=' +
			Math.round(rectangle.y1) +
			' width=' +
			Math.round(rectangle.width) +
			' height=' +
			Math.round(rectangle.height)
		);
	}

	// Layout work that belongs with a new visible area, run just before the
	// message goes out.
	protected onVisibleAreaChanged(): void {
		return;
	}

	// Sends the visible area to the server when it differs from the one last
	// sent. forceUpdate sends an unchanged area again.
	public sendClientVisibleArea(forceUpdate: boolean = false): void {
		if (!app.map._docLoaded) return;
		// A zoom animation drives the viewed rectangle through intermediate frame
		// values; the settled area goes out when the animation ends.
		if (app.sectionContainer.isInZoomAnimation()) return;

		const visibleAreaCommand = this.clientVisibleAreaMessage();
		if (visibleAreaCommand === this.clientVisibleAreaCommand && !forceUpdate)
			return;

		this.onVisibleAreaChanged();
		app.socket.sendMessage(visibleAreaCommand);

		if (app.map.contextToolbar) app.map.contextToolbar.hideContextToolbar(); // hide context toolbar when scroll/window resize etc...

		// The command counts as sent only while the connection is healthy, so the
		// area goes out again after a drop.
		if (!app.map._fatal && app.idleHandler._active && app.socket.connected())
			this.clientVisibleAreaCommand = visibleAreaCommand;
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

	// The scrollable extent WITHOUT comment overflow. Single-window layouts
	// (Writer/Impress/Draw edit) use the file size directly; stacked-page layouts
	// override with the layout extent they captured. Used as the stable base by
	// ensureViewSizeCoversComments and by side-space / page-positioning math, so a
	// comment-inflated viewSize never feeds back into them.
	protected getBaseViewSize(): cool.SimplePoint {
		if (app.activeDocument) return app.activeDocument.fileSize;
		else return new cool.SimplePoint(0, 0);
	}

	// Grow the scrollable area so the comments stay reachable, WITHOUT letting
	// callers assign viewSize directly (that mutation fed back into
	// calculateAvailableSpace and made the page drift). extraWidth (twips) is the
	// extra column width when a comment does not fit (0 otherwise); bottomY (twips)
	// is the absolute Y the lowest comment reaches. viewSize becomes a pure
	// function of the base extent plus these.
	public ensureViewSizeCoversComments(
		extraWidth: number,
		bottomY: number,
	): void {
		const base = this.getBaseViewSize();
		if (!base) return;

		// Tolerate missing/invalid inputs (e.g. no comment laid out yet) so a stray
		// NaN never poisons viewSize and blanks the tiles.
		const x = base.x + Math.max(0, extraWidth || 0);
		const y = Math.max(base.y, bottomY || 0);

		// Compare on the rounded values SimplePoint actually stores; otherwise a
		// fractional x/y never equals the rounded stored value and this reassigns
		// (churns a new object) every frame.
		if (
			this._viewSize.x === Math.round(x) &&
			this._viewSize.y === Math.round(y)
		)
			return;

		// Only the stored size changes here; the redraw comes with the request
		// that follows, which keeps the comment layout to a single pass.
		this._viewSize = new cool.SimplePoint(x, y);
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

	// The visible viewport in document core-pixel coordinates: the pure scroll
	// offset plus the frame size, with no centring baked in. Unlike
	// viewedRectangle (which the stacked-page layouts build in document-union
	// space, and which single-window layouts start at a negative centred origin)
	// this is the real viewport for every layout, so tile fetching and overlay
	// bounds read it uniformly instead of the leaflet map pixel bounds.
	public getViewportCorePixelBounds(): cool.Bounds {
		const frame = this.frameSize;
		const x = this.scrollProperties.viewX;
		const y = this.scrollProperties.viewY;
		return new cool.Bounds(
			new cool.Point(x, y),
			new cool.Point(x + frame.pX, y + frame.pY),
		);
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

	// The single-window layouts that place one page/slide with the inherited view
	// machinery (centred when smaller than the viewport): Writer and Impress/Draw
	// edit (ViewLayoutImpress) override this to true. Calc (fills the viewport, own
	// scroll/viewed rectangle) and the stacked-page subclasses keep the default.
	protected usesSingleWindowView(): boolean {
		return false;
	}

	// Single resize handler, bound to app.events 'resize' in the constructor and
	// dispatched polymorphically. First resize the canvas / document-anchor
	// section (the doc layer's _syncTileContainerSize; Calc's override is
	// self-complete and rebuilds its own viewed rectangle). Then, for single-window
	// layouts, rebuild the centred viewed rectangle - deferred to the layouting
	// phase so it reads the settled frame. Derived classes override to add their
	// own work (stacked-page reset, Writer comment margin).
	public onResize(): void {
		app.map._docLayer._syncTileContainerSize();
		if (this.usesSingleWindowView())
			app.layoutingService.appendLayoutingTask(() =>
				this.rebuildSingleWindowView(),
			);
	}

	private rebuildSingleWindowView(): void {
		if (!this.usesSingleWindowView()) return; // subclasses handle their own

		// applyZoom rebuilds the viewed rectangle around the zoom anchor
		// (setViewRectangleFromPointAndScale) which may leave a non-negative scroll
		// offset (when zoomed in) or the negative centering origin (when it fits).
		// Fold only the real (non-negative) scroll back into scrollProperties, then
		// let refreshVisibleAreaRectangle re-derive and re-centre the rectangle for
		// the current frame/zoom and request the tiles.
		// The scroll stays inside the range the document itself spans,
		// [0, base view size - frame]. The base size leaves out the comment
		// overflow, so a document narrower than the frame keeps its centred
		// placement and a comment column that does not fit hangs off the side.
		// Plain scrolling still reaches the comment column through the full view
		// size.
		const base = this.getBaseViewSize();
		const frame = this.frameSize;
		const maxX = Math.max(0, base.pX - frame.pX);
		const maxY = Math.max(0, base.pY - frame.pY);

		this.scrollProperties.viewX = Math.min(
			maxX,
			Math.max(0, this._viewedRectangle.pX1),
		);
		this.scrollProperties.viewY = Math.min(
			maxY,
			Math.max(0, this._viewedRectangle.pY1),
		);
		this.updateViewData();
	}

	// Centre content (a single slide/page) that is smaller than the viewport.
	// Returned in core pixels [x, y]; zero on an axis where the content fills or
	// exceeds the viewport (then normal scrolling applies).
	protected getCenteringOffset(): number[] {
		const frame = this.frameSize; // Viewport, core pixels.
		const content = this._viewSize; // Slide/page size; pX tracks app.twipsToPixels.

		// Before the first status the content size is not known yet - no centering.
		if (content.pX <= 0 || content.pY <= 0) return [0, 0];

		return [
			Math.max(0, Math.round((frame.pX - content.pX) / 2)),
			Math.max(0, Math.round((frame.pY - content.pY) / 2)),
		];
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
	// state and switches the tiles section into zoom-frame drawing. The
	// animation is then driven frame by frame by ZoomControl through the
	// section container's animation loop - NOT the tile painter's own
	// requestAnimationFrame. The scale for each frame comes from stepZoom.
	public beginZoom(anchorTwips: cool.SimplePoint): void {
		const docLayer: any = app.map._docLayer; // _painter is not on the typed interface.
		const painter = docLayer._painter;

		// The anchor is already in core pixels (SimplePoint.pX/pY) - no intern/CRS
		// round-trip needed. This is both the pinch pivot and the frame centre.
		const centerCorePx = new cool.Point(anchorTwips.pX, anchorTwips.pY);
		docLayer._pinchStartCenter = centerCorePx;
		painter._newCenter = centerCorePx;
		painter._offset = new cool.Point(0, 0);

		// Snapshot the starting view bounds before _inZoomAnim is set, so
		// _paintContext reads the live bounds here. Zoom frames drive the
		// viewed rectangle to intermediate values; the final-center computation
		// reads this stable snapshot instead.
		painter._zoomStartViewBounds = painter._paintContext().viewBounds;
		// The base twips->px each zoom frame scales from (see
		// TilesSection.drawZoomFrame) so vector sections scale with the tiles.
		painter._zoomBaseTwipsToPixels = app.twipsToPixels;

		// Hide the cursor and selection handles for the duration of the animation.
		if (docLayer._cursorMarker && app.file.textCursor.visible)
			docLayer._cursorMarker.setOpacity(0);
		const textInput: any = app.map._textInput;
		if (textInput._cursorHandler) textInput._cursorHandler.setOpacity(0);
		TextSelections.hideHandles();
		TextCursorSection.updateVisibilities();

		painter._inZoomAnim = true;

		// Publish the scale the tiles section uses for the zoom frame: the base
		// twips->px it started from and the effective (animated) one.
		// drawZoomFrame derives its bitmap ratio from these; stepZoom updates
		// the effective value each frame.
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

		// Restore the cursor and selection handles hidden for the animation.
		if (app.file.textCursor.visible) docLayer._cursorMarker.setOpacity(1);
		const textInput: any = app.map._textInput;
		if (textInput._cursorHandler) textInput._cursorHandler.setOpacity(1);
		TextSelections.showHandles();

		if (docLayer._annotations && docLayer._annotations.update) {
			const annotations = docLayer._annotations;
			setTimeout(function () {
				annotations.update();
			}, 250 /* ms */);
		}
	}

	// Commit the final integral zoom: set the new scale, rebuild the viewed
	// rectangle around anchorTwips at that scale, and refresh tiles/headers.
	// Used by both the animated end and the non-animated path. Map-free: the
	// scale is applied via _updateTileTwips (not _resetView) and the doc type's
	// 'zoomend' listeners do the rest of the post-zoom work.
	public applyZoom(targetZoom: number, anchorTwips?: cool.SimplePoint): void {
		// The zoom internals used here (_zoom, _clientZoom, _tileZoom,
		// _updateTileTwips) are not on the typed interfaces; cast for now while
		// the map is still being removed from the zoom path.
		const map: any = app.map;
		const docLayer: any = map._docLayer;
		const zoom = map._limitZoom(targetZoom);
		const zoomChanged = map._zoom !== zoom;

		map._zoom = zoom;
		map._clientZoom = zoom; // kept in sync as the initial-zoom fallback.
		docLayer._tileZoom = Math.round(zoom);

		// Recompute app.twipsToPixels for the new zoom (no map pane involved) and
		// re-sync the canvas, then place the viewed rectangle at the new scale.
		docLayer._updateTileTwips();
		if (anchorTwips) {
			// Explicit pivot (pinch / wheel / ZoomControl programmatic): keep that
			// document point fixed under the zoom. The anchor is captured before
			// this call so _updateTileTwips' rebuild does not move it.
			this.setViewRectangleFromPointAndScale(anchorTwips, app.twipsToPixels);
			// The rectangle above is a plain scroll position. The single-window
			// layouts hold their horizontal placement - the centring of a document
			// narrower than the frame, and Writer's shift for the comment margin - in
			// the centring offset instead, so take the scroll from that rectangle and
			// build the final one around it.
			if (this.usesSingleWindowView()) this.rebuildSingleWindowView();
		} else if (this.usesSingleWindowView()) {
			// No pivot (load / fit-width / programmatic): keep the current scroll
			// position by rebuilding from scrollProperties at the new scale, so a
			// fit-width Writer or Impress view stays where it is, at the document
			// top on load.
			this.refreshVisibleAreaRectangle();
		} else {
			this.setViewRectangleFromPointAndScale(
				this.zoomAnchorPoint(),
				app.twipsToPixels,
			);
		}

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
		this.scrollProperties.horizontalScrollLength = documentAnchor.size[0];
		this.scrollProperties.verticalScrollLength = documentAnchor.size[1];

		// This layout never restricts scrolling to less than the whole document
		// anchor, so the drawn railway matches the scrollable length exactly.
		this.scrollProperties.horizontalScrollRailwayOffset =
			this.scrollProperties.xOffset;
		this.scrollProperties.horizontalScrollRailwayLength =
			this.scrollProperties.horizontalScrollLength;
		this.scrollProperties.verticalScrollRailwayOffset =
			this.scrollProperties.yOffset;
		this.scrollProperties.verticalScrollRailwayLength =
			this.scrollProperties.verticalScrollLength;

		// Sizes of the scroll bars.
		this.calculateTheScrollSizes();

		// The ratio maps scrollbar-track pixels to view-space pixels: scroll()
		// takes view-space deltas, so a pointer delta on the track is scaled by
		// this. scroll() places the bar at viewX / viewSize * trackLength, so the
		// pointer-to-view factor is the inverse of that.
		this.scrollProperties.horizontalScrollRatio =
			this.scrollProperties.horizontalScrollLength > 0 && this.viewSize.pX > 0
				? this.viewSize.pX / this.scrollProperties.horizontalScrollLength
				: 1;
		this.scrollProperties.verticalScrollRatio =
			this.scrollProperties.verticalScrollLength > 0 && this.viewSize.pY > 0
				? this.viewSize.pY / this.scrollProperties.verticalScrollLength
				: 1;

		// Properties for quick scrolling.
		this.scrollProperties.verticalScrollStep = documentAnchor.size[1] / 2;
		this.scrollProperties.horizontalScrollStep = documentAnchor.size[0] / 2;
	}

	protected refreshCurrentCoordList() {
		this.currentCoordList.length = 0;
		const zoom = Math.round(app.map.getZoom());
		const tileSize = RenderManager.tileSize;

		// The coordinates name tiles of the part the view currently shows:
		// part 0 for a text document, the selected sheet for a spreadsheet,
		// and the shown page's unique id for a presentation or drawing.
		const part = app.map._docLayer.getSelectedPart();

		const r = this._viewedRectangle;

		const startCol = Math.floor(r.pX1 / tileSize);
		const startRow = Math.floor(r.pY1 / tileSize);
		const endCol = Math.floor((r.pX1 + r.pWidth - 1) / tileSize);
		const endRow = Math.floor((r.pY1 + r.pHeight - 1) / tileSize);

		this.pushTileGrid(
			startCol * tileSize,
			startRow * tileSize,
			endCol - startCol,
			endRow - startRow,
			zoom,
			tileSize,
			part,
			new Set<string>(),
		);
	}

	// Shared visible-area computation for the stacked-page layouts
	// (ViewLayoutMultiPage, ViewLayoutFileBased). Builds the bounding document
	// rectangle of every part whose view rectangle intersects the viewport.
	// When no part is visible (e.g. before reset finishes) the view is snapped
	// back to the start of `snapAxis` and the computation retried.
	protected refreshVisibleAreaRectangleImpl(
		documentRectangles: cool.SimpleRectangle[],
		viewRectangles: cool.SimpleRectangle[],
		snapAxis: 'x' | 'y',
	): void {
		const documentAnchor = this.getDocumentAnchorSection();

		// When the document container is hidden (e.g. BackstageView in CODA), the
		// anchor section has zero size - bail out to avoid an infinite retry loop.
		if (documentAnchor.size[0] <= 0 || documentAnchor.size[1] <= 0) return;

		const view = cool.SimpleRectangle.fromCorePixels([
			this.scrollProperties.viewX,
			this.scrollProperties.viewY,
			documentAnchor.size[0],
			documentAnchor.size[1],
		]);

		const resultingRectangle: cool.SimpleRectangle = new cool.SimpleRectangle(
			Number.POSITIVE_INFINITY,
			Number.POSITIVE_INFINITY,
			-10000,
			-10000,
		);

		for (let i = 0; i < documentRectangles.length; i++) {
			const documentRectangle = documentRectangles[i];
			const viewRectangle = viewRectangles[i];

			if (view.intersectsRectangle(viewRectangle.toArray())) {
				if (resultingRectangle.pX1 > documentRectangle.pX1)
					resultingRectangle.pX1 = documentRectangle.pX1;
				if (resultingRectangle.pY1 > documentRectangle.pY1)
					resultingRectangle.pY1 = documentRectangle.pY1;
				if (resultingRectangle.pX2 < documentRectangle.pX2)
					resultingRectangle.pX2 = documentRectangle.pX2;
				if (resultingRectangle.pY2 < documentRectangle.pY2)
					resultingRectangle.pY2 = documentRectangle.pY2;
			}
		}

		if (
			resultingRectangle.pX1 === Number.POSITIVE_INFINITY ||
			resultingRectangle.pY1 === Number.POSITIVE_INFINITY
		) {
			if (documentRectangles.length === 0) {
				// No layout yet (transient, e.g. before a part switch finishes): snap
				// the scroll back to the start of snapAxis and retry once the layout
				// exists.
				app.layoutingService.appendLayoutingTask(() => {
					if (snapAxis === 'x') this.scrollProperties.viewX = 0;
					else this.scrollProperties.viewY = 0;
					this.refreshVisibleAreaRectangle();
				});
				return;
			}

			// Pages exist but none intersect the viewport: the view is scrolled into
			// the region the comment overflow made scrollable beyond the last page.
			// The viewed rectangle is clamped to the nearest page, so its tiles stay
			// valid while the scroll position is kept and the bottom comments there
			// stay reachable; the comment section draws the comments. The scroll is
			// not snapped back because the snap axis need not match the overflowing
			// axis (MultiPage snaps X while the comment overflow is vertical).
			let nearest = 0;
			let bestDistance = Number.POSITIVE_INFINITY;
			for (let i = 0; i < viewRectangles.length; i++) {
				const vr = viewRectangles[i];
				const dx = Math.max(vr.pX1 - view.pX2, view.pX1 - vr.pX2, 0);
				const dy = Math.max(vr.pY1 - view.pY2, view.pY1 - vr.pY2, 0);
				const distance = dx * dx + dy * dy;
				if (distance < bestDistance) {
					bestDistance = distance;
					nearest = i;
				}
			}

			this._viewedRectangle = documentRectangles[nearest].clone();

			app.sectionContainer.onNewDocumentTopLeft();
			app.sectionContainer.requestReDraw();
		} else {
			this._viewedRectangle = resultingRectangle;

			app.sectionContainer.onNewDocumentTopLeft();
			app.sectionContainer.requestReDraw();
		}
	}

	// Default single-window viewed rectangle: the frame positioned at the
	// current scroll offset (scrollProperties.viewX/Y), in twips. Stacked-page
	// layouts (MultiPage/FileBased/CompareChanges) override to compute it from
	// page geometry via refreshVisibleAreaRectangleImpl.
	protected refreshVisibleAreaRectangle(): void {
		const documentAnchor = this.getDocumentAnchorSection();
		if (!documentAnchor) return;

		// Centre content smaller than the viewport by starting the viewed
		// rectangle at a negative document offset (the gray margin to the left/top
		// of the slide). The whole pipeline - drawing (documentToViewX/Y), mouse
		// hit-testing (MouseControl), tile requests and the zoom anchor - reads
		// viewedRectangle, so centring stays consistent from this single source.
		// The scroll offset (scrollProperties.viewX/Y) stays a pure, non-negative
		// scroll position; the centring is applied on top of it here. This mirrors
		// the negative-origin viewed rectangle used by ViewLayoutCompareChanges.
		const centering = this.getCenteringOffset();

		this._viewedRectangle = cool.SimpleRectangle.fromCorePixels([
			this.scrollProperties.viewX - centering[0],
			this.scrollProperties.viewY - centering[1],
			documentAnchor.size[0],
			documentAnchor.size[1],
		]);

		app.sectionContainer.onNewDocumentTopLeft();
		app.sectionContainer.requestReDraw();
	}

	// Default commit used by scroll(): rebuild the viewed rectangle and fetch
	// tiles. Stacked-page layouts override with their own (viewSize recompute
	// etc.).
	protected updateViewData(): void {
		this.commitVisibleAreaAndRequestTiles();
	}

	// Recompute the visible area, push it to the server and request the tiles
	// needed to render it.
	protected commitVisibleAreaAndRequestTiles(): void {
		this.refreshVisibleAreaRectangle();

		if (app.map._docLayer?._cursorMarker)
			app.map._docLayer._cursorMarker.update();

		app.map._docLayer._sendClientZoom();
		this.sendClientVisibleArea();

		this.refreshCurrentCoordList();
		// requestVisibleTiles refreshes every tile's distance from the view
		// before asking for tiles, so a tile that has just moved off screen
		// stops counting as pending and coherency-paused drawing resumes as
		// soon as the visible tiles are ready.
		RenderManager.requestVisibleTiles(this.currentCoordList);
	}

	// Reset currentCoordList and return the per-frame constants shared by the
	// tile-queue builders: rounded zoom, tile size and the viewport rectangle.
	protected beginCoordList(): {
		zoom: number;
		tileSize: number;
		view: cool.SimpleRectangle;
	} {
		this.currentCoordList.length = 0;
		const zoom = Math.round(app.map.getZoom());
		const tileSize = RenderManager.tileSize;

		const documentAnchor = this.getDocumentAnchorSection();
		const view = cool.SimpleRectangle.fromCorePixels([
			this.scrollProperties.viewX,
			this.scrollProperties.viewY,
			documentAnchor.size[0],
			documentAnchor.size[1],
		]);

		return { zoom, tileSize, view };
	}

	// Visible portion of one part's view rectangle, clipped to the viewport,
	// in view coordinates.
	protected getVisibleViewBounds(
		view: cool.SimpleRectangle,
		viewRect: cool.SimpleRectangle,
	): { vx1: number; vy1: number; vx2: number; vy2: number } {
		return {
			vx1: Math.max(view.pX1, viewRect.pX1),
			vy1: Math.max(view.pY1, viewRect.pY1),
			vx2: Math.min(view.pX1 + view.pWidth, viewRect.pX1 + viewRect.pWidth),
			vy2: Math.min(view.pY1 + view.pHeight, viewRect.pY1 + viewRect.pHeight),
		};
	}

	// Enqueue every tile of one part's grid into currentCoordList, skipping
	// duplicates (tracked in `added`) and invalid tiles.
	protected pushTileGrid(
		startX: number,
		startY: number,
		columnCount: number,
		rowCount: number,
		zoom: number,
		tileSize: number,
		part: PartNumber,
		added: Set<string>,
	): void {
		for (let c = 0; c <= columnCount; c++) {
			for (let r = 0; r <= rowCount; r++) {
				const coords = new TileCoordData(
					startX + c * tileSize,
					startY + r * tileSize,
					zoom,
					part,
					0,
				);

				const key = coords.key();
				if (added.has(key)) continue;
				added.add(key);

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

	public unselectCommentOnScroll() {
		const commentSection = app.sectionContainer.getSectionWithName(
			app.CSections.CommentList.name,
		) as cool.CommentSection;

		if (commentSection && commentSection.sectionProperties.selectedComment) {
			commentSection.unselect();
		}
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
	): boolean {
		if (userIsScrolling) this.unselectCommentOnScroll();
		this.refreshScrollProperties();
		const documentAnchor = this.getDocumentAnchorSection();
		let scrolled = false;

		if (pX !== 0 && this.canScrollHorizontal(documentAnchor)) {
			const max = Math.max(0, this._viewSize.pX - documentAnchor.size[0]);
			const newViewX = Math.max(
				0,
				Math.min(this.scrollProperties.viewX + pX, max),
			);
			if (newViewX !== this.scrollProperties.viewX) {
				this.scrollProperties.viewX = newViewX;
				this.scrollProperties.startX = Math.round(
					(this.scrollProperties.viewX / this._viewSize.pX) *
						this.scrollProperties.horizontalScrollLength,
				);
				scrolled = true;
			}
		}

		if (pY !== 0 && this.canScrollVertical(documentAnchor)) {
			const max = Math.max(0, this._viewSize.pY - documentAnchor.size[1]);
			const newViewY = Math.max(
				0,
				Math.min(this.scrollProperties.viewY + pY, max),
			);
			if (newViewY !== this.scrollProperties.viewY) {
				this.scrollProperties.viewY = newViewY;
				this.scrollProperties.startY = Math.round(
					(this.scrollProperties.viewY / this._viewSize.pY) *
						this.scrollProperties.verticalScrollLength,
				);
				scrolled = true;
			}
		}

		if (scrolled) {
			this.updateViewData();
			app.sectionContainer.requestReDraw();
		}

		return scrolled;
	}

	public scrollTo(
		pX: number,
		pY: number,
		userIsScrolling: boolean = false,
	): void {
		this.refreshScrollProperties();

		pX -= this.viewedRectangle.pX1;
		pY -= this.viewedRectangle.pY1;

		this.scroll(pX, pY, userIsScrolling);
	}

	public setOverviewPageVisArea(point: cool.SimplePoint): void {
		this.scrollTo(point.pX, point.pY);
	}
}
