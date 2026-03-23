// @ts-strict-ignore
/* -*- tab-width: 4 -*- */

/*
 * Copyright the Collabora Online contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/* See CanvasSectionContainer.ts for explanations. */

/* global L */

namespace cool {

export class TilesSection extends CanvasSectionObject {

	anchor: any = [[app.CSections.RulerSpacer.name, 'bottom', app.CSections.ColumnHeader.name, 'bottom', 'top'], [app.CSections.RowHeader.name, 'right', 'left']];
	expand: any = ['top', 'left', 'bottom', 'right'];
	processingOrder: number = app.CSections.Tiles.processingOrder;
	drawingOrder: number = app.CSections.Tiles.drawingOrder;
	zIndex: number = app.CSections.Tiles.zIndex;

	map: any;
	isJSDOM: boolean = false; // testing
	checkpattern: any;

	constructor () {
		super(app.CSections.Tiles.name);

		this.map = window.L.Map.THIS;

		this.sectionProperties.docLayer = this.map._docLayer;
		this.sectionProperties.tsManager = this.sectionProperties.docLayer._painter;
		this.sectionProperties.pageBackgroundInnerMargin = 0; // In core pixels. We don't want backgrounds to have exact same borders with tiles for not making them visible when tiles are rendered.
		this.sectionProperties.pageBackgroundBorderColor = 'lightgrey';
		this.sectionProperties.pageBackgroundTextColor = 'grey';
		this.sectionProperties.pageBackgroundFont = String(40 * app.roundedDpiScale) + 'px Arial';

		/*
			Seems that this number is equal to 45 twips in core.
			Page rectangles are sent from core side.
			Tiles also overlap with page rectangles but they don't overlap entirely.
			Tiles render a slightly longer page.
		*/
		this.sectionProperties.multiPageViewMagicHeightFix = 3;

		this.isJSDOM = typeof window === 'object' && window.name === 'nodejs';

		this.checkpattern = this.makeCheckPattern();
	}

	private makeCheckPattern() {
		var canvas = document.createElement('canvas');
		canvas.width = 256;
		canvas.height = 256;
		var drawctx = canvas.getContext('2d');
		var patternOn = true;
		for (var y = 0; y < 256; y+=32) {
			for (var x = 0; x < 256; x+=32) {
				if (patternOn)
					drawctx.fillStyle = 'darkgray';
				else
					drawctx.fillStyle = 'gray';
				patternOn = !patternOn;
				drawctx.fillRect(x, y, 32, 32);
			}
			patternOn = !patternOn;
		}
		return canvas;
	}

	// eslint-disable-next-line @typescript-eslint/explicit-module-boundary-types
	private paintWithPanes (tile: any, ctx: any, async: boolean): void {
		var tileTopLeft = tile.coords.getPos();
		var tileBounds = new cool.Bounds(tileTopLeft, tileTopLeft.add(new cool.Point(TileManager.tileSize, TileManager.tileSize)));

		for (var i = 0; i < ctx.paneBoundsList.length; ++i) {
			// co-ordinates of this pane in core document pixels
			var paneBounds = ctx.paneBoundsList[i];
			// co-ordinates of the main-(bottom right) pane in core document pixels
			var viewBounds = ctx.viewBounds;

			// into real pixel-land ...
			paneBounds.round();
			viewBounds.round();

			if (paneBounds.intersects(tileBounds)) {
				var paneOffset = paneBounds.getTopLeft(); // allocates
				// Cute way to detect the in-canvas pixel offset of each pane
				paneOffset.x = Math.min(paneOffset.x, viewBounds.min.x);
				paneOffset.y = Math.min(paneOffset.y, viewBounds.min.y);

				this.drawTileInPane(tile, tileBounds, paneBounds, paneOffset, this.context, async);
			}
		}
	}

	private beforeDraw(canvasCtx: CanvasRenderingContext2D): void {
		const mirrorTile: boolean = this.isCalcRTL();
		if (mirrorTile) {
			canvasCtx.save();
			canvasCtx.translate(this.size[0], 0);
			canvasCtx.scale(-1, 1);
		}
	}

	private afterDraw(canvasCtx: CanvasRenderingContext2D): void {
		const mirrorTile: boolean = this.isCalcRTL();
		if (mirrorTile) {
			canvasCtx.restore();
		}
	}

