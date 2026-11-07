/* -*- js-indent-level: 8; fill-column: 100 -*- */
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
 * TextPageVectorSection - Canvas section that draws the pages of a text document from vector
 * primitives. Each page is a part of its own, asked for by the id the status names it by, and
 * drawn where the view layout puts it. Only the pages in view and the pages around them are asked
 * for, the pages in view at once and the pages around them a few at a time. These are the wanted
 * pages, which stay cached, and the cache drops the other pages once it holds more than its
 * budget.
 *
 * Other sections ask for a redraw often, so the pixels of each page in view are kept on a canvas
 * of its own, and a redraw of a page that did not change copies them. A page too large for the
 * pixel budget, as at a high zoom, is drawn from its primitives at each redraw.
 */

namespace cool {
	/// What the section keeps of a page between redraws.
	interface KeptTextPage {
		/// The data the page was drawn from.
		data: VectorPrimitivesData;
		/// Everything else the pixels depend on. Empty when the page could not be drawn whole.
		key: string;
		/// The pixels of the page with a margin around it, or null for a page that is drawn
		/// from its primitives at each redraw.
		canvas: HTMLCanvasElement | null;
	}

	export class TextPageVectorSection extends CanvasSectionObject {
		processingOrder: number = app.CSections.VectorContent.processingOrder;
		drawingOrder: number = app.CSections.VectorContent.drawingOrder;
		zIndex: number = app.CSections.VectorContent.zIndex;
		boundToSection: string = 'tiles';
		interactable: boolean = false;

		/// The manager the section draws the pages of.
		private readonly _manager: WriterVectorManager;

		constructor(manager: WriterVectorManager) {
			super(app.CSections.VectorContent.name);
			this._manager = manager;

			// A page that arrived or changed is drawn again.
			RenderManager.onVectorChanged(this._onVectorChanged);
			// Text drawn in a font of the browser that was still loading is drawn again in
			// that font.
			document.fonts?.addEventListener('loadingdone', this._onVectorChanged);
		}

		/// The margin kept around a page, in pixels, for what an entry draws beyond its box.
		private static readonly keptMargin = 16;

		/// The most pixels all the kept canvases hold together, whatever the size of the view.
		private static readonly mostKeptPixels = 16000000;

		// The kept pixels of the pages, by page id.
		private _kept = new Map<string, KeptTextPage>();

		// How many changes of the vector content were announced. Kept pixels made before the
		// newest one are drawn anew.
		private _changes = 0;

		// A change of a page, a picture or a font that loaded and a change of the colors of
		// the view all arrive here, and none of them names the page it changed.
		private _onVectorChanged = (): void => {
			++this._changes;
			if (app.sectionContainer) app.sectionContainer.requestReDraw();
		};

		onRemove(): void {
			RenderManager.offVectorChanged(this._onVectorChanged);
			document.fonts?.removeEventListener('loadingdone', this._onVectorChanged);
			this._kept.clear();
		}

		// Draw the page from its primitives with its top left at (x, y), in pixels of the
		// context. A page that cannot be drawn is reported, and the other pages and sections
		// still draw. True when the whole page was drawn.
		private _renderPage(
			context: CanvasRenderingContext2D,
			pageId: string,
			data: VectorPrimitivesData,
			x: number,
			y: number,
			scale: number,
		): boolean {
			context.save();
			try {
				context.translate(x, y);
				context.scale(scale, scale);
				this._manager.renderTextPage(context, data);
				return true;
			} catch (error) {
				app.console.error(
					'TextPageVectorSection: page ' +
						pageId +
						' could not be drawn: ' +
						error,
				);
				return false;
			} finally {
				context.restore();
			}
		}

