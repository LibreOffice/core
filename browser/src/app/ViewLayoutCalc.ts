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

class ViewLayoutCalc extends ViewLayoutNewBase {
	public readonly type: string = 'ViewLayoutCalc';

	constructor() {
		super();
	}

	private isRTL(): boolean {
		const docLayer = app.map._docLayer;
		return !!(docLayer && docLayer.isCalcRTL && docLayer.isCalcRTL());
	}

	private splitAwareOffset(
		coord: number,
		viewedStart: number,
		split: number,
	): number {
		if (coord < split) return coord;
		const scrolled = coord - viewedStart;
		if (scrolled < split) return split;
		return scrolled;
	}

	// In RTL Calc the document origin (column A) sits at the right edge of
	// the document area, so a point's view X must be mirrored across the
	// document anchor's horizontal extent. LTR matches the base class.
	public override documentToViewX(point: cool.SimplePoint): number {
		const splitX = app.calc.splitCoordinate ? app.calc.splitCoordinate.pX : 0;
		const offset = this.splitAwareOffset(
			point.pX,
			this._viewedRectangle.pX1,
			splitX,
		);

		if (this.isRTL()) {
			return (
				this._documentAnchorPosition[0] +
				this.getDocumentAnchorSection().size[0] -
				offset
			);
		}
		return offset + this._documentAnchorPosition[0];
	}

	public override documentToViewY(point: cool.SimplePoint): number {
		const splitY = app.calc.splitCoordinate ? app.calc.splitCoordinate.pY : 0;
		const offset = this.splitAwareOffset(
			point.pY,
			this._viewedRectangle.pY1,
			splitY,
		);
		return offset + this._documentAnchorPosition[1];
	}

	public override canvasToDocumentPoint(
		point: cool.SimplePoint,
	): cool.SimplePoint {
		const result = point.clone();

		if (this.isRTL()) {
			result.pX =
				this._documentAnchorPosition[0] +
				this.getDocumentAnchorSection().size[0] +
				this._viewedRectangle.pX1 -
				point.pX;
		} else {
			result.pX =
				point.pX + this._viewedRectangle.pX1 - this._documentAnchorPosition[0];
		}
		result.pY =
			point.pY + this._viewedRectangle.pY1 - this._documentAnchorPosition[1];

		return result;
	}

	// CalcTileLayer assigns viewSize from fileSize when the document size is
	// known, and re-assigns when the document grows. ViewLayoutNewBase
	// disables the setter; re-enable it for Calc.
	public override get viewSize() {
		return this._viewSize;
	}
	public override set viewSize(size: cool.SimplePoint) {
		this._viewSize = size;
	}

	// Scroll drives the viewedRectangle update from inside the layout, but
	// zoom and the initial document load still come in via map events, so the
	// setter must stay writable from outside.
	public override get viewedRectangle() {
		return this._viewedRectangle;
	}
	public override set viewedRectangle(rectangle: cool.SimpleRectangle) {
		if (!this._viewedRectangle.equals(rectangle.toArray()))
			this.lastViewedRectangle = this._viewedRectangle.clone();

		this._viewedRectangle = rectangle;

		app.sectionContainer.onNewDocumentTopLeft();
		app.sectionContainer.requestReDraw();
	}

	// Push the current viewed rectangle into the sheet geometry's view limits,
	// which feed the in-view row and column range and group collection. The
	// rectangle is authoritative even mid-scroll, before the map catches up.
	public override refreshViewArea(): void {
		const docLayer = app.map && app.map._docLayer;
		if (!docLayer || !docLayer.sheetGeometry) return;

		const rectangle = this._viewedRectangle;
		docLayer.sheetGeometry.setViewArea(
			new cool.Point(rectangle.x1, rectangle.y1),
			new cool.Point(rectangle.width, rectangle.height),
		);
	}

	// The viewed rectangle already tracks the visible area (in twips), so it is
	// read directly rather than through the map pixel bounds. The bounds stay
	// correct while scrolling, with no map pane involved.
	protected override getVisibleAreaBounds(): any {
		const r = this._viewedRectangle;
		return new cool.Bounds(
			new cool.Point(r.x1, r.y1),
			new cool.Point(r.x2, r.y2),
		);
	}