	// eslint-disable-next-line @typescript-eslint/explicit-module-boundary-types
	private drawTileInPane (tile: any, tileBounds: any, paneBounds: any, paneOffset: any, canvasCtx: CanvasRenderingContext2D, clearBackground: boolean): void {
		// intersect - to avoid state thrash through clipping
		var crop = new cool.Bounds(tileBounds.min, tileBounds.max);
		crop.min.x = Math.max(paneBounds.min.x, tileBounds.min.x);
		crop.min.y = Math.max(paneBounds.min.y, tileBounds.min.y);
		crop.max.x = Math.min(paneBounds.max.x, tileBounds.max.x);
		crop.max.y = Math.min(paneBounds.max.y, tileBounds.max.y);

		var cropWidth = crop.max.x - crop.min.x;
		var cropHeight = crop.max.y - crop.min.y;

		if (cropWidth && cropHeight) {
			if (clearBackground || this.containerObject.isZoomChanged() || canvasCtx !== this.context) {
				// Whole canvas is not cleared after zoom has changed, so clear it per tile as they arrive.
				canvasCtx.fillStyle = this.containerObject.getClearColor();
				this.beforeDraw(canvasCtx);
				canvasCtx.fillRect(
					crop.min.x - paneOffset.x,
					crop.min.y - paneOffset.y,
					cropWidth, cropHeight);
				this.afterDraw(canvasCtx);
				var gridSection = this.containerObject.getSectionWithName(app.CSections.CalcGrid.name);
				gridSection.onDrawArea(crop, paneOffset, canvasCtx);
			}

			this.beforeDraw(canvasCtx);
			this.drawTileToCanvasCrop(tile, canvasCtx,
									  crop.min.x - tileBounds.min.x,
									  crop.min.y - tileBounds.min.y,
									  cropWidth, cropHeight,
									  crop.min.x - paneOffset.x,
									  crop.min.y - paneOffset.y,
									  cropWidth, cropHeight);
			this.afterDraw(canvasCtx);
		}
	}

	// eslint-disable-next-line @typescript-eslint/explicit-module-boundary-types
	pdfViewDrawTileBorders (tile: any, offset: any, tileSize: number): void {
		this.context.strokeStyle = 'red';
		this.context.strokeRect(offset.x, offset.y, tileSize, tileSize);
		this.context.font = '20px Verdana';
		this.context.fillStyle = 'black';
		this.context.fillText(tile.coords.x + ' ' + tile.coords.y + ' ' + tile.coords.part, Math.round(offset.x + tileSize * 0.5), Math.round(offset.y + tileSize * 0.5));
	}

	// eslint-disable-next-line @typescript-eslint/explicit-module-boundary-types
	private paintSimple (tile: any, async: boolean): void {
		const tilePos: cool.SimplePoint = tile.coords.getPosSimplePoint();

		if ((async || this.containerObject.isZoomChanged()) && !app.file.fileBasedView) {
			// Non Calc tiles(handled by paintSimple) can have transparent pixels,
			// so clear before paint if the call is an async one.
			// For the full view area repaint, whole canvas is cleared by section container.
			// Whole canvas is not cleared after zoom has changed, so clear it per tile as they arrive even if not async.
			this.context.fillStyle = this.containerObject.getClearColor();
			this.context.fillRect(tilePos.vX, tilePos.vY, TileManager.tileSize, TileManager.tileSize);
		}

		if (app.file.fileBasedView) {
			const partHeightPixels = Math.round((this.sectionProperties.docLayer._partHeightTwips + this.sectionProperties.docLayer._spaceBetweenParts) * app.twipsToPixels);

			tilePos.pY = tile.coords.part * partHeightPixels + tile.coords.y;
		}

		this.drawTileToCanvas(tile, this.context, tilePos.vX - this.myTopLeft[0], tilePos.vY - this.myTopLeft[1], TileManager.tileSize, TileManager.tileSize);
	}

	// eslint-disable-next-line @typescript-eslint/explicit-module-boundary-types
	public paint (tile: any, ctx: any, async: boolean): void {
		if (this.containerObject.isInZoomAnimation() || this.sectionProperties.tsManager.waitForTiles())
			return;

		if (!ctx)
			ctx = this.sectionProperties.tsManager._paintContext();

		this.containerObject.setPenPosition(this);

		if (ctx.paneBoundsActive === true)
			this.paintWithPanes(tile, ctx, async);
		else
			this.paintSimple(tile, async);
	}

	private forEachTileInView(zoom: number, part: number, mode: number, ctx: any,
		callback: (tile: any, coords: any) => boolean) {
		var tileRanges = ctx.paneBoundsList.map(TileManager.pxBoundsToTileRange, TileManager);

		if (app.file.fileBasedView) {
			const coordList = TileManager.updateFileBasedView(true);

			for (var k: number = 0; k < coordList.length; k++) {
				const tile: Tile = TileManager.get(coordList[k]);
				if (!callback(tile, coordList[k]))
					return;
			}
		}
		else {
			for (var rangeIdx = 0; rangeIdx < tileRanges.length; ++rangeIdx) {
				var tileRange = tileRanges[rangeIdx];
				for (var j = tileRange.min.y; j <= tileRange.max.y; ++j) {
					for (var i: number = tileRange.min.x; i <= tileRange.max.x; ++i) {
						var coords = new TileCoordData(
							i * TileManager.tileSize,
							j * TileManager.tileSize,
							zoom,
							part,
							mode);

						const tile: Tile = TileManager.get(coords);

						if (!callback(tile, coords))
							return;
					}
				}
			}
		}
	}

