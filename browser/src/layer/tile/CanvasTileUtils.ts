/*
 * Copyright the Collabora Online contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/* global L */

namespace cool {
	export class RawDelta {
		private _delta: Uint8Array;
		private _id: number;
		private _isKeyframe: boolean;

		constructor(delta: Uint8Array, id: number, isKeyframe: boolean) {
			this._delta = delta;
			this._id = id;
			this._isKeyframe = isKeyframe;
		}

		public get length(): number {
			return this.delta.length;
		}

		public get delta(): Uint8Array {
			return this._delta;
		}

		public get id(): number {
			return this._id;
		}

		public get isKeyframe(): boolean {
			return this._isKeyframe;
		}
	}

	export abstract class CanvasTileUtils {
		// A DataView reads in an explicit byte order, a Uint32Array writes in the
		// platform's, so pixels are read in the platform's order to copy their bytes as-is.
		private static readonly littleEndian =
			new Uint8Array(new Uint32Array([1]).buffer)[0] === 1;

		// Cairo (and therefore the server's tile buffers) uses premultiplied alpha,
		// but ImageData / 2D-canvas compositing expects straight alpha. We convert
		// the changed pixels back as deltas are applied. A lookup table keyed by
		// (alpha << 8) + channelValue avoids a per-channel divide in the hot path.
		private static unpremultiplyTable: Uint8Array | null = null;

		private static getUnpremultiplyTable(): Uint8Array {
			if (this.unpremultiplyTable) return this.unpremultiplyTable;

			const table = new Uint8Array(256 * 256);
			// The alpha === 0 row stays zero: fully transparent -> zero RGB.
			for (let a = 1; a < 256; ++a) {
				const base = a << 8;
				for (let v = 0; v < 256; ++v)
					table[base + v] = Math.min(255, Math.ceil((v * 255) / a));
			}

			this.unpremultiplyTable = table;
			return table;
		}

		// Convert premultiplied-alpha RGBA to straight alpha, in place.
		// byteOffset/byteLength delimit the region to touch.
		public static unpremultiply(
			data: Uint8Array,
			byteLength: number,
			byteOffset: number = 0,
		): void {
			const table = this.getUnpremultiplyTable();
			for (let i = byteOffset; i < byteOffset + byteLength; i += 4) {
				const alpha = data[i + 3];
				if (alpha === 255) continue;
				const base = alpha << 8;
				data[i] = table[base + data[i]];
				data[i + 1] = table[base + data[i + 1]];
				data[i + 2] = table[base + data[i + 2]];
			}
		}

		// output.byteOffset + outputOffset must be a multiple of 4, and output must
		// hold width * height * 4 bytes from outputOffset. Decoding stops at the
		// first row that does not fit in data, and leaves the rows below as they were.
		public static unrle(
			data: Uint8Array,
			width: number,
			height: number,
			output: Uint8Array,
			outputOffset: number = 0,
		): number {
			// Pixels are copied as 32-bit words.
			const output32 = new Uint32Array(
				output.buffer,
				output.byteOffset + outputOffset,
				width * height,
			);
			const input = new DataView(data.buffer, data.byteOffset, data.byteLength);
			const littleEndian = this.littleEndian;
			let offset = 0;
			const rleMaskSizeBytes = 256 / 8;
			for (let y = 0; y < height; ++y) {
				if (offset + 2 + rleMaskSizeBytes > data.length) return data.length;
				const rleSize = data[offset] + data[offset + 1] * 256;
				if (offset + 2 + rleMaskSizeBytes + rleSize * 4 > data.length)
					return data.length;
				offset += 2;

				const rleMask = offset;

				offset += rleMaskSizeBytes;

				if (rleSize > 0) this.unpremultiply(data, rleSize * 4, offset);

				let lastPixel = 0;
				let lastMask = 0;
				let bitToCheck = 256;
				let rleMaskOffset = rleMask;

				let pixOffset = y * width;
				let pixSrc = offset;

				for (let x = 0; x < width; ++x) {
					if (bitToCheck > 128) {
						bitToCheck = 1;
						lastMask = data[rleMaskOffset++];
					}
					if (!(lastMask & bitToCheck)) {
						lastPixel = input.getUint32(pixSrc, littleEndian);
						pixSrc += 4;
					}
					bitToCheck = bitToCheck << 1;

					output32[pixOffset++] = lastPixel;
				}

				offset += rleSize * 4;
			}

			return offset;
		}