	// Calc needs splitx/splity so the server renders frozen/split panes
	// correctly, plus the splitter onPositionChange notifications, the
	// context-toolbar hide, the cache suppression, and the forceUpdate flag.
	public override sendClientVisibleArea(forceUpdate: boolean = false) {
		if (!app.map._docLoaded) return;

		var splitPos = app.map._docLayer._splitPanesContext
			? app.map._docLayer._splitPanesContext.getSplitPos()
			: new cool.Point(0, 0);

		const visibleArea = this.getVisibleAreaBounds();

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
			if (app.map._docLayer._ySplitter) {
				app.map._docLayer._ySplitter.onPositionChange();
			}
			if (app.map._docLayer._xSplitter) {
				app.map._docLayer._xSplitter.onPositionChange();
			}
			app.socket.sendMessage(newClientVisibleAreaCommand);
			if (app.map.contextToolbar) app.map.contextToolbar.hideContextToolbar();
			if (!app.map._fatal && app.idleHandler._active && app.socket.connected())
				this.clientVisibleAreaCommand = newClientVisibleAreaCommand;
		}
	}

	public override refreshScrollProperties(): any {
		const documentAnchor = this.getDocumentAnchorSection();
		const scrollProps = this.scrollProperties;

		// The railway is drawn across the whole document anchor, so a frozen
		// row or column never leaves a gap in it.
		scrollProps.horizontalScrollRailwayOffset = documentAnchor.myTopLeft[0];
		scrollProps.horizontalScrollRailwayLength =
			documentAnchor.size[0] - scrollProps.horizontalScrollRightOffset;
		scrollProps.verticalScrollRailwayOffset = documentAnchor.myTopLeft[1];
		scrollProps.verticalScrollRailwayLength = documentAnchor.size[1];

		// The thumb only travels across the scrollable part, past any
		// frozen/split panes, since that part of the view never moves.
		scrollProps.xOffset = documentAnchor.myTopLeft[0];
		scrollProps.yOffset = documentAnchor.myTopLeft[1];

		const splitPanesContext: any = app.map.getSplitPanesContext();
		let splitX = 0;
		let splitY = 0;
		if (splitPanesContext) {
			const splitPos = splitPanesContext.getSplitPos();
			splitX = Math.round(splitPos.x * app.dpiScale);
			splitY = Math.round(splitPos.y * app.dpiScale);
		}
		scrollProps.xOffset += splitX;
		scrollProps.yOffset += splitY;

		scrollProps.horizontalScrollLength =
			documentAnchor.size[0] - splitX - scrollProps.horizontalScrollRightOffset;
		scrollProps.verticalScrollLength = documentAnchor.size[1] - splitY;

		this.calculateTheScrollSizes();

		// The ratio maps scrollbar-track pixels to view-space pixels.
		// ScrollSection.quickScroll* multiplies its pointer delta by this
		// ratio before calling scroll(), so scroll() always receives
		// view-space deltas regardless of entry point (wheel, drag, keys).
		scrollProps.horizontalScrollRatio = ViewLayoutCalc.trackToViewRatio(
			this.viewSize.pX,
			documentAnchor.size[0],
			scrollProps.horizontalScrollLength,
			scrollProps.horizontalScrollSize,
		);
		scrollProps.verticalScrollRatio = ViewLayoutCalc.trackToViewRatio(
			this.viewSize.pY,
			documentAnchor.size[1],
			scrollProps.verticalScrollLength,
			scrollProps.verticalScrollSize,
		);

		// Project the current view onto the scrollbar track.
		scrollProps.startX =
			scrollProps.xOffset +
			ViewLayoutCalc.viewToTrack(
				this._viewedRectangle.pX1,
				this.viewSize.pX,
				documentAnchor.size[0],
				scrollProps.horizontalScrollLength,
				scrollProps.horizontalScrollSize,
			);
		scrollProps.startY =
			scrollProps.yOffset +
			ViewLayoutCalc.viewToTrack(
				this._viewedRectangle.pY1,
				this.viewSize.pY,
				documentAnchor.size[1],
				scrollProps.verticalScrollLength,
				scrollProps.verticalScrollSize,
			);

		scrollProps.verticalScrollStep = documentAnchor.size[1] / 2;
		scrollProps.horizontalScrollStep = documentAnchor.size[0] / 2;
	}

	private static trackToViewRatio(
		viewSize: number,
		anchorSize: number,
		trackLen: number,
		trackSize: number,
	): number {
		const num = viewSize - anchorSize;
		const den = trackLen - trackSize;
		return den > 0 ? num / den : 1;
	}

	private static viewToTrack(
		viewPos: number,
		viewSize: number,
		anchorSize: number,
		trackLen: number,
		trackSize: number,
	): number {
		const maxView = Math.max(0, viewSize - anchorSize);
		const trackMax = Math.max(0, trackLen - trackSize);
		if (maxView <= 0 || trackMax <= 0) return 0;
		return (viewPos / maxView) * trackMax;
	}

	// pX, pY are view-space deltas in canvas (core) pixels.
	public override scroll(pX: number, pY: number): any {
		const documentAnchor = this.getDocumentAnchorSection();

		if (this.isRTL()) pX = -pX;

		const prevX = this._viewedRectangle.pX1;
		const prevY = this._viewedRectangle.pY1;
		let newX = prevX;
		let newY = prevY;

		if (pX !== 0 && this.canScrollHorizontal(documentAnchor)) {
			const maxView = Math.max(0, this.viewSize.pX - documentAnchor.size[0]);
			newX = Math.max(0, Math.min(maxView, prevX + pX));
		}

		if (pY !== 0 && this.canScrollVertical(documentAnchor)) {
			const maxView = Math.max(0, this.viewSize.pY - documentAnchor.size[1]);
			newY = Math.max(0, Math.min(maxView, prevY + pY));
		}

		if (newX === prevX && newY === prevY) return;

		// Update the viewed rectangle internally; the setter refreshes the
		// section container and remembers the previous rectangle for pan
		// direction pre-fetch.
		this.viewedRectangle = cool.SimpleRectangle.fromCorePixels([
			newX,
			newY,
			this._viewedRectangle.pWidth,
			this._viewedRectangle.pHeight,
		]);

		// Row/column headers recompute their visible entries from the viewed
		// rectangle. They used to do this off the leaflet map 'move' event;
		// drive it from the layout so scrolling stays correct without the map.
		this.refreshHeaders();

		// The blinking DOM text cursor (only attached while editing a cell)
		// repositions itself from the viewed rectangle. It also used the map
		// 'move' event; drive it from the layout instead.
		this.refreshTextCursor();

		this.sendClientVisibleArea();

		// Request the tiles that cover the new visible area and mark the
		// already-loaded ones current.
		this.refreshTiles();
	}

	// Refresh the visible-tile list from the viewed rectangle and hand it to
	// the tile manager, which fetches the missing tiles and marks the loaded
	// ones current. Public so the tile manager can drive it from update() for
	// invalidation-triggered refetches (see BitmapTileManager.update).
	public refreshTiles(): void {
		this.refreshCurrentCoordList();
		RenderManager.requestVisibleTiles(this.currentCoordList);
	}

	// pX, pY are absolute view-space positions in canvas (core) pixels.
	public override scrollTo(pX: number, pY: number): void {
		const deltaX = pX - this._viewedRectangle.pX1;
		const deltaY = pY - this._viewedRectangle.pY1;
		if (deltaX !== 0 || deltaY !== 0) this.scroll(deltaX, deltaY);
	}

	// Rebuild the viewed rectangle for a new zoom: centre it on the given
	// document-space point and size it to the current frame at the given scale.
	// point is in twips, scale is the twips-to-core-pixel factor for the new
	// zoom. Map._resetView calls this so the zoom position is driven by the
	// layout (from the canvas frame) instead of the leaflet map element.
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

	// ---- Zoom (driven by ZoomControl) ----------------------------------
	// These replace the map's role in zooming. The controller calls them; the
	// data (scale, viewed rectangle, tile requests) lives here so the map is
	// not involved. Implemented incrementally: P1b fills applyZoom (the
	// non-animated commit), P2 fills begin/step/endZoom (the animation).

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
	// own requestAnimationFrame, which never starts for Calc zoom. The scale for
	// each frame comes from stepZoom.
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
	// TilesSection.drawZoomFrame reads it while the container redraws this frame
	// (and updates app.twipsToPixels + the intermediate viewed rectangle there).
	public stepZoom(scale: number): void {
		// The effective twips->px the tiles section renders this frame at. All
		// Calc zoom-frame consumers (tiles, grid, headers, overlay) read the
		// ratio from this via tsManager.zoomFrameScale(); the painter's
		// _zoomFrameScale is no longer used for Calc.
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
	// Used by both the animated end (P2) and the non-animated path (P1b).
	// Map-free: the scale is applied via _updateTileTwips (not _resetView) and
	// the existing 'zoomend' listeners do the rest of the post-zoom work
	// (client zoom, document-size restrict, view data/tiles, status-bar %).
	public applyZoom(targetZoom: number, anchorTwips: cool.SimplePoint): void {
		// The zoom internals used here (_zoom, _clientZoom, _tileZoom,
		// _updateTileTwips) are not on the typed interfaces; cast for now while
		// the map is still being removed from the zoom path.
		const map: any = app.map;
		const docLayer: any = map._docLayer;
		const zoom = map._limitZoom(targetZoom);

		map._zoom = zoom;
		map._clientZoom = zoom; // fallback used when setZoom(0) is called later.
		docLayer._tileZoom = Math.round(zoom);

		// Recompute app.twipsToPixels for the new zoom (no map pane involved) and
		// re-sync the canvas/spacers, then rebuild the viewed rectangle around the
		// anchor at that scale.
		docLayer._updateTileTwips();
		this.setViewRectangleFromPointAndScale(anchorTwips, app.twipsToPixels);

		// Reuse the proven post-zoom chain: _onZoomRowColumns (client zoom,
		// _restrictDocumentSize, refreshViewData/tiles), the status-bar
		// percentage (Control.StatusBar.onZoomEnd) and cursor follow all run off
		// 'zoomend'.
		map.fire('zoomend');
	}

	// Recompute the visible entries of the row and column headers after a
	// scroll. _updateCanvas() refreshes each header's HeaderInfo from the
	// current viewed rectangle and requests a redraw; onDraw stays draw-only.
	private refreshHeaders(): void {
		const rowHeader = app.sectionContainer.getSectionWithName(
			app.CSections.RowHeader.name,
		) as any;
		const columnHeader = app.sectionContainer.getSectionWithName(
			app.CSections.ColumnHeader.name,
		) as any;
		if (rowHeader) rowHeader._updateCanvas();
		if (columnHeader) columnHeader._updateCanvas();
	}

	// Reposition the blinking DOM text cursor after a scroll. update() reads
	// the text cursor rectangle and viewed rectangle and handles its own
	// visibility; it is a no-op when no cursor marker is attached.
	private refreshTextCursor(): void {
		const cursorMarker = (app.map as any)?._docLayer?._cursorMarker;
		if (cursorMarker) cursorMarker.update();
	}

	// Build the list of tiles covering the visible area from the viewed
	// rectangle. The base version hard-codes part 0; Calc tiles are per-sheet,
	// so the coordinates must carry the selected part or the visible sheet's
	// tiles are never fetched. The grid covers exactly the visible tiles (no
	// margin): checkRequestTiles() marks every listed tile current
	// (distanceFromView 0), and the coherency-pause resume waits on all such
	// tiles, so the list holds on-screen tiles only.
	protected override refreshCurrentCoordList(): void {
		this.currentCoordList.length = 0;

		const zoom = Math.round(app.map.getZoom());
		const tileSize = RenderManager.tileSize;
		const part = app.map._docLayer.getSelectedPart();
		const r = this._viewedRectangle;

		// With frozen rows/columns the fixed panes show cells at fixed document
		// positions that lie outside the scrolled rectangle, so the list covers
		// every split pane on screen, not just the free pane. getPxBoundList
		// returns the single viewport rectangle when no split is active, and only
		// ever returns visible pane regions, so the list holds on-screen tiles
		// only.
		const docLayer: any = app.map._docLayer;
		const splitPanesContext = docLayer.getSplitPanesContext();
		const viewport = new cool.Bounds(
			new cool.Point(r.pX1, r.pY1),
			new cool.Point(r.pX1 + r.pWidth, r.pY1 + r.pHeight),
		);
		const boundList = splitPanesContext
			? splitPanesContext.getPxBoundList(viewport)
			: [viewport];

		const added = new Set<string>();
		for (const bounds of boundList) {
			const tl = bounds.getTopLeft();
			const br = bounds.getBottomRight();
			const startCol = Math.floor(tl.x / tileSize);
			const startRow = Math.floor(tl.y / tileSize);
			// Tile of the last visible pixel; the difference is the inclusive count
			// pushTileGrid iterates with (<= columnCount / <= rowCount).
			const endCol = Math.floor((br.x - 1) / tileSize);
			const endRow = Math.floor((br.y - 1) / tileSize);

			this.pushTileGrid(
				startCol * tileSize,
				startRow * tileSize,
				endCol - startCol,
				endRow - startRow,
				zoom,
				tileSize,
				part,
				added,
			);
		}
	}
}