	// eslint-disable-next-line @typescript-eslint/explicit-module-boundary-types
	public haveAllTilesInView(zoom?: number, part?: number, mode?: number, ctx?: any): boolean {
		zoom = zoom || Math.round(this.map.getZoom());
		part = part || this.sectionProperties.docLayer._selectedPart;
		ctx = ctx || this.sectionProperties.tsManager._paintContext();

		var allTilesFetched = true;
		this.forEachTileInView(zoom, part, mode, ctx, function (tile: any): boolean {
			// Ensure all tile are available.
			if (!tile || tile.needsFetch()) {
				allTilesFetched = false;
				return false; // stop search.
			}
			return true; // continue checking remaining tiles.
		});

		return allTilesFetched;
	}

	private drawPageBackgroundWriter (ctx: any) {
		const viewRectangleTwips = app.activeDocument.activeLayout.viewedRectangle.toArray();
		this.context.fillStyle = this.containerObject.getDocumentBackgroundColor();

		for (let i: number = 0; i < app.file.writer.pageRectangleList.length; i++) {
			const simpleRectangle = new cool.SimpleRectangle(
				app.file.writer.pageRectangleList[i][0],
				app.file.writer.pageRectangleList[i][1],
				app.file.writer.pageRectangleList[i][2],
				app.file.writer.pageRectangleList[i][3]
			);

			if (simpleRectangle.intersectsRectangle(viewRectangleTwips)) {
				this.context.fillRect(
					simpleRectangle.v1X + this.sectionProperties.pageBackgroundInnerMargin,
					simpleRectangle.v1Y + this.sectionProperties.pageBackgroundInnerMargin,
					simpleRectangle.v2X - simpleRectangle.v1X - this.sectionProperties.pageBackgroundInnerMargin,
					simpleRectangle.v3Y - simpleRectangle.v1Y - this.sectionProperties.pageBackgroundInnerMargin
				);
			}
		}
	}

	private drawPageBackgroundFileBasedView (ctx: any) {
		// PDF view supports only same-sized pages for now. So we can use simple math instead of a loop.
		var partHeightPixels: number = Math.round((this.map._docLayer._partHeightTwips + this.map._docLayer._spaceBetweenParts) * app.twipsToPixels);
		var visibleBounds: Array<number> = app.activeDocument.activeLayout.viewedRectangle.pToArray();
		var topVisible: number = Math.floor(visibleBounds[1] / partHeightPixels);
		var bottomVisible: number = Math.ceil((visibleBounds[1] + visibleBounds[3]) / partHeightPixels);

		// Check existence of pages.
		topVisible = topVisible >= 0 ? topVisible : 0;
		bottomVisible = bottomVisible < this.map._docLayer._parts ? bottomVisible : this.map._docLayer._parts - 1;

		if (!isNaN(partHeightPixels) && partHeightPixels > 0) {
			var partHeightPixels: number = Math.round(this.map._docLayer._partHeightTwips * app.twipsToPixels);
			var gap: number = Math.round(this.map._docLayer._spaceBetweenParts * app.twipsToPixels);
			var partWidthPixels: number = Math.round(this.map._docLayer._partWidthTwips * app.twipsToPixels);
			var startY: number = (partHeightPixels + gap) * (topVisible > 0 ? topVisible : 0);
			var rectangle: Array<number>;

			for (var i: number = 0; i <= bottomVisible - topVisible; i++) {
				rectangle = [0, startY, partWidthPixels, partHeightPixels];

				this.context.strokeRect(
					rectangle[0] - ctx.viewBounds.min.x + this.sectionProperties.pageBackgroundInnerMargin,
					rectangle[1] - ctx.viewBounds.min.y + this.sectionProperties.pageBackgroundInnerMargin,
					rectangle[2] - this.sectionProperties.pageBackgroundInnerMargin,
					rectangle[3] - this.sectionProperties.pageBackgroundInnerMargin);

				this.context.fillText(String(i + topVisible + 1),
					Math.round((2 * rectangle[0] + rectangle[2]) * 0.5) - ctx.viewBounds.min.x,
					Math.round((2 * rectangle[1] + rectangle[3]) * 0.5) - ctx.viewBounds.min.y,
					rectangle[2] * 0.4);

				startY += partHeightPixels + gap;
			}
		}
	}

	private drawPageBackgrounds (ctx: any) {
		if (!app.file.fileBasedView && this.map._docLayer._docType !== 'text')
			return;

		if (!this.containerObject.getDocumentAnchorSection())
			return;

		this.context.fillStyle = this.sectionProperties.pageBackgroundTextColor;
		this.context.strokeStyle = this.sectionProperties.pageBackgroundBorderColor;
		this.context.lineWidth = app.roundedDpiScale;

		this.context.font = this.sectionProperties.pageBackgroundFont;

		if (app.file.fileBasedView)
			this.drawPageBackgroundFileBasedView(ctx);
		else if (this.map._docLayer._docType === 'text')
			this.drawPageBackgroundWriter(ctx);
	}

