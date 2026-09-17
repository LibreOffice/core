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

class ViewLayoutMultiPage extends ViewLayoutBase {
	public readonly type: string = 'ViewLayoutMultiPage';
	public gapBetweenPages = 20; // Core pixels.
	private gapBeforeCommentColumn = 40; // Core pixels.
	private minimumSideMargin = 20; // Core pixels.
	private commentReserveUsedForZoom = 0;
	private maxRowsSize = 2;
	public documentRectangles = Array<cool.SimpleRectangle>();
	private viewRectangles = Array<cool.SimpleRectangle>();

	constructor() {
		super();

		app.map.on('zoomend', this.reset, this);
		app.map.on('insertannotation', this.onCommentLayoutChange, this);
		app.map.on('deleteannotation', this.onCommentLayoutChange, this);
		app.map.on('importannotations', this.onCommentLayoutChange, this);
		app.map.on('showannotationschanged', this.onCommentLayoutChange, this);

		this.adjustViewZoomLevel();
		this.reset();
	}

	public override dispose(): void {
		app.map.off('zoomend', this.reset, this);
		app.map.off('insertannotation', this.onCommentLayoutChange, this);
		app.map.off('deleteannotation', this.onCommentLayoutChange, this);
		app.map.off('importannotations', this.onCommentLayoutChange, this);
		app.map.off('showannotationschanged', this.onCommentLayoutChange, this);
		super.dispose();
	}

	private onCommentLayoutChange(): void {
		if (this.getCommentColumnReserve() === this.commentReserveUsedForZoom)
			return;

		this.adjustViewZoomLevel();
		this.reset();
	}

	public override onResize(): void {
		super.onResize();
		this.reset();
	}

	private getPagesWidth(): number {
		Util.ensureValue(app.activeDocument);

		const pageRectangleList = app.file.writer.pageRectangleList;

		if (pageRectangleList.length === 0) return app.activeDocument.fileSize.pX;

		let result = 0;

		for (let i = 0; i < pageRectangleList.length; i += this.maxRowsSize) {
			const rowEnd = Math.min(i + this.maxRowsSize, pageRectangleList.length);
			let rowWidth = 0;

			for (let j = i; j < rowEnd; j++) {
				if (j > i) rowWidth += this.gapBetweenPages;
				rowWidth += pageRectangleList[j][2] * app.twipsToPixels;
			}

			result = Math.max(result, rowWidth);
		}

		return result;
	}

	private getCommentColumnReserve(): number {
		const commentSection = app.sectionContainer.getSectionWithName(
			app.CSections.CommentList.name,
		) as cool.CommentSection;

		if (!commentSection || commentSection.commentsHiddenOrNotPresent())
			return 0;

		return this.gapBeforeCommentColumn + cool.CommentSection.getCommentWidth();
	}

	private getZoomForWidth(
		availableWidth: number,
		contentWidth: number,
	): number {
		Util.ensureValue(app.activeDocument);

		const min = 0.1;
		const max = 10;

		const ratio = Math.max(1, availableWidth) / contentWidth;
		let zoom = app.activeDocument.getScaleZoom(ratio);
		zoom = Math.min(max, Math.max(min, zoom));

		if (zoom > 1) zoom = Math.floor(zoom);

		return zoom;
	}

	public override adjustViewZoomLevel() {
		Util.ensureValue(app.activeDocument);

		const availableWidth =
			this.getDocumentAnchorSection().size[0] - this.minimumSideMargin * 2;
		const pagesWidth = this.getPagesWidth();
		const commentReserve = this.getCommentColumnReserve();

		this.commentReserveUsedForZoom = commentReserve;

		const zoomWithoutComments = this.getZoomForWidth(
			availableWidth,
			pagesWidth,
		);

		let zoom = zoomWithoutComments;

		if (commentReserve > 0) {
			const zoomWithComments = this.getZoomForWidth(
				availableWidth - commentReserve,
				pagesWidth,
			);

			const floor = app.activeDocument.getZoomIndex(60);
			const onlyCommentsFallBelowFloor =
				zoomWithComments < floor && zoomWithoutComments >= floor;

			zoom = onlyCommentsFallBelowFloor ? floor : zoomWithComments;
		}

		this.applyZoom(zoom);
	}

	private resetViewLayout() {
		this.documentRectangles.length = 0;
		this.viewRectangles.length = 0;

		if (app.file.writer.pageRectangleList.length === 0) return;

		const frameSize = this.getDocumentAnchorSection().size;
		const commentReserve = this.getCommentColumnReserve();

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
		let contentRight = 0;

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
				if (x + addition < frameSize[0] || j === i) {
					if (x + addition > frameSize[0]) {
						go = false;
					}

					x += addition;
					totalWidth += this.documentRectangles[j].pWidth;

					j++;
				} else go = false;
			}

			if (x < frameSize[0]) {
				const rowItemCount = j - i;
				const gap = (rowItemCount - 1) * this.gapBetweenPages;
				const rowWidth = totalWidth + gap;
				const margin = Math.max(
					this.minimumSideMargin,
					(frameSize[0] - commentReserve - rowWidth) * 0.5,
				);
				let currentX = margin;
				let maxY = 0;
				for (let k = i; k < j; k++) {
					this.viewRectangles[k].pX1 = currentX;
					this.viewRectangles[k].pY1 = lastY;

					currentX += this.documentRectangles[k].pWidth + this.gapBetweenPages;
					maxY = Math.max(maxY, this.documentRectangles[k].pHeight);
				}

				contentRight = Math.max(contentRight, margin + rowWidth);
				lastY += maxY + this.gapBetweenPages;
			} else {
				this.viewRectangles[i].pX1 = this.minimumSideMargin;
				this.viewRectangles[i].pY1 = lastY;

				contentRight = Math.max(
					contentRight,
					this.minimumSideMargin + this.documentRectangles[i].pWidth,
				);
				lastY += this.documentRectangles[i].pHeight + this.gapBetweenPages;
			}

			i = j - 1;
		}

		this._viewSize.pX = Math.max(
			frameSize[0],
			contentRight + commentReserve + this.minimumSideMargin,
		);
		this._viewSize.pY = Math.max(lastY, frameSize[1]);

		// Capture the page-layout extent, the comment column included. Side-space
		// math reads this base so it does not feed back into the comment width
		// computation.
		this._baseViewSize = this._viewSize.clone();
	}

	protected override getBaseViewSize(): cool.SimplePoint {
		return this._baseViewSize;
	}

	public override ensureViewSizeCoversComments(
		extraWidth: number,
		bottomY: number,
	): void {
		super.ensureViewSizeCoversComments(0, bottomY);
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

	protected override buildsViewedRectangleFromPages(): boolean {
		return true;
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

			// A text document renders everything as part 0.
			this.pushTileGrid(
				startX,
				startY,
				columnCount,
				rowCount,
				zoom,
				tileSize,
				'0' as PartNumber,
				added,
			);
		}
	}

	protected updateViewData() {
		if (!app.file.writer.pageRectangleList.length) return;

		this.commitVisibleAreaAndRequestTiles();
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

		// Twice the space between the last page and the right edge of the view.
		const viewWidth = this.getDocumentAnchorSection().size[0];

		return (viewWidth - maxX - this.gapBeforeCommentColumn) * 2;
	}
}