		// True when the box of every entry that draws lies on the page, or beyond it by at
		// most the slack, in twips.
		private static _staysOnPage(
			data: VectorPrimitivesData,
			width: number,
			height: number,
			slack: number,
		): boolean {
			for (const entry of data.objects.values()) {
				if (!entry.primitives?.length) continue;
				if (
					entry.x === undefined ||
					entry.y === undefined ||
					entry.width === undefined ||
					entry.height === undefined ||
					entry.x < -slack ||
					entry.y < -slack ||
					entry.x + entry.width > width + slack ||
					entry.y + entry.height > height + slack
				)
					return false;
			}
			return true;
		}

		// The pixels all the kept canvases hold together.
		private _keptPixels(): number {
			let pixels = 0;
			for (const kept of this._kept.values())
				if (kept.canvas) pixels += kept.canvas.width * kept.canvas.height;
			return pixels;
		}

		// A canvas of that size for the page, or null when the kept canvases would hold more
		// pixels than the budget with it. The canvases of pages out of view go first.
		private _canvasFor(
			pageId: string,
			width: number,
			height: number,
			inView: Set<string>,
		): HTMLCanvasElement | null {
			const budget = Math.min(
				3 * this.size[0] * this.size[1],
				TextPageVectorSection.mostKeptPixels,
			);
			const own = this._kept.get(pageId)?.canvas ?? null;
			let others = this._keptPixels() - (own ? own.width * own.height : 0);
			if (others + width * height > budget) {
				for (const [otherId, kept] of Array.from(this._kept)) {
					if (inView.has(otherId) || !kept.canvas) continue;
					others -= kept.canvas.width * kept.canvas.height;
					this._kept.delete(otherId);
				}
			}
			if (others + width * height > budget) return null;

			const canvas = own ?? document.createElement('canvas');
			if (canvas.width !== width || canvas.height !== height) {
				canvas.width = width;
				canvas.height = height;
			}
			return canvas;
		}

		// Draw the page with its top left at the origin, in pixels of the section, from its
		// kept pixels, which are made anew when what they depend on changed. False when the
		// page keeps no pixels, and nothing was drawn.
		private _drawKeptPage(
			pageId: string,
			data: VectorPrimitivesData,
			origin: number[],
			rectangle: number[],
			scale: number,
			inView: Set<string>,
		): boolean {
			const margin = TextPageVectorSection.keptMargin;
			// The canvas lies on whole pixels, and the page starts in it at the same part of a
			// pixel as in the section, so its edges and its text fall on the same pixels.
			const left = Math.floor(origin[0]);
			const top = Math.floor(origin[1]);
			const x = margin + origin[0] - left;
			const y = margin + origin[1] - top;
			const width = Math.ceil(x + rectangle[2] * scale) + margin;
			const height = Math.ceil(y + rectangle[3] * scale) + margin;
			const paper = cool.VectorColors.documentBackground();
			const key = [
				this._changes,
				data.version,
				scale,
				x,
				y,
				width,
				height,
				rectangle[2],
				rectangle[3],
				paper,
			].join(':');

			let kept = this._kept.get(pageId);
			if (!kept || kept.data !== data || kept.key !== key) {
				// An entry may draw beyond its box by half the margin and still be whole.
				const canvas = TextPageVectorSection._staysOnPage(
					data,
					rectangle[2],
					rectangle[3],
					margin / 2 / scale,
				)
					? this._canvasFor(pageId, width, height, inView)
					: null;
				const context = canvas?.getContext('2d') ?? null;
				kept = { data, key, canvas: context ? canvas : null };
				this._kept.set(pageId, kept);
				if (context) {
					context.setTransform(1, 0, 0, 1, 0, 0);
					context.clearRect(0, 0, width, height);
					// The page draws on its paper here as it does in the section.
					context.fillStyle = paper;
					context.fillRect(x, y, rectangle[2] * scale, rectangle[3] * scale);
					// A page that was not drawn whole is drawn again at the next redraw.
					if (!this._renderPage(context, pageId, data, x, y, scale))
						kept.key = '';
				}
			}
			if (!kept.canvas) return false;
			this.context.drawImage(kept.canvas, left - margin, top - margin);
			return true;
		}

