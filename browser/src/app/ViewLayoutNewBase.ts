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

/*
        With the relatively new "ViewLayout" class, a new phase has started.

        In this phase, "View"s will try to carry the burden of coordinate systems,
    where CanvasSectionContainer should only know about screen coordinates. For now, CSC also knows about document coordinates.

        ViewLayoutBase represents some parts of our old approach. "ViewLayoutMultiPage" and "ViewLayoutCompareChanges" represent mostly the new approach.
    So the latter 2 are closer to each other than they are to "ViewLayoutBase".

        This differences require a new base class (ViewLayoutNewBase) for new approach's classes. It is "required", because there are duplicated code otherwise.
    This class may be merged with "ViewLayoutBase" in the future.
*/

class ViewLayoutNewBase extends ViewLayoutBase {
	public readonly type: string = 'ViewLayoutNewBase';

	constructor() {
		super();

		// The stacked-page subclasses (MultiPage/FileBased/CompareChanges) install
		// their own zoomend/resize handlers that rebuild page geometry and request
		// tiles. The plain single-window layout (Impress/Draw edit) has none, so
		// without these the viewed rectangle (which carries the centering offset)
		// is never rebuilt on load/zoom/resize: the slide would not centre and
		// would go blank after a zoom. Guarded by type so it is a no-op for the
		// subclasses (they handle their own).
		app.map.on('zoomend', this.rebuildSingleWindowView.bind(this));
		app.events.on('resize', this.rebuildSingleWindowViewDeferred.bind(this));
		app.layoutingService.appendLayoutingTask(() =>
			this.rebuildSingleWindowView(),
		);
	}

	// The new-structure layouts (Calc, MultiPage, FileBased, CompareChanges)
	// are off-map and zoom through ZoomControl.
	public override usesZoomControl(): boolean {
		return true;
	}

	// The single-window layouts that place one page/slide with the inherited view
	// machinery (centred when smaller than the viewport): Impress/Draw edit (plain
	// ViewLayoutNewBase) and Writer. Calc (fills the viewport, own scroll/viewed
	// rectangle) and the stacked-page subclasses build their own and opt out.
	protected usesSingleWindowView(): boolean {
		return this.type === 'ViewLayoutNewBase';
	}

	// The 'resize' event (a ResizeObserver on the container) can fire before the
	// document anchor section has been resized, so defer to the layouting phase
	// where the frame size has settled - otherwise centering would use a stale
	// frame. Zoom end runs synchronously (no resize in flight, and we want the
	// centred rectangle committed before the next redraw).
	private rebuildSingleWindowViewDeferred(): void {
		if (!this.usesSingleWindowView()) return;
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
		this.scrollProperties.viewX = Math.max(0, this._viewedRectangle.pX1);
		this.scrollProperties.viewY = Math.max(0, this._viewedRectangle.pY1);
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

	public sendClientVisibleArea() {
		const visibleAreaCommand =
			'clientvisiblearea x=' +
			this.viewedRectangle.x1 +
			' y=' +
			this.viewedRectangle.y1 +
			' width=' +
			this.viewedRectangle.width +
			' height=' +
			this.viewedRectangle.height;

		app.socket.sendMessage(visibleAreaCommand);

		if (app.map.contextToolbar) app.map.contextToolbar.hideContextToolbar(); // hide context toolbar when scroll/window resize etc...
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

		// scroll() divides pY by 20 for mouse wheel dampening, so the vertical
		// ratio must compensate: diffY * 20 / 20 = diffY (1:1 scrollbar tracking).
		// Horizontal scroll has no such division, so ratio is 1.
		// startX/startY are managed by scroll() and scrollTo() — not recalculated
		// here to avoid drift caused by the vScrollMultiplier and rounding.
		// We will keep this approach until we remove document-size based scroll entirely (will replace with view-size based scroll).
		this.scrollProperties.verticalScrollRatio = 20;
		this.scrollProperties.horizontalScrollRatio = 1;

		// Properties for quick scrolling.
		this.scrollProperties.verticalScrollStep = documentAnchor.size[1] / 2;
		this.scrollProperties.horizontalScrollStep = documentAnchor.size[0] / 2;
	}

	// Lives here rather than in ViewLayoutBase: the single-window layouts
	// (Writer/Impress/Draw) inherit it, while Calc overrides scroll() without
	// calling it - putting it in the base introduced view jumps in Calc.
	public unselectCommentOnScroll() {
		const commentSection = app.sectionContainer.getSectionWithName(
			app.CSections.CommentList.name,
		) as cool.CommentSection;

		if (commentSection && commentSection.sectionProperties.selectedComment) {
			commentSection.unselect();
		}
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

	public get viewSize() {
		return this._viewSize;
	}

	public set viewSize(size: cool.SimplePoint) {
		// Single-window layouts (Impress/Draw/Writer/Calc) set the scrollable
		// extent from the document size. Stacked-page layouts compute _viewSize
		// themselves and do not call this.
		this._viewSize = size;
	}

	public get viewedRectangle() {
		return this._viewedRectangle;
	}

	public set viewedRectangle(rectangle: cool.SimpleRectangle) {
		// Single-window layouts set the viewed rectangle directly (e.g. zoom via
		// setViewRectangleFromPointAndScale). Stacked-page layouts assign
		// _viewedRectangle from page geometry in refreshVisibleAreaRectangleImpl
		// and do not use this setter. The map no longer writes it: _syncTilePanePos
		// early-returns for usesZoomControl layouts.
		if (!this._viewedRectangle.equals(rectangle.toArray()))
			this.lastViewedRectangle = this._viewedRectangle.clone();
		this._viewedRectangle = rectangle;
		app.sectionContainer.onNewDocumentTopLeft();
		app.sectionContainer.requestReDraw();
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
			app.layoutingService.appendLayoutingTask(() => {
				if (snapAxis === 'x') this.scrollProperties.viewX = 0;
				else this.scrollProperties.viewY = 0;
				this.refreshVisibleAreaRectangle();
			});
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

	// Default single-window tile grid: the tiles covering the viewed rectangle
	// for the currently selected part (slide/page). Writer is a single part (0);
	// Impress/Draw use the selected slide/page. Stacked-page layouts and Calc
	// override this.
	protected override refreshCurrentCoordList(): void {
		this.currentCoordList.length = 0;
		const zoom = Math.round(app.map.getZoom());
		const tileSize = RenderManager.tileSize;
		const part = app.map._docLayer._selectedPart || 0;
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

	// Recompute the visible area, push it to the server and request the tiles
	// needed to render it.
	protected commitVisibleAreaAndRequestTiles(): void {
		this.refreshVisibleAreaRectangle();

		if (app.map._docLayer?._cursorMarker)
			app.map._docLayer._cursorMarker.update();

		app.map._docLayer._sendClientZoom();
		this.sendClientVisibleArea();

		this.refreshCurrentCoordList();
		RenderManager.beginTransaction();
		RenderManager.checkRequestTiles(this.currentCoordList);
		RenderManager.endTransaction(null);
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
}