	private drawForViewLayoutMultiPage() {
		const view = app.activeDocument.activeLayout as ViewLayoutMultiPage;

		const visibleCoordList: Array<TileCoordData> = view.getCurrentCoordList();

		for (let i = 0; i < visibleCoordList.length; i++) {
			const tile = TileManager.get(visibleCoordList[i]);

			if (tile && tile.isReadyToDraw()) {
				const tilePos = tile.coords.getPosSimplePoint();

				const layoutRectangle1 = view.documentRectangles[view.getClosestRectangleIndex(tilePos)];
				const layoutRectangle2 = view.documentRectangles[view.getClosestRectangleIndex(cool.SimplePoint.fromCorePixels([tilePos.pX, tilePos.pY + TileManager.tileSize]))];

				if (layoutRectangle1.part === layoutRectangle2.part)
					this.drawTileToCanvas(tile, this.context, tilePos.vX, tilePos.vY, TileManager.tileSize, TileManager.tileSize);
				else {
					// A tile in Writer may intersect 2 pages.
					const height1 = layoutRectangle1.pY2 - tilePos.pY + this.sectionProperties.multiPageViewMagicHeightFix;
					this.drawTileToCanvasCrop(tile, this.context, 0, 0, TileManager.tileSize, height1, tilePos.vX, tilePos.vY, TileManager.tileSize, height1);

					tilePos.pY += TileManager.tileSize;
					const height2 = TileManager.tileSize - height1;
					this.drawTileToCanvasCrop(tile, this.context, 0, height1, TileManager.tileSize, height2, tilePos.vX, tilePos.vY - height2, TileManager.tileSize, height2);
				}
			}
		}
	}

	private drawForViewLayoutCompareChanges() {
		const view = app.activeDocument.activeLayout as ViewLayoutCompareChanges;

		const visibleCoordList: Array<TileCoordData> = view.getCurrentCoordList();

		for (let i = 0; i < visibleCoordList.length; i++) {
			const tile = TileManager.get(visibleCoordList[i]);

			if (tile && tile.isReadyToDraw()) {
				const tilePos = tile.coords.getPosSimplePoint();

				this.drawTileToCanvas(tile, this.context, tilePos.vX, tilePos.vY, TileManager.tileSize, TileManager.tileSize);
			}
		}
	}

	public onDraw (frameCount: number = null, elapsedTime: number = null): void {
		if (this.containerObject.isInZoomAnimation())
			return;

		if (this.containerObject.testing) {
			this.containerObject.createUpdateSingleDivElement(this);
		}

		// Calculate all this here instead of doing it per tile.
		var ctx = this.sectionProperties.tsManager._paintContext();

		if (app.activeDocument && app.activeDocument.activeLayout.type === 'ViewLayoutMultiPage') {
			this.context.translate(-this.myTopLeft[0], -this.myTopLeft[1]);
			this.drawPageBackgrounds(ctx);
			this.drawForViewLayoutMultiPage();
			this.context.translate(this.myTopLeft[0], this.myTopLeft[1]);
			return;
		}
		else if (app.activeDocument.activeLayout.type === 'ViewLayoutCompareChanges') {
			this.context.translate(-this.myTopLeft[0], -this.myTopLeft[1]);
			this.drawForViewLayoutCompareChanges();
			this.context.translate(this.myTopLeft[0], this.myTopLeft[1]);
			return;
		}

		var zoom = Math.round(this.map.getZoom());
		var part = this.sectionProperties.docLayer._selectedPart;
		var mode = app.activeDocument.activeModes[0];

		if (this.sectionProperties.tsManager.waitForTiles()) {
			if (!this.haveAllTilesInView(zoom, part, mode, ctx))
				return;
		} else if (!this.containerObject.isZoomChanged()) {
			// Don't show page border and page numbers (drawn by drawPageBackgrounds) if zoom is changing
			// after a zoom animation.

			// drawPageBackgroundWriter uses absolute view coordinates (v1X, v1Y etc.)
			// but the context is already translated by myTopLeft (by the section container).
			// Compensate for this offset, same as the multipage view path.
			this.context.translate(-this.myTopLeft[0], -this.myTopLeft[1]);
			this.drawPageBackgrounds(ctx);
			this.context.translate(this.myTopLeft[0], this.myTopLeft[1]);
		}

		var doneTiles = new Set();
		this.forEachTileInView(zoom, part, mode, ctx, function (tile: any, coords: TileCoordData): boolean {

			if (doneTiles.has(coords.key()))
				return true;

			// Ensure tile is within document bounds.
			if (tile && TileManager.isValidTile(coords)) {
				if (!this.isJSDOM) { // perf-test code
					if (tile.isReadyToDraw() || this.map._debug.tileOverlaysOn) { // Ensure tile is loaded
						this.paint(tile, ctx, false /* async? */);
					}
				}
			}
			doneTiles.add(coords.key());
			return true; // continue with remaining tiles.
		}.bind(this));
	}

