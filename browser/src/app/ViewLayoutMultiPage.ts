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

class ViewLayoutMultiPage extends ViewLayoutNewBase {
	public readonly type: string = 'ViewLayoutMultiPage';
	public gapBetweenPages = 20; // Core pixels.
	private maxRowsSize = 2;
	public documentRectangles = Array<cool.SimpleRectangle>();
	private viewRectangles = Array<cool.SimpleRectangle>();

	constructor() {
		super();

		app.events.on('resize', this.reset.bind(this));
		app.map.on('zoomend', this.reset.bind(this));

		this.adjustViewZoomLevel();
		this.reset();
	}

	public override adjustViewZoomLevel() {
		Util.ensureValue(app.activeDocument);

		const min = 0.1;
		const max = 10;

		const anchorSection = this.getDocumentAnchorSection();

		// Take 50% of the width and try to render 2 pages side by side.
		const halfWidth = Math.round(anchorSection.size[0] * 0.5);

		const ratio = halfWidth / app.activeDocument.fileSize.pX;
		let zoom = app.map.getScaleZoom(ratio);
		zoom = Math.min(max, Math.max(min, zoom));

		if (zoom > 1) zoom = Math.floor(zoom);

		app.map.setZoom(zoom, { animate: false });
	}

	private resetViewLayout() {
		this.documentRectangles.length = 0;
		this.viewRectangles.length = 0;

		if (app.file.writer.pageRectangleList.length === 0) return;

		const canvasSize = app.sectionContainer.getViewSize();

		// Copy the page rectangle array.
		for (let i = 0; i < app.file.writer.pageRectangleList.length; i++) {
			const r = app.file.writer.pageRectangleList[i];
			this.documentRectangles.push(
				new cool.SimpleRectangle(r[0], r[1], r[2], r[3]),
			);
			this.viewRectangles.push(new cool.SimpleRectangle(0, 0, r[2], r[3])); // Width and height are the same. Screen positions will differ.

			this.documentRectangles[i].part = i;
			this.viewRectangles[i].part = i;
		}

		let lastY = this.gapBetweenPages;
		this._viewSize.pX = canvasSize[0];

		for (let i = 0; i < this.documentRectangles.length; i++) {
			let x = 0;

			let j = i;
			let totalWidth = 0;
			let go = true;

			while (
				go &&
				j - i < this.maxRowsSize &&
				j < this.documentRectangles.length
			) {
				const addition =
					this.documentRectangles[j].pWidth + this.gapBetweenPages;
				if (x + addition < canvasSize[0] || j === i) {
					if (x + addition > canvasSize[0]) {
						go = false;
					}

					x += addition;
					totalWidth += this.documentRectangles[j].pWidth;

					j++;
				} else go = false;
			}

			if (x < canvasSize[0]) {
				const rowItemCount = j - i;
				const gap = (rowItemCount - 1) * this.gapBetweenPages;
				const margin = (canvasSize[0] - (totalWidth + gap)) * 0.5;
				let currentX = margin;
				let maxY = 0;
				for (let k = i; k < j; k++) {
					this.viewRectangles[k].pX1 = currentX;
					this.viewRectangles[k].pY1 = lastY;

					currentX += this.documentRectangles[k].pWidth + this.gapBetweenPages;
					maxY = Math.max(maxY, this.documentRectangles[k].pHeight);
				}

				lastY += maxY + this.gapBetweenPages;
			} else {
				if (x > this._viewSize.pX)
					this._viewSize.pX = x + this.gapBetweenPages * 2;

				this.viewRectangles[i].pX1 = this.gapBetweenPages;
				this.viewRectangles[i].pY1 = lastY;

				lastY += this.documentRectangles[i].pHeight + this.gapBetweenPages;
			}

			i = j - 1;
		}

		this._viewSize.pY = Math.max(lastY, canvasSize[1]);
	}

	// Get the page rectangle or its corresponding view rectangle which contains the given point (document point or view point).
	public getClosestRectangleIndex(
		point: cool.SimplePoint,
		documentPoint = true,
	): number {
		const rectangleList =
			documentPoint === true ? this.documentRectangles : this.viewRectangles;

		for (let i = 0; i < rectangleList.length; i++) {
			if (rectangleList[i].containsPoint(point.toArray())) return i; // Return if a rectangle contains our point.
		}

		// Never return null here. This is the last stand. Find the closest rectangle.
		// This part assumes page rectangles are not angled (portrait / landscape is fine).
		let closest = Number.POSITIVE_INFINITY;
		let part = -1;
		for (let i = 0; i < rectangleList.length; i++) {
			const rectangle = rectangleList[i];
			let current: number;

			if (point.x >= rectangle.x1 && point.x <= rectangle.x2) {
				current = Math.min(
					Math.abs(point.y - rectangle.y2),
					Math.abs(point.y - rectangle.y1),
				);
			} else if (point.y >= rectangle.y1 && point.y <= rectangle.y2) {
				current = Math.min(
					Math.abs(point.x - rectangle.x2),
					Math.abs(point.x - rectangle.x1),
				);
			} else {
				current = Math.min(
					point.distanceTo([rectangle.x1, rectangle.y1]),
					point.distanceTo([rectangle.x2, rectangle.y1]),
					point.distanceTo([rectangle.x1, rectangle.y2]),
					point.distanceTo([rectangle.x2, rectangle.y2]),
				);
			}

			if (current < closest) {
				closest = current;
				part = i;
			}
		}

		return part;
	}

	protected refreshVisibleAreaRectangle(): void {
		this.refreshVisibleAreaRectangleImpl(
			this.documentRectangles,
			this.viewRectangles,
			'x',
		);
	}

