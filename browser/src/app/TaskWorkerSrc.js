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

/* eslint-disable no-inner-declarations */
/* eslint no-unused-vars: ["warn", { "argsIgnorePattern": "^_" }] */
/* global importScripts Uint8Array cool */

if ('undefined' === typeof window) {
	self.L = {};

	// CanvasTileUtils is bundled inline for mobile/desktop app builds
	if (typeof cool === 'undefined' || !cool.CanvasTileUtils) {
		importScripts(
			'%SERVICE_ROOT%/browser/%VERSION%/src/layer/tile/CanvasTileUtils.js',
		);
	}

	let tileImageCache = new Map(); // Map<string, Uint8Array>

	addEventListener('message', onMessage);

	console.info('TaskWorker initialised');

	function onMessage(e) {
		switch (e.data.message) {
			case 'endTransaction': {
				// Update tile image cache
				for (const key of Array.from(tileImageCache.keys())) {
					if (!e.data.cachedTiles.has(key)) tileImageCache.delete(key);
				}

				const bitmaps = []; // Promise<ImageBitmap>[]
				const tilesWithBitmaps = [];

				const tileByteSize = e.data.tileSize * e.data.tileSize * 4;
				for (const tile of e.data.deltas) {
					const deltas = self.fzstd.decompress(tile.rawDelta);

					// Decompress the keyframe buffer
					let keyframeDeltaSize = 0;
					let imageData = null;
					if (tile.isKeyframe) {
						imageData = new Uint8Array(tileByteSize);
						keyframeDeltaSize = cool.CanvasTileUtils.unrle(
							deltas,
							e.data.tileSize,
							e.data.tileSize,
							imageData,
						);
					} else imageData = tileImageCache.get(tile.key);

					// The main thread has no use for the concatenated rawDelta, delete it here
					// instead of passing it back.
					delete tile.rawDelta;

					if (imageData === null) {
						console.warn('Delta update received on tile with no cached image');
						continue;
					}

					cool.CanvasTileUtils.updateImageFromDeltas(
						imageData,
						deltas,
						keyframeDeltaSize,
						e.data.tileSize,
					);
					tileImageCache.set(tile.key, imageData);

					const clampedData = new Uint8ClampedArray(
						imageData.buffer,
						imageData.byteOffset,
						imageData.byteLength,
					);
					const image = new ImageData(
						clampedData,
						e.data.tileSize,
						e.data.tileSize,
					);
					bitmaps.push(
						createImageBitmap(image, {
							premultiplyAlpha: 'none',
						}),
					);
					tilesWithBitmaps.push(tile);
				}

				Promise.all(bitmaps).then((bitmaps) => {
					for (const bitmap of bitmaps) {
						const tile = tilesWithBitmaps.shift();
						tile.bitmap = bitmap;
					}
					postMessage(
						{
							message: e.data.message,
							deltas: e.data.deltas,
							tileSize: e.data.tileSize,
						},
						bitmaps,
					);
				});
				break;
			}

			case 'decompressImageData': {
				const img = new Uint8ClampedArray(
					self.fzstd.decompress(e.data.rawData),
				);
				createImageBitmap(new ImageData(img, e.data.width, e.data.height), {
					premultiplyAlpha: 'none',
				}).then((bitmap) => {
					postMessage(
						{
							message: 'decompressedImageData',
							id: e.data.id,
							bitmap: bitmap,
						},
						[bitmap],
					);
				});
				break;
			}

			default:
				console.error('Unrecognised worker message');
		}
	}
}
