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
 * VectorManager - Renders the document content with vector primitives.
 *
 * It owns the per-part vector data, the bitmap cache and the primitive
 * renderer. It exposes thumbnail rendering (requestThumbnail) and per-part
 * access with change notification (requestPart, renderInto, onVectorChanged)
 * over one shared cache, so each part is fetched and decoded only once. The
 * bitmap-grid behaviour stays as the no-ops inherited from RenderManagerBase:
 * a vector-rendered document has no bitmap tile grid.
 */

class VectorManager extends RenderManagerBase {
	private _renderer: cool.VectorPrimitiveRenderer =
		new cool.VectorPrimitiveRenderer((checksum) =>
			this._bitmapCache.get(checksum),
		);

	// Cached parsed JSON primitive tree keyed by part number.
	private _cache: Map<number, cool.VectorPrimitivesData> = new Map();

	// Previews waiting for a JSON primitive tree response, keyed by part.
	private _pendingPreviews: Map<number, cool.PendingPreview[]> = new Map();

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
	// preview keeps the size and part it was rendered at.
	private _renderedPreviews: Map<cool.PreviewId, cool.RenderedPreview> =
		new Map();

	// Callbacks fired when cached vector data changes: a part's
	// primitive tree arrived, a deferred bitmap decoded, or the cache
	// was cleared.
	private _changeListeners: (() => void)[] = [];

	// The Impress or Draw doc layer, read lazily so the manager can be
	// created before the layer is registered on the map.
	private get _docLayer(): cool.CanvasTileLayerInterface {
		return app.map._docLayer as unknown as cool.CanvasTileLayerInterface;
	}

	isVectorRendering(): boolean {
		return true;
	}

	/// Register a callback fired whenever cached vector data changes.
	onVectorChanged(callback: () => void): void {
		this._changeListeners.push(callback);
	}

	private _fireChanged(): void {
		for (const callback of this._changeListeners) callback();
	}

	/// Return the cached primitive tree for a part, or undefined while a
	/// request is sent. Subscribers registered with onVectorChanged are
	/// notified once the data arrives.
	requestPart(part: number): cool.VectorPrimitivesData | undefined {
		const cached = this._cache.get(part);
		if (cached) return cached;

		if (!this._inFlightParts.has(part)) {
			this._inFlightParts.add(part);
			this._sendVectorPrimitivesRequest(part);
		}
		return undefined;
	}

	/// Render a part's primitive tree, master page first then the
	/// objects on top. The caller sets up the context transform that
	/// maps the part's twips to the target pixels.
	renderInto(
		context: CanvasRenderingContext2D,
		data: cool.VectorPrimitivesData,
	): void {
		this._renderer.setSlideBounds(data.slideWidth, data.slideHeight);
		for (const primitive of data.masterPage) {
			this._renderer.renderPrimitive(context, primitive);
		}
		for (const obj of data.objects) {
			if (!obj.primitives) continue;
			for (const primitive of obj.primitives) {
				this._renderer.renderPrimitive(context, primitive);
			}
		}
	}

	/// Request a thumbnail for a preview.
	requestThumbnail(
		id: cool.PreviewId,
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
			this._sendVectorPrimitivesRequest(part);
		}
	}

	private _sendVectorPrimitivesRequest(part: number): void {
		app.socket.sendMessage(
			'commandvalues command=.uno:VectorPrimitives?part=' + String(part),
		);
	}

	/// Handle a vector primitives response.
	handleVectorPrimitivesResponse(values: cool.VectorPrimitivesResponse): void {
		const part =
			values.part !== undefined ? values.part : this._docLayer._selectedPart;

		this._inFlightParts.delete(part);

		const masterPage =
			values.masterPage && values.masterPage.primitives
				? values.masterPage.primitives
				: [];
		const objects = values.objects || [];

		const data: cool.VectorPrimitivesData = {
			slideWidth: values.slideWidth || 0,
			slideHeight: values.slideHeight || 0,
			masterPage: masterPage,
			objects: objects,
		};
		this._cache.set(part, data);

		const required = new Set<number>();
		const walker = new cool.VectorBitmapWalker(required);
		walker.walkPrimitives(masterPage);
		walker.walkObjects(objects);
		this._indexChecksumsForPart(part, required);
		this._requestMissingBitmaps(required);

		this._drainPending(part, data);
		this._fireChanged();
	}

	/// Handle a fetched bitmap: cache it and re-render the
	/// previews that use it.
	handleVectorRenderingGraphicsResponse(
		values: cool.VectorRenderingGraphicsResponse,
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
			this._fireChanged();
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
		this._fireChanged();
	}

	/// Drop the cached primitive trees for every part so the views
	/// re-fetch them. Decoded bitmaps stay cached, since an image that
	/// actually changed comes back under a new checksum anyway.
	clearAllParts(): void {
		this._cache.clear();
		this._inFlightParts.clear();
		this._fireChanged();
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
		this._fireChanged();
	}

	private _drainPending(part: number, data: cool.VectorPrimitivesData): void {
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
		id: cool.PreviewId,
		part: number,
		maxWidth: number,
		maxHeight: number,
		data: cool.VectorPrimitivesData,
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

		this.renderInto(context, data);

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