	protected override refreshCurrentCoordList() {
		const { zoom, tileSize, view } = this.beginCoordList();
		const added: Set<string> = new Set();

		for (let i = 0; i < this.documentRectangles.length; i++) {
			const viewRect = this.viewRectangles[i];

			if (!view.intersectsRectangle(viewRect.toArray())) continue;

			const docRect = this.documentRectangles[i];
			const { vx1, vy1, vx2, vy2 } = this.getVisibleViewBounds(view, viewRect);

			// Map the visible view portion back to document coordinates.
			const docVisX1 = docRect.pX1 + (vx1 - viewRect.pX1);
			const docVisY1 = docRect.pY1 + (vy1 - viewRect.pY1);
			const docVisX2 = docRect.pX1 + (vx2 - viewRect.pX1);
			const docVisY2 = docRect.pY1 + (vy2 - viewRect.pY1);

			const startX = Math.floor(docVisX1 / tileSize) * tileSize;
			const startY = Math.floor(docVisY1 / tileSize) * tileSize;
			const columnCount = Math.ceil((docVisX2 - startX) / tileSize);
			const rowCount = Math.ceil((docVisY2 - startY) / tileSize);

			this.pushTileGrid(
				startX,
				startY,
				columnCount,
				rowCount,
				zoom,
				tileSize,
				0,
				added,
			);
		}
	}

	protected updateViewData() {
		if (!app.file.writer.pageRectangleList.length) return;

		this.refreshVisibleAreaRectangle();

		if (app.map._docLayer?._cursorMarker)
			app.map._docLayer._cursorMarker.update();

		app.map._docLayer._sendClientZoom();
		this.sendClientVisibleArea();

		this.refreshCurrentCoordList();
		RenderManager.beginTransaction();
		RenderManager.checkRequestTiles(this.currentCoordList);
		RenderManager.endTransaction(null);

		// We most likely scrolled the view. We also need to check ruler position.
		if (app.UI.horizontalRuler) app.UI.horizontalRuler.fixOffset();
	}

	// Map a document point to its on-screen position using an explicitly
	// supplied page index. The corner-mapping getters (documentToViewX/Y) each
	// re-resolve the index via getClosestRectangleIndex, so two points of the
	// same tile/page can land on different pages (gaps between stacked pages,
	// mixed page sizes, or the closest-fallback heuristic). Callers that already
	// know which page a point belongs to should use this to stay on one index.
	public documentPointToScreenWithIndex(
		point: cool.SimplePoint,
		index: number,
	): { x: number; y: number } {
		const anchor = app.sectionContainer.getDocumentAnchor();
		return {
			x:
				this.viewRectangles[index].pX1 +
				(point.pX - this.documentRectangles[index].pX1) -
				this.scrollProperties.viewX +
				anchor[0],
			y:
				this.viewRectangles[index].pY1 +
				(point.pY - this.documentRectangles[index].pY1) -
				this.scrollProperties.viewY +
				anchor[1],
		};
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

	public override scroll(
		pX: number,
		pY: number,
		userIsScrolling: boolean = false,
	): boolean {
		if (userIsScrolling) this.unselectCommentOnScroll();
		const scrolled = super.scroll(pX, pY, userIsScrolling);

		if (scrolled) {
			this.updateViewData();
			app.sectionContainer.requestReDraw();
		}

		return scrolled;
	}

	public override scrollTo(
		pX: number,
		pY: number,
		userIsScrolling: boolean = false,
	): void {
		const point = cool.SimplePoint.fromCorePixels([pX, pY]);
		if (!this.viewedRectangle.containsPoint(point.toArray())) {
			const index = this.getClosestRectangleIndex(point);
			const layoutR = this.documentRectangles[index];
			const viewR = this.viewRectangles[index];

			if (layoutR) {
				let scrolled = false;

				// Check if the target X is already visible in the viewport.
				const viewportWidth = this.getDocumentAnchorSection().size[0];
				const xVisibleInViewport =
					viewR.pX1 >= this.scrollProperties.viewX &&
					viewR.pX1 + layoutR.pWidth <=
						this.scrollProperties.viewX + viewportWidth;

				if (!xVisibleInViewport) {
					this.scrollProperties.startX = Math.round(
						(viewR.pX1 / this._viewSize.pX) *
							this.scrollProperties.horizontalScrollLength,
					);
					this.scrollProperties.viewX = Math.round(
						(this.scrollProperties.startX /
							this.scrollProperties.horizontalScrollLength) *
							this.viewSize.pX,
					);
					scrolled = true;
				}

				if (!this.viewedRectangle.containsY(point.y)) {
					this.scrollProperties.startY = Math.round(
						(viewR.pY1 / this._viewSize.pY) *
							this.scrollProperties.verticalScrollLength,
					);
					this.scrollProperties.viewY = Math.round(
						(this.scrollProperties.startY /
							this.scrollProperties.verticalScrollLength) *
							this.viewSize.pY,
					);
					scrolled = true;
				}

				if (scrolled) {
					this.updateViewData();
					app.sectionContainer.requestReDraw();
				}
			}
		}
	}

	public reset() {
		if (!app.file.writer.pageRectangleList.length) return;

		app.layoutingService.appendLayoutingTask(() => {
			this.resetViewLayout();
			this.updateViewData();
		});
	}

	public getTotalSideSpace() {
		const maxX: number = this.viewRectangles.reduce((result, currentItem) => {
			return Math.max(currentItem.pX2, result);
		}, 0);
		const minX: number = this.viewRectangles.reduce((result, currentItem) => {
			return Math.min(currentItem.pX1, result);
		}, 100000);
		const width = maxX - minX;

		const sideSpace = this.viewSize.pX - width;

		return sideSpace;
	}
}
