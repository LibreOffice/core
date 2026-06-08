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

namespace cool {
	/// Handle slide thumbnails that are rendered from JSON primitive tree.
	export class VectorThumbnailHandler {
		private _docLayer: CanvasTileLayerInterface;

		private _renderer: VectorPrimitiveRenderer = new VectorPrimitiveRenderer(
			(checksum) => this._bitmapCache.get(checksum),
		);

		// Cached parsed JSON primitive tree keyed by part number.
		private _cache: Map<number, VectorTileData> = new Map();

		// Previews waiting for a JSON primitive tree response, keyed by part.
		private _pendingPreviews: Map<number, PendingPreview[]> = new Map();

		// Parts for which a JSON primitive tree request is in flight.
		private _inFlightParts: Set<number> = new Set();

		// Decoded bitmap images keyed by their checksum.
		private _bitmapCache: Map<number, HTMLImageElement> = new Map();

		// Checksums for which a .uno:VectorRenderingGraphics request is
		// in flight.
		private _bitmapsInFlight: Set<number> = new Set();

		// Reverse index from bitmap checksum to the parts that reference
		// it. Lets a freshly-decoded bitmap re-render only the affected
		// thumbnails.
		private _checksumToParts: Map<number, Set<number>> = new Map();

		// Size and part of every preview that has been rendered. A redraw
		// triggered by a decoded bitmap reuses these so the re-fired
		// preview matches what the consumer originally asked for.
		private _renderedPreviews: Map<PreviewId, RenderedPreview> = new Map();

		constructor(docLayer: CanvasTileLayerInterface) {
			this._docLayer = docLayer;
		}

		/// Request a thumbnail for a preview.
		requestThumbnail(
			id: PreviewId,
			part: number,
			maxWidth: number,
			maxHeight: number,
		): void {
			const cached = this._cache.get(part);
			if (cached) {
				this._renderAndFire(id, part, maxWidth, maxHeight, cached);
				return;
			}

			let queue = this._pendingPreviews.get(part);
			if (!queue) {
				queue = [];
				this._pendingPreviews.set(part, queue);
			}
			queue.push({ id: id, maxWidth: maxWidth, maxHeight: maxHeight });

			if (!this._inFlightParts.has(part)) {
				this._inFlightParts.add(part);
				this._sendVectorTileRequest(part);
			}
		}

		private _sendVectorTileRequest(part: number): void {
			app.socket.sendMessage(
				'commandvalues command=.uno:VectorTile?part=' + String(part),
			);
		}

		/// Called when a response arrives.
		handleVectorTileResponse(values: VectorTileResponse): void {
			const part =
				values.part !== undefined ? values.part : this._docLayer._selectedPart;

			this._inFlightParts.delete(part);

			const masterPage =
				values.masterPage && values.masterPage.primitives
					? values.masterPage.primitives
					: [];
			const objects = values.objects || [];

			const data: VectorTileData = {
				slideWidth: values.slideWidth || 0,
				slideHeight: values.slideHeight || 0,
				masterPage: masterPage,
				objects: objects,
			};
			this._cache.set(part, data);

			const required = new Set<number>();
			const walker = new VectorBitmapWalker(required);
			walker.walkPrimitives(masterPage);
			walker.walkObjects(objects);
			this._indexChecksumsForPart(part, required);
			this._requestMissingBitmaps(required);

			this._drainPending(part, data);
		}

		/// Called when a bitmap arrives in response to an earlier
		/// fetch request.
		handleVectorRenderingGraphicsResponse(
			values: VectorRenderingGraphicsResponse,
		): void {
			if (this._bitmapCache.has(values.checksum)) {
				this._bitmapsInFlight.delete(values.checksum);
				return;
			}
			const image = new Image();
			// Add to the cache only on a successful decode. Clear
			// the in-flight mark on either outcome.
			image.onload = () => {
				this._bitmapCache.set(values.checksum, image);
				this._bitmapsInFlight.delete(values.checksum);
				this._redrawPreviewsFor(values.checksum);
			};
			image.onerror = () => {
				this._bitmapsInFlight.delete(values.checksum);
			};
			image.src = values.data;
		}