	public onClick(point: cool.SimplePoint, e: MouseEvent): void {
		// Slides pane is not focusable, we are using a variable to follow its focused state.
		// Until the pane is focusable, we will need to keep below check here.
		if (this.map._docLayer._docType === 'presentation' || this.map._docLayer._docType === 'drawing')
			this.map._docLayer._preview.partsFocused = false; // Parts (slide preview pane) is no longer focused, we need to set this here to avoid unwanted behavior.
	}

	// Return the fraction of intersection area with area1.
	// eslint-disable-next-line @typescript-eslint/explicit-module-boundary-types
	static getTileIntersectionAreaFraction(tileBounds: any, viewBounds: any): number {

		var size = tileBounds.getSize();
		if (size.x <= 0 || size.y <= 0)
			return 0;

		var intersection = new cool.Bounds(
			new cool.Point(
				Math.max(tileBounds.min.x, viewBounds.min.x),
				Math.max(tileBounds.min.y, viewBounds.min.y)),
			new cool.Point(
				Math.min(tileBounds.max.x, viewBounds.max.x),
				Math.min(tileBounds.max.y, viewBounds.max.y))
		);

		var interSize = intersection.getSize();
		return Math.max(0, interSize.x) * Math.max(0, interSize.y) / (size.x * size.y);
	}

	private forEachTileInArea(area: any, zoom: number, part: number, mode: number, ctx: any,
		callback: (tile: any, coords: TileCoordData, section: TilesSection) => boolean) {

		if (app.file.fileBasedView) {
			const coordList = TileManager.updateFileBasedView(true, area, zoom);

			for (var k: number = 0; k < coordList.length; k++) {
				const coords = coordList[k];
				const tile: Tile = TileManager.get(coords);
				if (tile)
					callback(tile, coords, this);
			}

			return;
		}

		var tileRange = TileManager.pxBoundsToTileRange(area);

		for (var j = tileRange.min.y; j <= tileRange.max.y; ++j) {
			for (var i = tileRange.min.x; i <= tileRange.max.x; ++i) {
				const coords = new TileCoordData(
					i * TileManager.tileSize,
					j * TileManager.tileSize,
					zoom,
					part,
					mode);

				const tile: Tile = TileManager.get(coords);
				if (tile)
					callback(tile, coords, this);
			}
		}
	}

	/**
	 * Used for rendering a zoom-out frame, to determine which zoom level tiles
	 * to use for rendering.
	 *
	 * @param area specifies the document area in core-pixels at the current
	 * zoom level.
	 *
	 * @returns the zoom-level with maximum tile content.
	 */
	private zoomLevelWithMaxContentInArea(area: any,
		areaZoom: number, part: number, mode: number, ctx: any): number {

		var frameScale = this.sectionProperties.tsManager._zoomFrameScale;
		var docLayer = this.sectionProperties.docLayer;
		var targetZoom = Math.round(this.map.getScaleZoom(frameScale, areaZoom));
		var bestZoomLevel = targetZoom;
		var availAreaScoreAtBestZL = -Infinity; // Higher the better.
		var area = area.clone();
		if (area.min.x < 0)
			area.min.x = 0;
		if (area.min.y < 0)
			area.min.y = 0;

		var minZoom = <number> this.map.options.minZoom;
		var maxZoom = <number> this.map.options.maxZoom;
		for (var zoom = minZoom; zoom <= maxZoom; ++zoom) {
			var availAreaScore = 0; // Higher the better.
			var hasTiles = false;

			// To scale up missing-area scores to maxZoom as we need an
			// good resolution integer score at the end.
			var dimensionCorrection = this.map.zoomToFactor(maxZoom - zoom + this.map.options.zoom);

			// Compute area for zoom-level 'zoom'.
			var areaAtZoom = this.scaleBoundsForZoom(area, zoom, areaZoom);
			//console.log('DEBUG:: areaAtZoom = ' + areaAtZoom);
			var relScale = this.map.getZoomScale(zoom, areaZoom);

			this.forEachTileInArea(areaAtZoom, zoom, part, mode, ctx, function(tile, coords, section) {
				if (tile && tile.image) {
					var tilePos = coords.getPos();

					if (app.file.fileBasedView) {
						var ratio = TileManager.tileSize * relScale / app.tile.size.y;
						var partHeightPixels = Math.round((docLayer._partHeightTwips + docLayer._spaceBetweenParts) * ratio);
						tilePos.y = coords.part * partHeightPixels + tilePos.y;
					}

					var tileBounds = new cool.Bounds(tilePos, tilePos.add(new cool.Point(TileManager.tileSize, TileManager.tileSize)));
					var interFrac = TilesSection.getTileIntersectionAreaFraction(tileBounds, areaAtZoom);

					// Add to score how much of tile area is available.
					availAreaScore += interFrac;
					if (!hasTiles)
						hasTiles = true;
				}

				return true;
			});

			// Scale up with a correction factor to make area scores comparable b/w zoom levels.
			availAreaScore = hasTiles ? Math.round(availAreaScore
				* dimensionCorrection /* width */
				* dimensionCorrection /* height */
				/ 10 /* resolution control */) : -Infinity;

			// Accept this zoom if it has a lower missing-area score
			// In case of a tie we prefer tiles from a zoom level closer to targetZoom.
			if (availAreaScore > availAreaScoreAtBestZL ||
				(availAreaScore == availAreaScoreAtBestZL && Math.abs(targetZoom - bestZoomLevel) > Math.abs(targetZoom - zoom))) {
				availAreaScoreAtBestZL = availAreaScore;
				bestZoomLevel = zoom;
			}
		}

		return bestZoomLevel;
	}

