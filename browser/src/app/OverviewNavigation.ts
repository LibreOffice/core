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
 * OverviewNavigation - positions the view on the overview page so that one slide preview
 * is centered in the visible area and fills most of it.
 *
 * The preview rectangles live in app.impress.previewRects, in canvas-page twips. The
 * positioning happens in two synchronous steps: a non-animated setZoom sized from the
 * wanted fill ratio, then a scroll that puts the preview center on the viewport center.
 */

class OverviewNavigation {
	// The fraction of the tighter viewport axis a preview takes after a "Fit slide to
	// view" command.
	public static readonly fitFillRatio = 0.9;

	// The fraction used when landing on the overview page from a zoom-out gesture. A bit
	// looser than the fit ratio, so the neighbouring previews peek into the view.
	private static readonly landingFillRatio = 0.85;

	// The part a pending landing request wants centered, or -1 when there is none.
	private static pendingLandingPart = -1;

	// Zooms and scrolls so the preview of the given part is centered in the visible area
	// and its larger side takes fillRatio of the tighter viewport axis, clamped to the
	// allowed zoom range. Returns false without touching the view when the preview
	// rectangles are missing or the part has no preview.
	public static centerPreviewOnViewport(
		part: number,
		fillRatio: number,
	): boolean {
		const previews = app.impress.previewRects;
		if (!previews) return false;
		const preview = previews.find((entry) => entry.part === part);
		if (!preview) return false;
		const anchor = app.sectionContainer.getDocumentAnchorSection();
		if (!anchor) return false;

		const rect = preview.rectangle;
		if (rect.pWidth <= 0 || rect.pHeight <= 0) return false;
		if (anchor.size[0] <= 0 || anchor.size[1] <= 0) return false;

		// The wanted scale factor relative to the current zoom; getScaleZoom turns it
		// into a zoom level. Flooring keeps the preview at or below the wanted fill.
		const scale =
			fillRatio *
			Math.min(anchor.size[0] / rect.pWidth, anchor.size[1] / rect.pHeight);
		Util.ensureValue(app.activeDocument);
		let zoom = Math.floor(app.activeDocument.getScaleZoom(scale));
		zoom = Math.max(app.map.getMinZoom(), Math.min(app.map.getMaxZoom(), zoom));
		app.zoomControl.zoomTo(zoom, undefined, false);

		// The non-animated zoomTo returns with viewedRectangle and the twips
		// multipliers already updated, so the centering scroll can use them directly.
		const viewed = app.activeDocument.activeLayout.viewedRectangle;
		const center = rect.center;
		const scrollPoint = new cool.SimplePoint(
			center[0] - viewed.width / 2,
			center[1] - viewed.height / 2,
		);
		app.activeDocument.activeLayout.scrollTo(scrollPoint.pX, scrollPoint.pY);
		return true;
	}

	// True when the overview page is on screen: the document is a presentation with an
	// overview page, laid out in the canvas-page layout rather than fileBasedView, and
	// the overview slide (part 0) is the selected part.
	public static isOverviewActive(): boolean {
		if (!app.map || !app.map._docLayer) return false;
		if (app.map._docLayer._docType !== 'presentation') return false;
		if (!app.impress.hasOverviewPage || app.file.fileBasedView) return false;
		return app.map._docLayer._selectedPart === 0;
	}

	// The part whose preview center sits nearest the viewport center, considering only
	// previews that intersect the visible area. Returns -1 when the view is not on the
	// overview page or no preview is in sight.
	public static previewNearestViewportCenter(): number {
		if (!OverviewNavigation.isOverviewActive()) return -1;
		const previews = app.impress.previewRects;
		if (!previews) return -1;

		Util.ensureValue(app.activeDocument);
		const viewed = app.activeDocument.activeLayout.viewedRectangle;
		const viewedCenter = viewed.center;
		let nearestPart = -1;
		let nearestDistance = Infinity;
		for (const preview of previews) {
			if (!viewed.intersectsRectangle(preview.rectangle.toArray())) continue;
			const center = preview.rectangle.center;
			const distance = Math.hypot(
				center[0] - viewedCenter[0],
				center[1] - viewedCenter[1],
			);
			if (distance < nearestDistance) {
				nearestDistance = distance;
				nearestPart = preview.part;
			}
		}
		return nearestPart;
	}

	// Fits the preview nearest the viewport center. Returns false when no preview is in
	// sight.
	public static fitNearestPreview(fillRatio: number): boolean {
		const part = OverviewNavigation.previewNearestViewportCenter();
		if (part < 1) return false;
		return OverviewNavigation.centerPreviewOnViewport(part, fillRatio);
	}

	// Remembers the part to center once the next switch to the overview page has
	// positioned the view. Parts below 1 clear the request.
	public static requestLanding(part: number): void {
		OverviewNavigation.pendingLandingPart = part >= 1 ? part : -1;
	}

	// Performs and clears the pending landing request. Returns true when a preview was
	// centered, false when no request was pending or the preview was not available.
	public static consumeLandingRequest(): boolean {
		const part = OverviewNavigation.pendingLandingPart;
		OverviewNavigation.pendingLandingPart = -1;
		if (part < 1) return false;
		return OverviewNavigation.centerPreviewOnViewport(
			part,
			OverviewNavigation.landingFillRatio,
		);
	}
}