		private _indexChecksumsForPart(part: number, checksums: Set<number>): void {
			for (const checksum of checksums) {
				let parts = this._checksumToParts.get(checksum);
				if (!parts) {
					parts = new Set<number>();
					this._checksumToParts.set(checksum, parts);
				}
				parts.add(part);
			}
		}

		private _redrawPreviewsFor(checksum: number): void {
			const parts = this._checksumToParts.get(checksum);
			if (!parts) return;
			for (const part of parts) {
				const data = this._cache.get(part);
				if (!data) continue;
				for (const [id, info] of this._renderedPreviews) {
					if (info.part !== part) continue;
					this._renderAndFire(id, part, info.maxWidth, info.maxHeight, data);
				}
			}
		}

		private _requestMissingBitmaps(checksums: Set<number>): void {
			for (const checksum of checksums) {
				if (this._bitmapCache.has(checksum)) continue;
				if (this._bitmapsInFlight.has(checksum)) continue;
				this._bitmapsInFlight.add(checksum);
				app.socket.sendMessage(
					'commandvalues command=.uno:VectorRenderingGraphics?checksum=' +
						String(checksum),
				);
			}
		}

		/// Drop cached data for a part and any in-flight state.
		clearCachedPart(part: number): void {
			this._cache.delete(part);
			this._inFlightParts.delete(part);
		}

		/// Drop all cached data for all parts.
		clearCache(): void {
			this._cache.clear();
			this._inFlightParts.clear();
			this._pendingPreviews.clear();
			this._bitmapCache.clear();
			this._bitmapsInFlight.clear();
			this._checksumToParts.clear();
			this._renderedPreviews.clear();
		}

		private _drainPending(part: number, data: VectorTileData): void {
			const queue = this._pendingPreviews.get(part);
			if (!queue) return;
			this._pendingPreviews.delete(part);

			for (const pending of queue) {
				this._renderAndFire(
					pending.id,
					part,
					pending.maxWidth,
					pending.maxHeight,
					data,
				);
			}
		}

		/// Render to an offscreen canvas and trigger rendering of the thumbnail.
		private _renderAndFire(
			id: PreviewId,
			part: number,
			maxWidth: number,
			maxHeight: number,
			data: VectorTileData,
		): void {
			if (data.slideWidth <= 0 || data.slideHeight <= 0) return;

			this._renderedPreviews.set(id, {
				part: part,
				maxWidth: maxWidth,
				maxHeight: maxHeight,
			});

			const pxW = Math.max(1, Math.round(maxWidth * app.roundedDpiScale));
			const pxH = Math.max(1, Math.round(maxHeight * app.roundedDpiScale));

			const canvas = document.createElement('canvas');
			canvas.width = pxW;
			canvas.height = pxH;
			const context = canvas.getContext('2d');
			if (!context) return;

			// Twips to canvas pixels.
			context.scale(pxW / data.slideWidth, pxH / data.slideHeight);

			for (const primitive of data.masterPage) {
				this._renderer.renderPrimitive(context, primitive);
			}
			for (const obj of data.objects) {
				if (!obj.primitives) continue;
				for (const primitive of obj.primitives) {
					this._renderer.renderPrimitive(context, primitive);
				}
			}

			const previewImage = new Image();
			previewImage.width = maxWidth;
			previewImage.height = maxHeight;
			previewImage.src = canvas.toDataURL('image/png');

			app.map.fire('tilepreview', {
				tile: previewImage,
				id: id,
				width: maxWidth,
				height: maxHeight,
				part: part,
				mode: 0,
				docType: this._docLayer._docType,
			});
		}
	}
}