		// Keep the pixels of the wanted pages only. A page that was drawn from its primitives
		// because the pixel budget had no room for it gets kept pixels again at its next redraw.
		private _keepPixelsOf(wanted: Set<string>): void {
			for (const [pageId, kept] of Array.from(this._kept))
				if (!wanted.has(pageId) || !kept.canvas) this._kept.delete(pageId);
		}

		/// Where the top left of the page lies in this section, in pixels.
		private static pageOrigin(
			layout: ViewLayoutBase,
			index: number,
			rectangle: number[],
		): number[] {
			const corner = new cool.SimplePoint(rectangle[0], rectangle[1]);
			const anchor = app.sectionContainer.getDocumentAnchor();
			// The multi-page layout places pages apart from where the document has them and
			// says where a given page went. The other layouts keep the document's places.
			const onCanvas =
				layout instanceof ViewLayoutMultiPage
					? layout.documentPointToScreenWithIndex(corner, index)
					: {
							x: layout.documentToViewX(corner),
							y: layout.documentToViewY(corner),
						};
			return [onCanvas.x - anchor[0], onCanvas.y - anchor[1]];
		}

		onDraw(): void {
			if (this.containerObject.isInZoomAnimation()) return;

			const manager = this._manager;
			// The ids come with the rectangles, and until both arrived there is no page to name.
			const rectangles = manager.pageRectangles();
			const layout = app.activeDocument?.activeLayout;
			if (!layout) return;

			const scale = app.twipsToPixels;
			// The pages in view, and the pages within one view height above and below them.
			const wanted = new Set<string>();
			const origins = new Map<number, number[]>();
			const nearby: string[] = [];
			const viewHeight = this.size[1];
			for (let index = 0; index < rectangles.length; ++index) {
				const pageId = manager.partIdAt(index);
				if (pageId === undefined) continue;
				const rectangle = rectangles[index];
				const origin = TextPageVectorSection.pageOrigin(
					layout,
					index,
					rectangle,
				);
				const width = rectangle[2] * scale;
				const height = rectangle[3] * scale;
				if (
					origin[0] + width < 0 ||
					origin[0] > this.size[0] ||
					origin[1] + height < -viewHeight ||
					origin[1] > 2 * viewHeight
				)
					continue;
				wanted.add(pageId);
				if (origin[1] + height >= 0 && origin[1] <= viewHeight)
					origins.set(index, origin);
				else nearby.push(pageId);
			}
			if (origins.size === 0) {
				this._keepPixelsOf(wanted);
				return;
			}

			const inView = new Set<string>();
			for (const index of origins.keys()) {
				const pageId = manager.partIdAt(index);
				if (pageId !== undefined) inView.add(pageId);
			}

			for (const [index, origin] of origins) {
				const rectangle = rectangles[index];

				// The paper is the document background of the view, and the page draws on it.
				this.context.fillStyle = cool.VectorColors.documentBackground();
				this.context.fillRect(
					origin[0],
					origin[1],
					rectangle[2] * scale,
					rectangle[3] * scale,
				);

				const pageId = manager.partIdAt(index);
				const data =
					pageId === undefined ? undefined : manager.requestTextPage(pageId);
				if (!data || pageId === undefined) continue;
				if (!this._drawKeptPage(pageId, data, origin, rectangle, scale, inView))
					this._renderPage(
						this.context,
						pageId,
						data,
						origin[0],
						origin[1],
						scale,
					);
			}

			this._keepPixelsOf(wanted);
			manager.keepTextPages(wanted);
			// The pages in view were asked for first, so they arrive first. The pages around
			// them are asked for a few at a time, while they are among the wanted pages.
			for (const pageId of nearby) manager.requestTextPageAhead(pageId);
		}
	}
}