	// eslint-disable-next-line @typescript-eslint/explicit-module-boundary-types
	public drawTileToCanvas(tile: any, canvas: CanvasRenderingContext2D,
							dx: number, dy: number, dWidth: number, dHeight: number): void
	{
		this.drawTileToCanvasCrop(tile, canvas,
								  0, 0, TileManager.tileSize, TileManager.tileSize,
								  dx, dy, dWidth, dHeight);
	}

	private drawDebugHistogram(canvas: CanvasRenderingContext2D, x: number, y: number, value: number, offset: number)
	{
			const tSize = 256;
			const deltaSize = 4;
			var maxDeltas = (tSize - 16) / deltaSize;

			// offset vertically down to 'offset'
			const yoff = Math.floor(offset / maxDeltas);
			y += yoff * deltaSize;
			offset -= yoff * maxDeltas;

			var firstRowFill = Math.min(value, maxDeltas - offset);

			// fill first row from offset
			if (firstRowFill > 0)
				canvas.fillRect(x + offset * deltaSize, y, firstRowFill * deltaSize, deltaSize);

			// render the rest:
			value = value - firstRowFill;

			// central rectangle
			var rowBlock = Math.floor(value / maxDeltas);

			if (rowBlock > 0)
				canvas.fillRect(x, y + deltaSize, maxDeltas * deltaSize, rowBlock * deltaSize);

			// Fill last row
			var rowLeft = value % maxDeltas;
			if (rowLeft > 0)
				canvas.fillRect(x, y + rowBlock * deltaSize + deltaSize, rowLeft * deltaSize, deltaSize);
	}

	// eslint-disable-next-line @typescript-eslint/explicit-module-boundary-types
	public drawTileToCanvasCrop(tile: any, canvas: CanvasRenderingContext2D,
								sx: number, sy: number, sWidth: number, sHeight: number,
								dx: number, dy: number, dWidth: number, dHeight: number): void
	{
		TileManager.touchImage(tile);

		/* if (!(tile.wireId % 4)) // great for debugging tile grid alignment.
				canvas.drawImage(this.checkpattern, sx, sy, sWidth, sHeight, dx, dy, dWidth, dHeight);
		else */
		if (tile.image)
			canvas.drawImage(tile.image, sx, sy, sWidth, sHeight, dx, dy, dWidth, dHeight);

		if (this.map._debug.tileOverlaysOn)
		{
			this.beforeDraw(canvas);

			// clipping push - normally we avoid clipping for perf.
			canvas.save();
			const clipRegion = new Path2D();
			clipRegion.rect(dx, dy, dWidth, dHeight);
			canvas.clip(clipRegion);

			// want to render our bits 'inside' the tile - but we may have only part of it.
			// so offset our rendering and rely on clipping to help.
			const ox = -sx;
			const oy = -sy;
			const tSize = 256;

			// blue boundary line on tiles
			canvas.lineWidth = 1;
			canvas.strokeStyle = 'rgba(0, 0, 255, 0.8)';
			canvas.beginPath();
			canvas.moveTo(ox + dx + 0.5, oy + dy + 0.5);
			canvas.lineTo(ox + dx + 0.5, oy + dy + tSize + 0.5);
			canvas.lineTo(ox + dx + tSize + 0.5, oy + dy + tSize + 0.5);
			canvas.lineTo(ox + dx + tSize + 0.5, oy + dy + 0.5);
			canvas.lineTo(ox + dx + 0.5, oy + dy + 0.5);
			canvas.stroke();

			// state of the tile
			if (!tile.hasContent())
				canvas.fillStyle = 'rgba(255, 0, 0, 0.8)';   // red
			else if (tile.needsFetch())
				canvas.fillStyle = 'rgba(255, 255, 0, 0.8)'; // yellow
			else if (!tile.isReady())
				canvas.fillStyle = 'rgba(0, 160, 0, 0.8)'; // dark green
			else // present
				canvas.fillStyle = 'rgba(0, 255, 0, 0.5)';   // green
			canvas.fillRect(ox + dx + 1.5, oy + dy + 1.5, 12, 12);

			// deltas graph
			if (tile.deltaCount)
			{
				// blue/grey deltas
				canvas.fillStyle = 'rgba(0, 0, 256, 0.3)';
				this.drawDebugHistogram(canvas, ox + dx + 1.5 + 14, oy + dy + 1.5, tile.deltaCount, 0);
				// yellow/grey deltas
				canvas.fillStyle = 'rgba(256, 256, 0, 0.3)';
				this.drawDebugHistogram(canvas, ox + dx + 1.5 + 14, oy + dy + 1.5, tile.updateCount, tile.deltaCount);
			}

			// Metrics on-top of the tile:
			var lines = [
				'wireId: ' + tile.wireId,
				'invalidFrom: ' + tile.invalidFrom,
				'nviewid: ' + tile.viewId,
				'invalidates: ' + tile.invalidateCount,
				'tile: ' + tile.loadCount + ' \u0394: ' + tile.deltaCount + ' upd: ' + tile.updateCount,
				'misses: ' + tile.missingContent + ' gce: ' + tile.gcErrors,
				'dlta size/kB: ' + ((tile.rawDeltas ? tile.rawDeltas.length : 0)/1024).toFixed(2)
			];

			// FIXME: generate metrics of how long a tile has been visible & invalid for.
			//			if (tile._debugTime && tile._debugTime.date !== 0)
			//					lines.push(this.map._debug.updateTimeArray(tile._debugTime, +new Date() - tile._debugTime.date));

			const startY = tSize - 12 * lines.length;

			// background
			canvas.fillStyle = 'rgba(220, 220, 220, 0.5)'; // greyish
			canvas.fillRect(ox + dx + 1.5, oy + dy + startY - 12.0, 100, 12 * lines.length + 8.0);

			canvas.font = '12px sans';
			canvas.fillStyle = 'rgba(0, 0, 0, 1.0)';   // black
			canvas.textAlign = 'left';
			for (var i = 0 ; i < lines.length; ++i)
					canvas.fillText(lines[i], ox + dx + 5.5, oy + dy + startY + i*12);

			canvas.restore();
			this.afterDraw(canvas);
		}
	}

