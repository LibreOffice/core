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

		// Start of the scrollable area in canvas coords (after row/col
		// headers and any frozen/split panes).
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

		// Keep the leaflet map in sync so map-dependent features (headers,
		// cursor visibility, context menus, freeze-pane splitters) keep
		// working. A follow-up will remove this leaflet dependency entirely.
		(app.map as any).panBy(
			new cool.Point(
				(newX - prevX) / app.dpiScale,
				(newY - prevY) / app.dpiScale,
			),
			{ animate: false },
		);

		this.refreshCurrentCoordList();
		this.sendClientVisibleArea();
	}

	// pX, pY are absolute view-space positions in canvas (core) pixels.
	public override scrollTo(pX: number, pY: number): void {
		const deltaX = pX - this._viewedRectangle.pX1;
		const deltaY = pY - this._viewedRectangle.pY1;
		if (deltaX !== 0 || deltaY !== 0) this.scroll(deltaX, deltaY);
	}
}
