// @ts-strict-ignore
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
 * VectorContentSection - Canvas section that draws the main view from
 * vector primitives, for Impress and Draw. The part data, bitmap cache
 * and renderer are shared through the RenderManager facade, so each part is
 * fetched and decoded only once.
 *
 * The section is bound to the bitmap tiles section and drawn right after
 * it. The background primitive is opaque, so the vector output fully
 * covers the tiles area.
 */

/* global app */

namespace cool {
	export class VectorContentSection extends CanvasSectionObject {
		processingOrder: number = app.CSections.VectorContent.processingOrder;
		drawingOrder: number = app.CSections.VectorContent.drawingOrder;
		zIndex: number = app.CSections.VectorContent.zIndex;
		boundToSection: string = 'tiles';
		interactable: boolean = false;

		private _docLayer: any;

		// Holds the rendered pixels of the current frame. Other sections
		// trigger frequent redraws, so a redraw reuses these pixels when the
		// part, scroll and zoom are unchanged.
		private _offscreen: HTMLCanvasElement = null;
		private _offscreenCtx: CanvasRenderingContext2D = null;
		private _lastRenderKey: string = '';
		// Part whose pixels are in the offscreen canvas, so a blit reuses
		// them only for that same part.
		private _offscreenPart: number = -1;

		constructor() {
			super(app.CSections.VectorContent.name);

			// Redraw when a part's data arrives, a deferred bitmap decodes, or
			// the part is invalidated. Drop the cache key so the next draw
			// re-renders the part.
			RenderManager.onVectorChanged(() => {
				this._lastRenderKey = '';
				if (app.sectionContainer) app.sectionContainer.requestReDraw();
			});
		}

		onInitialize(): void {
			this._docLayer = app.map._docLayer;
		}

		private _ensureOffscreen(width: number, height: number): void {
			if (!this._offscreen) {
				this._offscreen = document.createElement('canvas');
				this._offscreenCtx = this._offscreen.getContext('2d');
			}
			if (
				this._offscreen.width !== width ||
				this._offscreen.height !== height
			) {
				this._offscreen.width = width;
				this._offscreen.height = height;
			}
		}

		onDraw(): void {
			if (this.containerObject.isInZoomAnimation()) return;
			if (!this._docLayer) return;

			if (app.file.fileBasedView) {
				this._drawFileBasedView();
				return;
			}

			const part = this._docLayer._selectedPart;
			const cached = RenderManager.requestPart(part);
			if (!cached) {
				// Data not ready. Blit the prior frame only when it holds
				// the part we want, so the view keeps the current part
				// until its fresh data arrives.
				if (this._offscreen && this._offscreenPart === part) {
					this.context.drawImage(this._offscreen, 0, 0);
				}
				return;
			}

			const scale = app.twipsToPixels;
			const viewedRectangle = app.activeDocument.activeLayout.viewedRectangle;
			// The section container has translated the context to the
			// section top-left, which equals the document anchor. So the
			// scroll offset alone maps the part origin into place.
			const offsetX = -viewedRectangle.pX1;
			const offsetY = -viewedRectangle.pY1;

			const w = this.context.canvas.width;
			const h = this.context.canvas.height;
			this._ensureOffscreen(w, h);

			const renderKey =
				part + ':' + scale + ':' + offsetX + ':' + offsetY + ':' + w + ':' + h;
			if (renderKey !== this._lastRenderKey) {
				this._offscreenCtx.clearRect(0, 0, w, h);
				this._offscreenCtx.save();
				this._offscreenCtx.translate(offsetX, offsetY);
				this._offscreenCtx.scale(scale, scale);
				RenderManager.renderInto(this._offscreenCtx, cached);
				this._offscreenCtx.restore();
				this._lastRenderKey = renderKey;
				this._offscreenPart = part;
			}
			this.context.drawImage(this._offscreen, 0, 0);
		}

		private _drawFileBasedView(): void {
			const docLayer = this._docLayer;
			const partHeightPixels = Math.round(
				(docLayer._partHeightTwips + docLayer._spaceBetweenParts) *
					app.twipsToPixels,
			);
			if (partHeightPixels <= 0) return;

			const viewedRectangle = app.activeDocument.activeLayout.viewedRectangle;
			let topVisible = Math.floor(viewedRectangle.pY1 / partHeightPixels);
			let bottomVisible = Math.ceil(
				(viewedRectangle.pY1 + viewedRectangle.pHeight) / partHeightPixels,
			);
			topVisible = Math.max(0, topVisible);
			bottomVisible = Math.min(bottomVisible, docLayer._parts - 1);

			const scale = app.twipsToPixels;
			const offsetX = -viewedRectangle.pX1;

			for (let part = topVisible; part <= bottomVisible; part++) {
				const cached = RenderManager.requestPart(part);
				if (!cached) continue;

				const offsetY = -viewedRectangle.pY1 + part * partHeightPixels;

				this.context.save();
				this.context.translate(offsetX, offsetY);
				this.context.scale(scale, scale);
				RenderManager.renderInto(this.context, cached);
				this.context.restore();
			}
		}
	}
}
