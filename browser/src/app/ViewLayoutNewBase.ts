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

	// This is duplicated in ViewLayoutWriter and ViewLayoutNewBase because the
	// other way would have been to move this to ViewLayoutBase, but that introduces
	// view jumps in Calc, so code duplication seemed like the obvious choice after
	// that.
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
			const max =
				this.scrollProperties.horizontalScrollLength -
				this.scrollProperties.horizontalScrollSize;
			const min = 0;
			const current = this.scrollProperties.startX + pX;
			const endPosition = Math.max(min, Math.min(max, current));

			if (endPosition !== this.scrollProperties.startX) {
				this.scrollProperties.startX = endPosition;
				const hScrollMultiplier =
					max > 0
						? (this.scrollProperties.horizontalScrollLength -
								this.scrollProperties.horizontalScrollSizeForScrolling) /
							max
						: 1;
				this.scrollProperties.viewX = Math.round(
					(endPosition / this.scrollProperties.horizontalScrollLength) *
						this.viewSize.pX *
						hScrollMultiplier,
				);
				scrolled = true;
			}
		}

		if (pY !== 0 && this.canScrollVertical(documentAnchor)) {
			pY /= 20;

			const max =
				this.scrollProperties.verticalScrollLength -
				this.scrollProperties.verticalScrollSize;
			const min = 0;
			const current = this.scrollProperties.startY + pY;
			const endPosition = Math.max(min, Math.min(max, current));

			if (endPosition !== this.scrollProperties.startY) {
				this.scrollProperties.startY = endPosition;
				const vScrollMultiplier =
					max > 0
						? (this.scrollProperties.verticalScrollLength -
								this.scrollProperties.verticalScrollSizeForScrolling) /
							max
						: 1;
				this.scrollProperties.viewY = Math.round(
					(endPosition / this.scrollProperties.verticalScrollLength) *
						this.viewSize.pY *
						vScrollMultiplier,
				);
				scrolled = true;
			}
		}
		return scrolled;
	}

	public get viewSize() {
		return this._viewSize;
	}

	public set viewSize(size: cool.SimplePoint) {
		return; // Disable setting the size externally.
	}

	public get viewedRectangle() {
		return this._viewedRectangle;
	}

	public set viewedRectangle(rectangle: cool.SimpleRectangle) {
		return; // Disable setting the viewed rectangle externally.
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

	protected refreshVisibleAreaRectangle(): void {
		// Subclasses override to recompute the viewed rectangle.
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
		part: number,
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
