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

		private _renderer: VectorPrimitiveRenderer = new VectorPrimitiveRenderer();

		// Cached parsed JSON primitive tree keyed by part number.
		private _cache: Map<number, VectorTileData> = new Map();

		// Previews waiting for a JSON primitive tree response, keyed by part.
		private _pendingPreviews: Map<number, PendingPreview[]> = new Map();

		// Parts for which a JSON primitive tree request is in flight.
		private _inFlightParts: Set<number> = new Set();

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

			this._drainPending(part, data);
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
