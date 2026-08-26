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

/*
 * ZoomControl - owns zooming for the document, replacing the leaflet map's
 * zoomIn/zoomOut/setZoom.
 *
 * It is a window section (covers the whole canvas) modelled on ScrollSection.
 * Zoom gestures are only acted on when they fall inside the document-anchor
 * (tiles) area. The animation runs through the section container's own loop:
 * `zoomTo` starts it, `onAnimate` advances one frame, `onAnimationEnded`
 * commits the final zoom. All the per-frame and final data updates (scale,
 * viewedRectangle, tile requests) are delegated to the active layout so this
 * class stays layout-agnostic.
 *
 * Programmatic zooms (menu / keyboard / status bar) animate through the
 * container's loop; pinch and Ctrl+wheel commit directly (pointer-anchored).
 */

class ZoomControl extends CanvasSectionObject {
	processingOrder: number = app.CSections.ZoomControl.processingOrder;
	drawingOrder: number = app.CSections.ZoomControl.drawingOrder;
	zIndex: number = app.CSections.ZoomControl.zIndex;
	windowSection: boolean = true; // Covers the whole canvas, like ScrollSection.

	// Animated-zoom state, valid between beginZoom and onAnimationEnded.
	private zoomDurationMs: number = 250;
	private zoomTarget: number | null = null;
	private zoomAnchor: cool.SimplePoint | null = null;
	private zoomEndScale: number = 1; // Frame scale at the end (relative to start).

	constructor() {
		super(app.CSections.ZoomControl.name);
	}

	private get layout(): any {
		return app.activeDocument && app.activeDocument.activeLayout;
	}

	// True when a canvas (core-pixel) point falls inside the document-anchor
	// (tiles) section, i.e. the area zoom gestures act on.
	private inDocumentArea(point: cool.SimplePoint): boolean {
		const anchor = app.sectionContainer.getDocumentAnchorSection();
		if (!anchor) return false;
		const x = point.pX;
		const y = point.pY;
		return (
			x >= anchor.myTopLeft[0] &&
			x <= anchor.myTopLeft[0] + anchor.size[0] &&
			y >= anchor.myTopLeft[1] &&
			y <= anchor.myTopLeft[1] + anchor.size[1]
		);
	}

	// Public entry point that replaces map.setZoom/zoomIn/zoomOut for the
	// document. Zooms to `targetZoom`, centring on `anchorTwips` (a
	// document-space point in twips; defaults to the current view anchor).
	public zoomTo(
		targetZoom: number,
		anchorTwips?: cool.SimplePoint,
		animate: boolean = true,
	): void {
		const layout = this.layout;
		if (!layout) return;

		// A request past the zoom range switches to or from the Overview slide
		// instead of zooming. It needs the unclamped request, so ask before
		// limiting.
		if (OverviewFade.handleZoomBeyondLimit(targetZoom)) return;

		const target = app.map._limitZoom(targetZoom);
		if (target === app.map.getZoom()) return;

		// Other views' cell cursor name popups are placed for the current scale.
		if (app.map._docLayer._docType === 'spreadsheet')
			OtherViewCellCursorSection.closePopups();

		// Animate only the centre-anchored (programmatic: menu / keyboard /
		// status bar) case. There the zoom pivots on the view centre and the
		// commit re-centres on the same point, so there is no jump between the
		// last frame and the commit. Pointer-anchored zooms (pinch, wheel)
		// commit directly.
		if (animate && !anchorTwips && !window.L.Browser.cypressTest) {
			this.animateZoom(target);
			return;
		}

		// When no explicit anchor is given (programmatic zoom that skipped the
		// animation above, e.g. under cypress), pivot on the current view centre,
		// captured here while the viewed rectangle still holds the old scale.
		layout.applyZoom(target, anchorTwips || layout.zoomAnchorPoint());
	}