	// Called by tsManager to draw a zoom animation frame.
	// eslint-disable-next-line @typescript-eslint/explicit-module-boundary-types
	public drawZoomFrame(ctx?: any): void {
		var tsManager = this.sectionProperties.tsManager;
		if (!tsManager._inZoomAnim)
			return;

		var scale = tsManager._zoomFrameScale;
		if (!scale || !tsManager._newCenter)
			return;

		ctx = ctx || this.sectionProperties.tsManager._paintContext();
		var docLayer = this.sectionProperties.docLayer;
		var zoom = Math.round(this.map.getZoom());
		var part = docLayer._selectedPart;
		var mode = app.activeDocument.activeModes[0];
		var splitPos = ctx.splitPos;

		this.containerObject.setPenPosition(this);
		var viewSize = ctx.viewBounds.getSize();
		// clear the document area first.
		this.context.fillStyle = this.containerObject.getClearColor();
		this.context.fillRect(0, 0, viewSize.x, viewSize.y);

		var paneBoundsList = ctx.paneBoundsList;

		let maxXBound = 0;
		let maxYBound = 0;

		for (const paneBounds of paneBoundsList) {
			maxXBound = Math.max(maxXBound, paneBounds.min.x);
			maxYBound = Math.max(maxYBound, paneBounds.min.y);
		}

		for (var k = 0; k < paneBoundsList.length ; ++k) {
			var paneBounds = paneBoundsList[k];
			var paneSize = paneBounds.getSize();

			var destPos = new cool.Point(0, 0);
			var docAreaSize = paneSize.divideBy(scale);

			let freezeX: boolean;
			let freezeY: boolean;

			if (paneBounds.min.x === 0 && maxXBound !== 0) {
				// There is another pane in the X direction and we are at 0, so we are fixed in X
				docAreaSize.x = paneSize.x;
				freezeX = true;
			} else {
				// Pane is free to move in X direction.
				destPos.x = splitPos.x;
				docAreaSize.x += splitPos.x / scale;
				freezeX = false;
			}

			if (paneBounds.min.y === 0 && maxYBound !== 0) {
				// There is another pane in the Y direction and we are at 0, so we are fixed in Y
				docAreaSize.y = paneSize.y;
				freezeY = true;
			} else {
				// Pane is free to move in Y direction.
				destPos.y = splitPos.y;
				docAreaSize.y += splitPos.y / scale;
				freezeY = false;
			}

			// Calculate top-left in doc core-pixels for the frame.
			var docPos = tsManager._getZoomDocPos(
				tsManager._newCenter,
				tsManager._layer._pinchStartCenter,
				paneBounds,
				{ freezeX, freezeY },
				splitPos,
				scale,
				false /* findFreePaneCenter? */
			);

			if (!freezeX) {
				tsManager._zoomAtDocEdgeX = docPos.topLeft.x == splitPos.x;
			}

			if (!freezeY) {
				tsManager._zoomAtDocEdgeY = docPos.topLeft.y == splitPos.y;
			}

			var docRange = new cool.Bounds(docPos.topLeft, docPos.topLeft.add(docAreaSize));
			if (tsManager._calcGridSection) {
				tsManager._calcGridSection.onDrawArea(docRange, docRange.min.subtract(destPos), this.context);
			}
			var canvasContext = this.context;

			var bestZoomSrc = zoom;
			var sheetGeometry = docLayer.sheetGeometry;
			var useSheetGeometry = false;
			if (scale < 1.0) {
				useSheetGeometry = !!sheetGeometry;
				bestZoomSrc = this.zoomLevelWithMaxContentInArea(docRange, zoom, part, mode, ctx);
			}

			var docRangeScaled = (bestZoomSrc == zoom) ? docRange : this.scaleBoundsForZoom(docRange, bestZoomSrc, zoom);
			var destPosScaled = (bestZoomSrc == zoom) ? destPos : this.scalePosForZoom(destPos, bestZoomSrc, zoom);
			var relScale = (bestZoomSrc == zoom) ? 1 : this.map.getZoomScale(bestZoomSrc, zoom);

			this.beforeDraw(canvasContext);
			this.forEachTileInArea(docRangeScaled, bestZoomSrc, part, mode, ctx, function (tile, coords, section): boolean {
				if (!tile || !tile.isReadyToDraw() || !TileManager.isValidTile(coords))
					return false;

				var tileCoords = tile.coords.getPos();
				if (app.file.fileBasedView) {
					var ratio = TileManager.tileSize * relScale / app.tile.size.y;
					var partHeightPixels = Math.round((docLayer._partHeightTwips + docLayer._spaceBetweenParts) * ratio);
					tileCoords.y = tile.coords.part * partHeightPixels + tileCoords.y;
				}
				var tileBounds = new cool.Bounds(tileCoords, tileCoords.add(new cool.Point(TileManager.tileSize, TileManager.tileSize)));

				var crop = new cool.Bounds(tileBounds.min, tileBounds.max);
				crop.min.x = Math.max(docRangeScaled.min.x, tileBounds.min.x);
				crop.min.y = Math.max(docRangeScaled.min.y, tileBounds.min.y);
				crop.max.x = Math.min(docRangeScaled.max.x, tileBounds.max.x);
				crop.max.y = Math.min(docRangeScaled.max.y, tileBounds.max.y);

				var cropWidth = crop.max.x - crop.min.x;
				var cropHeight = crop.max.y - crop.min.y;

				var tileOffset = crop.min.subtract(tileBounds.min);
				var paneOffset = crop.min.subtract(docRangeScaled.min.subtract(destPosScaled));
				if (cropWidth && cropHeight) {
						section.drawTileToCanvasCrop(
								tile, canvasContext,
								tileOffset.x, tileOffset.y, // source x, y
								cropWidth, cropHeight, // source size
								// Destination x, y, w, h (In non-Chrome browsers it leaves lines without the 0.5 correction).
								Math.floor(paneOffset.x / relScale * scale) + 0.5, // Destination x
								Math.floor(paneOffset.y / relScale * scale) + 0.5, // Destination y
								Math.floor((cropWidth / relScale) * scale) + 1.5,    // Destination width
								Math.floor((cropHeight / relScale) * scale) + 1.5);    // Destination height
				}

				return true;
			}); // end of forEachTileInArea call.
			this.afterDraw(canvasContext);

		} // End of pane bounds list loop.

	}

