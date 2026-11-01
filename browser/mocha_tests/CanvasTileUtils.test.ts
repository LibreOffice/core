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

describe('CanvasTileUtils', function () {
	const assert = require('assert');

	const maskBytes = 256 / 8;

	// Encode RGBA rows the way kit/Delta.hpp writes a keyframe: per row, the
	// count of new pixels (16-bit little endian), a 256-bit mask where a set
	// bit repeats the previous pixel, then the new pixels. The previous pixel
	// starts as transparent black on every row.
	function encodeKeyframe(
		pixels: Uint8Array,
		width: number,
		height: number,
	): Uint8Array {
		const out: number[] = [];
		for (let y = 0; y < height; ++y) {
			const mask = new Array(maskBytes).fill(0);
			const newPixels: number[] = [];
			let last = [0, 0, 0, 0];
			for (let x = 0; x < width; ++x) {
				const i = (y * width + x) * 4;
				const pixel = [pixels[i], pixels[i + 1], pixels[i + 2], pixels[i + 3]];
				if (pixel.every((v, c) => v === last[c])) mask[x >> 3] |= 1 << (x & 7);
				else {
					newPixels.push(...pixel);
					last = pixel;
				}
			}
			const count = newPixels.length / 4;
			out.push(count & 0xff, count >> 8, ...mask, ...newPixels);
		}
		return new Uint8Array(out);
	}

	function row(count: number, mask: number[], pixels: number[]): number[] {
		const maskRow = new Array(maskBytes).fill(0);
		mask.forEach((v, i) => (maskRow[i] = v));
		return [count & 0xff, count >> 8, ...maskRow, ...pixels];
	}

	describe('unrle', function () {
		it('expands repeated pixels and restarts each row from transparent', function () {
			// Row 0: red, repeat, blue, repeat. Row 1: repeat (transparent), green, repeat, repeat.
			const data = new Uint8Array([
				...row(2, [0b1010], [255, 0, 0, 255, 0, 0, 255, 255]),
				...row(1, [0b1101], [0, 255, 0, 255]),
			]);
			const output = new Uint8Array(4 * 2 * 4);

			cool.CanvasTileUtils.unrle(data, 4, 2, output);

			assert.deepStrictEqual(
				Array.from(output),
				[
					[255, 0, 0, 255],
					[255, 0, 0, 255],
					[0, 0, 255, 255],
					[0, 0, 255, 255],
					[0, 0, 0, 0],
					[0, 255, 0, 255],
					[0, 255, 0, 255],
					[0, 255, 0, 255],
				].flat(),
			);
		});

		it('returns the size of the keyframe, where the next delta starts', function () {
			const keyframe = row(1, [0b10], [1, 2, 3, 255]);
			const data = new Uint8Array([...keyframe, 99, 1, 0, 0]);

			const size = cool.CanvasTileUtils.unrle(data, 2, 1, new Uint8Array(8));

			assert.strictEqual(size, keyframe.length);
		});

		it('converts premultiplied pixels to straight alpha', function () {
			const data = new Uint8Array(
				row(2, [], [64, 32, 0, 128, 10, 20, 30, 255]),
			);
			const output = new Uint8Array(8);

			cool.CanvasTileUtils.unrle(data, 2, 1, output);

			assert.deepStrictEqual(
				Array.from(output),
				[128, 64, 0, 128, 10, 20, 30, 255],
			);
		});

		it('writes from outputOffset and leaves the bytes before it alone', function () {
			const data = new Uint8Array(row(1, [0b10], [9, 8, 7, 255]));
			const output = new Uint8Array(12).fill(42);

			cool.CanvasTileUtils.unrle(data, 2, 1, output, 4);

			assert.deepStrictEqual(
				Array.from(output),
				[42, 42, 42, 42, 9, 8, 7, 255, 9, 8, 7, 255],
			);
		});

		it('decodes the complete rows of a keyframe that is cut short', function () {
			// The second row ends inside its repeat mask.
			const data = new Uint8Array([
				...row(1, [0b10], [5, 6, 7, 255]),
				...row(1, [], [1, 2, 3, 255]).slice(0, 10),
			]);
			const output = new Uint8Array(2 * 2 * 4);

			cool.CanvasTileUtils.unrle(data, 2, 2, output);

			assert.deepStrictEqual(
				Array.from(output),
				[5, 6, 7, 255, 5, 6, 7, 255, 0, 0, 0, 0, 0, 0, 0, 0],
			);
		});

		it('decodes a full tile back to the pixels it was encoded from', function () {
			const size = 256;
			const pixels = new Uint8Array(size * size * 4);
			// Opaque runs of varying length, so both mask bits and new pixels
			// appear in every mask byte, plus a transparent stretch.
			let seed = 7;
			for (let y = 0; y < size; ++y) {
				let x = 0;
				while (x < size) {
					seed = (seed * 1103515245 + 12345) & 0x7fffffff;
					const run = 1 + (seed % 13);
					const transparent = y % 32 === 5 && x < 64;
					const pixel = transparent
						? [0, 0, 0, 0]
						: [seed & 0xff, (seed >> 8) & 0xff, (seed >> 16) & 0xff, 255];
					for (let i = 0; i < run && x < size; ++i, ++x)
						pixels.set(pixel, (y * size + x) * 4);
				}
			}
			const data = encodeKeyframe(pixels, size, size);
			const output = new Uint8Array(size * size * 4);

			const consumed = cool.CanvasTileUtils.unrle(data, size, size, output);

			assert.strictEqual(consumed, data.length);
			assert.deepStrictEqual(output, pixels);
		});
	});
});
