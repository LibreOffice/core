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

class ViewLayoutCalc extends ViewLayoutBase {
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

	// A zoom keeps the cell the user works in on screen: the anchor is the top
	// left corner of the cell cursor, or the middle of a cell selection when
	// there is no cell cursor in view. With neither on screen the middle of the
	// view (the base class anchor) is kept.
	public override zoomAnchorPoint(): cool.SimplePoint {
		const cursor = app.calc.cellCursorRectangle;
		if (cursor && this._viewedRectangle.containsPoint([cursor.x1, cursor.y1]))
			return new cool.SimplePoint(cursor.x1, cursor.y1);

		const selection = app.map._docLayer._cellSelectionArea;
		if (selection && this._viewedRectangle.containsPoint(selection.center)) {
			const center = selection.center;
			return new cool.SimplePoint(center[0], center[1]);
		}

		return super.zoomAnchorPoint();
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

	// Calc needs splitx/splity so the server renders frozen and split panes
	// correctly.
	protected override clientVisibleAreaMessage(): string {
		const splitPosition = app.map._docLayer._corePixelsToTwips(
			app.map._docLayer._splitPanesContext
				? app.map._docLayer._splitPanesContext.getSplitPos()
				: new cool.Point(0, 0),
		);

		return (
			super.clientVisibleAreaMessage() +
			' splitx=' +
			Math.round(splitPosition.x) +
			' splity=' +
			Math.round(splitPosition.y)
		);
	}

	// The splitters are placed from the visible area, so they follow it.
	protected override onVisibleAreaChanged(): void {
		if (app.map._docLayer._ySplitter)
			app.map._docLayer._ySplitter.onPositionChange();
		if (app.map._docLayer._xSplitter)
			app.map._docLayer._xSplitter.onPositionChange();
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

		// Project the current scroll position onto the scrollbar track.
		scrollProps.startX =
			scrollProps.xOffset +
			ViewLayoutCalc.viewToTrack(
				scrollProps.viewX,
				this.viewSize.pX,
				documentAnchor.size[0],
				scrollProps.horizontalScrollLength,
				scrollProps.horizontalScrollSize,
			);
		scrollProps.startY =
			scrollProps.yOffset +
			ViewLayoutCalc.viewToTrack(
				scrollProps.viewY,
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

	// pX, pY are view-space deltas in screen (canvas) pixels, from a relative
	// scroll (wheel, scrollbar, auto-scroll). In RTL a rightward screen delta
	// moves the document the other way, so mirror X here - at the screen-input
	// boundary - and keep the document-space core direction-agnostic.
	public override scroll(pX: number, pY: number): any {
		if (this.isRTL()) pX = -pX;
		this.scrollByDocumentDelta(pX, pY);
	}

	// The scroll position is scrollProperties.viewX and viewY, in canvas (core)
	// pixels. Calc's viewed rectangle is the frame placed at that position, so it
	// is rebuilt from there whenever the position or the frame size changes.
	// Assigning through the property rather than the field keeps the section
	// container refresh and the previous-rectangle bookkeeping.
	public rebuildViewedRectangle(): void {
		const frame = this.frameSize;

		// A document container that is hidden or not laid out yet has no frame, so
		// there is nowhere to place it and the current rectangle stays.
		if (frame.pX <= 0 || frame.pY <= 0) return;

		this.clampScrollPosition(frame);

		this.viewedRectangle = cool.SimpleRectangle.fromCorePixels([
			this.scrollProperties.viewX,
			this.scrollProperties.viewY,
			frame.pX,
			frame.pY,
		]);
	}

	// Keep the scroll position inside the range the document spans at the current
	// frame size, [0, view size - frame]. A frame that grows - a larger window, a
	// sidebar or a ribbon that closes, the on-screen keyboard going down - makes
	// that range smaller, while the position stays where the last scroll left it.
	private clampScrollPosition(frame: cool.SimplePoint): void {
		const maxX = Math.max(0, this.viewSize.pX - frame.pX);
		const maxY = Math.max(0, this.viewSize.pY - frame.pY);

		this.scrollProperties.viewX = Math.min(
			maxX,
			Math.max(0, this.scrollProperties.viewX),
		);
		this.scrollProperties.viewY = Math.min(
			maxY,
			Math.max(0, this.scrollProperties.viewY),
		);
	}

	// Calc has one shape for the viewed rectangle, so both a scroll and a zoom
	// arrive at it the same way.
	protected override refreshVisibleAreaRectangle(): void {
		this.rebuildViewedRectangle();
	}

	// pX, pY are document-space scroll deltas in canvas (core) pixels (no RTL
	// mirroring). Clamps to the scrollable range, moves the scroll position and
	// refreshes headers/cursor/tiles. Both scroll() (screen input) and
	// scrollTo() (absolute document position) funnel through here.
	private scrollByDocumentDelta(pX: number, pY: number): void {
		const documentAnchor = this.getDocumentAnchorSection();

		const prevX = this.scrollProperties.viewX;
		const prevY = this.scrollProperties.viewY;
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

		this.scrollProperties.viewX = newX;
		this.scrollProperties.viewY = newY;
		this.rebuildViewedRectangle();

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

	// Calc scrolls by moving the viewed rectangle. It holds the plain scroll
	// offset in core pixels, with no centring, so it is the viewport.
	public override getViewportCorePixelBounds(): cool.Bounds {
		const rect = this._viewedRectangle;
		return new cool.Bounds(
			new cool.Point(rect.pX1, rect.pY1),
			new cool.Point(rect.pX1 + rect.pWidth, rect.pY1 + rect.pHeight),
		);
	}

	// pX, pY are absolute view-space positions in canvas (core) pixels. This is
	// an absolute document-space move, so it bypasses scroll()'s RTL screen
	// mirroring and applies the delta directly.
	public override scrollTo(pX: number, pY: number): void {
		const deltaX = pX - this.scrollProperties.viewX;
		const deltaY = pY - this.scrollProperties.viewY;
		if (deltaX !== 0 || deltaY !== 0)
			this.scrollByDocumentDelta(deltaX, deltaY);
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