	private scalePosForZoom(pos: any, toZoom: number, fromZoom: number): any {
		var docLayer = this.sectionProperties.docLayer;
		var convScale = this.map.getZoomScale(toZoom, fromZoom);

		if (docLayer.sheetGeometry) {
			var toScale = convScale * TileManager.tileSize * 15.0 / app.tile.size.x;
			toScale = TileManager.tileSize * 15.0 / Math.round(15.0 * TileManager.tileSize / toScale);
			var posScaled = docLayer.sheetGeometry.getCorePixelsAtZoom(pos, toScale);
			return posScaled;
		}

		return pos.multiplyBy(convScale);
	}

	private scaleBoundsForZoom(corePxBounds: any, toZoom: number, fromZoom: number) {
		var docLayer = this.sectionProperties.docLayer;
		var convScale = this.map.getZoomScale(toZoom, fromZoom);

		if (docLayer.sheetGeometry) {

			var topLeft = this.scalePosForZoom(corePxBounds.min, toZoom, fromZoom);
			var size = corePxBounds.getSize().multiplyBy(convScale);
			return new cool.Bounds(
				topLeft,
				topLeft.add(size)
			);
		}

		return new cool.Bounds(
			corePxBounds.min.multiplyBy(convScale),
			corePxBounds.max.multiplyBy(convScale)
		);
	}

	onNewDocumentTopLeft(): void {
		if (app.map._textInput && app.map._textInput._isDebugOn)
			app.map._textInput.update();
	}
}

}
