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

/* global app JSDialog _ $ errorMessages Uint8Array brandProductName GraphicSelection RenderManager */

// SlideBitmapManager handles the layers for the slideshow
// It provides utility to decompress the row data from zstd and then make bitmaps from it
interface CompressedSlideLayer {
	json: any;
	imgRawData: Uint8Array;
}

class SlideBitmapManager {
	public static pendingLayers: Promise<ImageBitmap>[] = [];
	private static compressedSlideCache: Map<string, CompressedSlideLayer[]> =
		new Map();
	private static lastWorker: number = -1;
	private static lastMessageId: number = 0;
	private static pendingBitmaps: Map<number, any[]> = new Map();

	private static decompressImageData(
		imgRawData: Uint8Array,
		width: number,
		height: number,
	): Promise<ImageBitmap> {
		const workers = window.app.socket.getTaskWorkers();
		if (workers.length) {
			if (this.lastWorker === -1) {
				window.app.socket.addTaskErrorHandler(this.onWorkerError.bind(this));
				window.app.socket.setTaskHandler(
					'decompressedImageData',
					this.onWorkerDecompressedImageData.bind(this),
				);
				this.lastWorker = 0;
			}

			const worker = workers[this.lastWorker];
			this.lastWorker = (this.lastWorker + 1) % workers.length;

			return new Promise<ImageBitmap>((resolve, reject) => {
				this.pendingBitmaps.set(this.lastMessageId, [resolve, reject]);

				// This copies imgRawData, we may want to consider letting the cache live
				// on the worker so we don't have to copy this.
				worker.postMessage({
					message: 'decompressImageData',
					id: this.lastMessageId++,
					rawData: imgRawData,
					width: width,
					height: height,
				});
			});
		}

		// Synchronous path
		const img = (window as any).fzstd.decompress(imgRawData);
		const clampedArray = new Uint8ClampedArray(img);
		const imgData = new ImageData(clampedArray, width, height);
		return createImageBitmap(imgData);
	}

	private static onWorkerDecompressedImageData(e: any) {
		const callbacks = this.pendingBitmaps.get(e.data.id);
		if (callbacks) {
			callbacks[0](e.data.bitmap);
			this.pendingBitmaps.delete(e.data.id);
		} else window.app.console.error('Decompressed image with unrecognised id');
	}

	private static onWorkerError(e: any) {
		for (const callbacks of this.pendingBitmaps.values()) {
			callbacks[1](null);
		}
		this.pendingBitmaps.clear();
		this.lastWorker = -1;
	}

	public static renderCachedCompressedSlide(layers: CompressedSlideLayer[]) {
		const layerBitmapPromises: Promise<ImageBitmap>[] = [];
		for (const layer of layers) {
			const { json, imgRawData } = layer;
			if (json?.width && json?.height) {
				console.debug(
					'CompressedCache: fetching layers from compressed cache',
					layer,
				);
				layerBitmapPromises.push(
					this.decompressImageData(imgRawData, json.width, json.height),
				);
			}
		}

		Promise.all(layerBitmapPromises)
			.then((bitmaps) => {
				let i = 0;
				for (const layer of layers) {
					const json = layer.json;
					if (json?.width && json?.height)
						app.map.fire('slidelayer', {
							message: json,
							image: bitmaps[i++],
						});
				}
			})
			.then(() => {
				console.debug('CompressedCache: Complete Slide rendering');

				app.map.fire('sliderenderingcomplete', {
					success: 'success',
					compressedLayers: false,
				});
			})
			.catch((err) => {
				app.map.fire('sliderenderingcomplete', {
					success: 'fail',
					compressedLayers: false,
				});
				console.error('Something Went wrong while preparing layers', err);
			});
	}

	public static decompressSlideLayer(
		json: any,
		imgBytes: Uint8Array,
	): Promise<ImageBitmap> | null {
		if (json.isCompressed) {
			this.cacheCompressedLayer(json, imgBytes);
		} else if (json.width && json.height) {
			const imgPromise = this.decompressImageData(
				imgBytes,
				json.width,
				json.height,
			);
			this.pendingLayers.push(imgPromise);
			return imgPromise;
		}
		return null;
	}

	public static waitForSlideDecompression(
		json: any,
	): Promise<ImageBitmap[]> | null {
		if (
			json.compressedLayers &&
			this.compressedSlideCache.get(json.slidehash) != null
		) {
			const layers = this.compressedSlideCache.get(json.slidehash);
			if (json.status === 'success') {
				app.map.fire('compressedslide', {
					slideHash: json.slidehash,
					layers: layers,
				});
			}
			this.compressedSlideCache.delete(json.slidehash);
			return null;
		}

		const pendingLayers = this.pendingLayers;
		this.pendingLayers = [];
		return Promise.all(pendingLayers);
	}

	private static cacheCompressedLayer(json: any, imgRawData: Uint8Array) {
		if (!this.compressedSlideCache.has(json.slideHash)) {
			this.compressedSlideCache.set(json.slideHash, []);
		}
		this.compressedSlideCache.get(json.slideHash).push({ json, imgRawData });
	}
}
