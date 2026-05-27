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

// ViewLayoutFileBased: stacks all parts vertically in a single column, centering
// narrower pages within the widest page. Used by Draw/PDF and read-only Impress
// in app.file.fileBasedView mode. Per-part dimensions come from
// docLayer._partDimensions; falls back to uniform (_partWidthTwips,
// _partHeightTwips) when the per-part array is empty.

class ViewLayoutFileBased extends ViewLayoutNewBase {
	public readonly type: string = 'ViewLayoutFileBased';
	public documentRectangles = Array<cool.SimpleRectangle>();
	public viewRectangles = Array<cool.SimpleRectangle>();

	constructor() {
		super();

		app.events.on('resize', this.reset.bind(this));
		app.map.on('zoomend', this.reset.bind(this));

		// Populate rectangles synchronously so callers that read documentRectangles
		// right after `new ViewLayoutFileBased()` (e.g. paint paths kicked off
		// immediately after swapLayout) see a fully-built layout.
		this.resetViewLayout();
		app.layoutingService.appendLayoutingTask(() => this.updateViewData());
	}

	private getPartCount(): number {
		const docLayer = app.map._docLayer;
		if (!docLayer) return 0;
		return docLayer._parts || 0;
	}

	private getPartSizeTwips(i: number): { w: number; h: number } {
		const docLayer = app.map._docLayer;
		if (
			docLayer._partDimensions &&
			docLayer._partDimensions.length === docLayer._parts &&
			docLayer._partDimensions[i]
		) {
			return {
				w: docLayer._partDimensions[i].x,
				h: docLayer._partDimensions[i].y,
			};
		}
		return {
			w: docLayer._partWidthTwips,
			h: docLayer._partHeightTwips,
		};
	}

	private resetViewLayout(): void {
		this.documentRectangles.length = 0;
		this.viewRectangles.length = 0;

		const partCount = this.getPartCount();
		if (partCount === 0) return;

		const docLayer = app.map._docLayer;
		const gap = docLayer._spaceBetweenParts;

		let maxWidth = 0;
		for (let i = 0; i < partCount; i++)
			maxWidth = Math.max(maxWidth, this.getPartSizeTwips(i).w);

		const canvasSize = app.sectionContainer.getViewSize();
		const canvasWidthTwips = Math.round(canvasSize[0] * app.pixelsToTwips);
		const layoutWidthTwips = Math.max(maxWidth, canvasWidthTwips);

		let cumulativeY = 0;
		for (let i = 0; i < partCount; i++) {
			const size = this.getPartSizeTwips(i);

			const docRect = new cool.SimpleRectangle(0, cumulativeY, size.w, size.h);
			const viewRect = new cool.SimpleRectangle(
				Math.round((layoutWidthTwips - size.w) * 0.5),
				cumulativeY,
				size.w,
				size.h,
			);
			docRect.part = i;
			viewRect.part = i;

			this.documentRectangles.push(docRect);
			this.viewRectangles.push(viewRect);

			cumulativeY += size.h + gap;
		}

		this._viewSize.pX = Math.max(
			Math.round(layoutWidthTwips * app.twipsToPixels),
			canvasSize[0],
		);
		this._viewSize.pY = Math.max(
			Math.round(cumulativeY * app.twipsToPixels),
			canvasSize[1],
		);
	}

	public reset(): void {
		if (!app.map._docLayer || !app.map._docLayer._parts) return;

		this.resetViewLayout();
		app.layoutingService.appendLayoutingTask(() => this.updateViewData());
	}

	// Per-part document rect (X=0 baseline, cumulative Y) in pixels at `ratio`.
	// Tile-queue builders work in stacked-document coords; this gives them the
	// per-part rect at the queried zoom without recomputing twips->pixels math.
	public getDocumentPartRectAtRatio(
		idx: number,
		ratio: number,
	): { x: number; y: number; w: number; h: number } | null {
		const r = this.documentRectangles[idx];
		if (!r) return null;
		return {
			x: Math.round(r.x1 * ratio),
			y: Math.round(r.y1 * ratio),
			w: Math.round(r.width * ratio),
			h: Math.round(r.height * ratio),
		};
	}

	// Per-part view rect (X-centered, cumulative Y) in pixels at `ratio`.
	public getViewPartRectAtRatio(
		idx: number,
		ratio: number,
	): { x: number; y: number; w: number; h: number } | null {
		const r = this.viewRectangles[idx];
		if (!r) return null;
		return {
			x: Math.round(r.x1 * ratio),
			y: Math.round(r.y1 * ratio),
			w: Math.round(r.width * ratio),
			h: Math.round(r.height * ratio),
		};
	}

	// Twip-space document rect for non-pixel callers.
	public getDocumentPartRect(idx: number): cool.SimpleRectangle | null {
		return this.documentRectangles[idx] ?? null;
	}

	// Cumulative Y offset of part `i` in core pixels (top of the page).
	public getPartYOffset(partIndex: number): number {
		if (partIndex < 0 || partIndex >= this.viewRectangles.length) return 0;
		return this.viewRectangles[partIndex].pY1;
	}

	// X offset of part `i` (centering) in core pixels.
	public getPartXOffset(partIndex: number): number {
		if (partIndex < 0 || partIndex >= this.viewRectangles.length) return 0;
		return this.viewRectangles[partIndex].pX1;
	}

	// Find which part contains the given core-pixel point in document space
	// (i.e. pY relative to the stacked layout's origin, not viewport).
	public getPartIndexAtDocumentY(pY: number): number {
		for (let i = 0; i < this.documentRectangles.length; i++) {
			const r = this.documentRectangles[i];
			if (pY >= r.pY1 && pY < r.pY1 + r.pHeight) return i;
		}
		if (this.documentRectangles.length === 0) return 0;
		if (pY < this.documentRectangles[0].pY1) return 0;
		return this.documentRectangles.length - 1;
	}