	// Animate a centre-pivoted zoom through the section container's own
	// animation loop (startAnimating -> onAnimate -> onDraw ->
	// onAnimationEnded), not the tile painter's requestAnimationFrame. The
	// layout switches the tiles section into zoom-frame drawing; each frame we
	// hand it the interpolated scale, and at the end it commits map-free. The
	// pivot is the current view centre from the viewed rectangle (not the map
	// centre, which can be stale after a scroll).
	private animateZoom(target: number): void {
		const layout = this.layout;
		const anchor = layout.zoomAnchorPoint();

		// Fall back to a direct commit if we cannot animate (another section
		// is already animating, or we are in a cypress test).
		if (
			window.L.Browser.cypressTest ||
			app.sectionContainer.getAnimatingSectionName()
		) {
			layout.applyZoom(target, anchor);
			return;
		}

		this.zoomTarget = target;
		this.zoomAnchor = anchor;
		this.zoomEndScale = app.activeDocument.getZoomScale(
			target,
			app.map.getZoom(),
		);

		layout.beginZoom(anchor);
		this.startAnimating({ duration: this.zoomDurationMs });
	}

	// One animation frame: interpolate the zoom-frame scale from 1 (start) to
	// zoomEndScale (target) and hand it to the layout, which the tiles section
	// paints as this frame is drawn.
	public onAnimate(frameCount: number, elapsedTime: number): void {
		const p =
			this.zoomDurationMs > 0
				? Math.min(1, elapsedTime / this.zoomDurationMs)
				: 1;
		this.layout.stepZoom(1 + (this.zoomEndScale - 1) * p);
	}

	// Commit the final zoom when the animation finishes.
	public onAnimationEnded(frameCount: number, elapsedTime: number): void {
		if (this.zoomTarget !== null && this.zoomAnchor) {
			this.layout.endZoom(this.zoomTarget, this.zoomAnchor);
		}
		this.zoomTarget = null;
		this.zoomAnchor = null;
	}

	// Ctrl+wheel zoom. ScrollSection routes the Ctrl+wheel here for Calc
	// (plain wheel stays scrolling). One notch = one zoom level, pivoting on
	// the pointer; committed directly (no animation) so notches feel snappy.
	public onMouseWheel(
		point: cool.SimplePoint,
		delta: Array<number>,
		e: WheelEvent,
	): void {
		if (!e.ctrlKey || !this.layout) return;
		if (!this.inDocumentArea(point)) return;

		// Wheel up (deltaY < 0) zooms in.
		const step = e.deltaY < 0 ? 1 : -1;
		const anchor = this.layout.canvasToDocumentPoint(point);
		this.zoomTo(app.map.getZoom() + step, anchor, false);
	}

	// ---- Pinch zoom -----------------------------------------------------
	// Distance between the two fingers when the gesture started, the
	// document-space anchor (twips) under the pinch centre, and the running
	// requested zoom (unclamped; zoomTo limits it). The zoom is committed when
	// the gesture ends.
	private pinchStartDistance: number | null = null;
	private pinchStartZoom: number | null = null;
	private pinchAnchor: cool.SimplePoint | null = null;
	private pinchTargetZoom: number | null = null;

	public onMultiTouchStart(e: TouchEvent): void {
		this.pinchStartDistance = null;
		this.pinchStartZoom = app.map.getZoom();
		this.pinchAnchor = null;
		this.pinchTargetZoom = null;
	}

	public onMultiTouchMove(
		point: cool.SimplePoint,
		distance: number,
		e: TouchEvent,
	): void {
		if (!this.layout) return;
		if (!this.inDocumentArea(point)) return;

		// First move fixes the start distance and the anchor under the pinch.
		if (this.pinchStartDistance === null) {
			this.pinchStartDistance = distance;
			this.pinchAnchor = this.layout.canvasToDocumentPoint(point);
			return;
		}
		if (!distance || !this.pinchStartDistance) return;

		const ratio = distance / this.pinchStartDistance;
		this.pinchTargetZoom = app.activeDocument.getScaleZoom(
			ratio,
			this.pinchStartZoom,
		);
	}

	public onMultiTouchEnd(e: TouchEvent): void {
		if (this.pinchTargetZoom !== null && this.pinchAnchor) {
			const delta = this.pinchTargetZoom - this.pinchStartZoom;
			const finalZoom =
				delta > 0
					? Math.ceil(this.pinchTargetZoom)
					: Math.floor(this.pinchTargetZoom);
			this.zoomTo(finalZoom, this.pinchAnchor, false);
		}
		this.pinchStartDistance = null;
		this.pinchTargetZoom = null;
		this.pinchAnchor = null;
	}
}