		public static updateImageFromDeltas(
			imageData: Uint8Array,
			deltas: Uint8Array,
			keyframeDeltaSize: number,
			tileSize: number,
			debug: boolean = false,
		): void {
			let nDelta = 0;
			let offset = keyframeDeltaSize;

			while (offset < deltas.length) {
				if (debug)
					console.debug(
						'Next delta at ' + offset + ' length ' + (deltas.length - offset),
					);

				const delta = !offset ? deltas : deltas.subarray(offset);

				// Debugging paranoia: if we get this wrong bad things happen.
				const susDeltaSize = tileSize * tileSize * 4;
				if (debug && delta.length >= susDeltaSize) {
					console.warn(
						'Unusual delta possibly mis-tagged, suspicious size vs. type ' +
							delta.length +
							' vs. ' +
							susDeltaSize,
					);
				}

				// copy old data to work from:
				const oldData = new Uint8Array(imageData);

				if (debug)
					console.debug(
						'Applying delta chunk ' +
							nDelta++ +
							' of total size ' +
							delta.length +
							' at stream offset ' +
							offset,
					);
				// + ' hex: ' + hex2string(delta, delta.length));

				const len = this.applyDeltaChunk(
					imageData,
					delta,
					oldData,
					tileSize,
					tileSize,
					debug,
				);

				if (debug) console.debug('Applied delta chunk of size ' + len);

				offset += len;
			}
		}

		private static applyDeltaChunk(
			imgData: Uint8Array,
			delta: Uint8Array,
			oldData: Uint8Array,
			width: number,
			height: number,
			debug: boolean = false,
		): number {
			const pixSize = width * height * 4;
			let offset = 0;

			// Green-tinge the old-Data ...
			if (0) {
				for (let i = 0; i < pixSize; ++i) oldData[i * 4 + 1] = 128;
			}

			// wipe to grey.
			if (0) {
				for (let i = 0; i < pixSize * 4; ++i) imgData[i] = 128;
			}

			// Apply delta.
			let i = 0;
			for (let stop = false; i < delta.length && !stop; ) {
				switch (delta[i]) {
					case 99: {
						// 'c': // copy row
						const count = delta[i + 1];
						const srcRow = delta[i + 2];
						const destRow = delta[i + 3];
						if (debug)
							console.debug(
								'[' +
									i +
									']: copy ' +
									count +
									' row(s) ' +
									srcRow +
									' to ' +
									destRow,
							);
						i += 4;
						for (let cnt = 0; cnt < count; ++cnt) {
							const src = (srcRow + cnt) * width * 4;
							const dest = (destRow + cnt) * width * 4;
							for (let j = 0; j < width * 4; ++j) {
								imgData[dest + j] = oldData[src + j];
							}
						}
						break;
					}
					case 100: {
						// 'd': // new run
						const destRow = delta[i + 1];
						const destCol = delta[i + 2];
						let span = delta[i + 3];
						offset = destRow * width * 4 + destCol * 4;
						if (debug)
							console.debug(
								'[' +
									i +
									']: apply new span of size ' +
									span +
									' at pos ' +
									destCol +
									', ' +
									destRow +
									' into delta at byte: ' +
									offset,
							);
						i += 4;
						span *= 4;
						this.unpremultiply(delta, span, i);
						for (let j = 0; j < span; ++j) imgData[offset++] = delta[i + j];
						i += span;
						// imgData.data[offset - 2] = 256; // debug - blue terminator
						break;
					}
					case 116: {
						// 't': // terminate delta new one next
						stop = true;
						i++;
						break;
					}
					default: {
						console.error('[' + i + ']: ERROR: Unknown delta code ' + delta[i]);
						i = delta.length;
						break;
					}
				}
			}

			return i;
		}
	}
} // namespace cool