	private getClosestRectangleIndex(
		point: cool.SimplePoint,
		documentPoint = true,
	): number {
		const rectangleList = documentPoint
			? this.documentRectangles
			: this.viewRectangles;

		for (let i = 0; i < rectangleList.length; i++) {
			if (rectangleList[i].containsPoint(point.toArray())) return i;
		}

		let closest = Number.POSITIVE_INFINITY;
		let part = 0;
		for (let i = 0; i < rectangleList.length; i++) {
			const r = rectangleList[i];
			const cy = (r.y1 + r.y2) * 0.5;
			const d = Math.abs(point.y - cy);
			if (d < closest) {
				closest = d;
				part = i;
			}
		}
		return part;
	}

	protected refreshVisibleAreaRectangle(): void {
		this.refreshVisibleAreaRectangleImpl(
			this.documentRectangles,
			this.viewRectangles,
			'y',
		);
	}

	protected refreshCurrentCoordList() {
		const { zoom, tileSize, view } = this.beginCoordList();
		const added: Set<string> = new Set();

		for (let i = 0; i < this.documentRectangles.length; i++) {
			const viewRect = this.viewRectangles[i];

			if (!view.intersectsRectangle(viewRect.toArray())) continue;

			const { vx1, vy1, vx2, vy2 } = this.getVisibleViewBounds(view, viewRect);

			// Map back to per-page local coordinates (tile.x/y are page-local
			// for fileBasedView).
			const localX1 = vx1 - viewRect.pX1;
			const localY1 = vy1 - viewRect.pY1;
			const localX2 = vx2 - viewRect.pX1;
			const localY2 = vy2 - viewRect.pY1;

			const startX = Math.floor(localX1 / tileSize) * tileSize;
			const startY = Math.floor(localY1 / tileSize) * tileSize;
			const columnCount = Math.ceil((localX2 - startX) / tileSize);
			const rowCount = Math.ceil((localY2 - startY) / tileSize);

			this.pushTileGrid(
				startX,
				startY,
				columnCount,
				rowCount,
				zoom,
				tileSize,
				i,
				added,
			);
		}
	}

	protected updateViewData(): void {
		this.refreshVisibleAreaRectangle();

		if (app.map._docLayer?._cursorMarker)
			app.map._docLayer._cursorMarker.update();

		app.map._docLayer._sendClientZoom();
		this.sendClientVisibleArea();

		this.refreshCurrentCoordList();
		TileManager.beginTransaction();
		TileManager.checkRequestTiles(this.currentCoordList);
		TileManager.endTransaction(null);
	}

	public override documentToViewX(point: cool.SimplePoint): number {
		if (this.viewRectangles.length === 0) return super.documentToViewX(point);

		const index = this.getClosestRectangleIndex(point);
		return (
			this.viewRectangles[index].pX1 +
			(point.pX - this.documentRectangles[index].pX1) -
			this.scrollProperties.viewX +
			app.sectionContainer.getDocumentAnchor()[0]
		);
	}

	public override documentToViewY(point: cool.SimplePoint): number {
		if (this.viewRectangles.length === 0) return super.documentToViewY(point);

		const index = this.getClosestRectangleIndex(point);
		return (
			this.viewRectangles[index].pY1 +
			(point.pY - this.documentRectangles[index].pY1) -
			this.scrollProperties.viewY +
			app.sectionContainer.getDocumentAnchor()[1]
		);
	}

	public override canvasToDocumentPoint(
		point: cool.SimplePoint,
	): cool.SimplePoint {
		if (this.viewRectangles.length === 0)
			return super.canvasToDocumentPoint(point);

		point.pX += this.scrollProperties.viewX;
		point.pY += this.scrollProperties.viewY;

		const index = this.getClosestRectangleIndex(point, false);

		const result = point.clone();

		if (this.documentRectangles[index] && this.viewRectangles[index]) {
			result.pX =
				this.documentRectangles[index].pX1 +
				(point.pX - this.viewRectangles[index].pX1) -
				this._documentAnchorPosition[0];
			result.pY =
				this.documentRectangles[index].pY1 +
				(point.pY - this.viewRectangles[index].pY1) -
				this._documentAnchorPosition[1];
		}

		return result;
	}

	public override scroll(pX: number, pY: number): boolean {
		// pX / pY are document-pixel deltas (legacy ViewLayoutBase semantics).
		// ViewLayoutNewBase.scroll dampens pY by /20, so callers that pass raw
		// pixel deltas (page-down, wheel, scrollByPoint) would barely scroll.
		// Apply the delta directly to viewX/viewY and recompute the scrollbar
		// position from viewSize, matching the scrollVerticalWithOffset path.
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
			app.map._docLayer._checkSelectedPart();
		}

		return scrolled;
	}

	public override scrollTo(pX: number, pY: number): void {
		let scrolled = false;

		if (pX !== this.scrollProperties.viewX) {
			this.scrollProperties.viewX = Math.max(0, pX);
			this.scrollProperties.startX = Math.round(
				(this.scrollProperties.viewX / this._viewSize.pX) *
					this.scrollProperties.horizontalScrollLength,
			);
			scrolled = true;
		}

		if (pY !== this.scrollProperties.viewY) {
			this.scrollProperties.viewY = Math.max(0, pY);
			this.scrollProperties.startY = Math.round(
				(this.scrollProperties.viewY / this._viewSize.pY) *
					this.scrollProperties.verticalScrollLength,
			);
			scrolled = true;
		}

		if (scrolled) {
			this.updateViewData();
			app.sectionContainer.requestReDraw();
		}
	}
}
