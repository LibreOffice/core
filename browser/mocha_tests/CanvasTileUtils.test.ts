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

	const size = 256;

	// A 256x256 tile of opaque runs of varying length, so both mask bits and
	// new pixels appear in every mask byte, plus a transparent stretch.
	function testTile(seed: number): Uint8Array {
		const pixels = new Uint8Array(size * size * 4);
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
		return pixels;
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
			const pixels = testTile(7);
			const data = encodeKeyframe(pixels, size, size);
			const output = new Uint8Array(size * size * 4);

			const consumed = cool.CanvasTileUtils.unrle(data, size, size, output);

			assert.strictEqual(consumed, data.length);
			assert.deepStrictEqual(output, pixels);
		});
	});

	describe('updateImageFromDeltas', function () {
		const end = 116; // 't'

		function copyRows(count: number, src: number, dest: number): number[] {
			return [99, count, src, dest];
		}

		function span(y: number, x: number, pixels: number[][]): number[] {
			return [100, y, x, pixels.length, ...pixels.flat()];
		}

		// A 4x4 image where each pixel holds its row and column.
		function smallImage(): Uint8Array {
			const image = new Uint8Array(4 * 4 * 4);
			for (let y = 0; y < 4; ++y)
				for (let x = 0; x < 4; ++x) image.set([y, x, 9, 255], (y * 4 + x) * 4);
			return image;
		}

		function rowOf(image: Uint8Array, y: number): number[] {
			return Array.from(image.subarray(y * 16, (y + 1) * 16));
		}

		it('copies rows from the image as it was before the delta', function () {
			const old = smallImage();
			const image = smallImage();

			// Scroll up by one row: rows 1-3 move to rows 0-2.
			cool.CanvasTileUtils.updateImageFromDeltas(
				image,
				new Uint8Array([...copyRows(3, 1, 0), end]),
				0,
				4,
			);

			assert.deepStrictEqual(rowOf(image, 0), rowOf(old, 1));
			assert.deepStrictEqual(rowOf(image, 1), rowOf(old, 2));
			assert.deepStrictEqual(rowOf(image, 2), rowOf(old, 3));
			assert.deepStrictEqual(rowOf(image, 3), rowOf(old, 3));
		});

		it('writes spans of new pixels as straight alpha', function () {
			const image = smallImage();
			const expected = smallImage();
			expected.set([128, 64, 0, 128, 1, 2, 3, 255], (2 * 4 + 1) * 4);

			cool.CanvasTileUtils.updateImageFromDeltas(
				image,
				new Uint8Array([
					...span(2, 1, [
						[64, 32, 0, 128],
						[1, 2, 3, 255],
					]),
					end,
				]),
				0,
				4,
			);

			assert.deepStrictEqual(image, expected);
		});

		it('copies a row as it was before the delta, after a span changed it', function () {
			const old = smallImage();
			const image = smallImage();
			const red = [255, 0, 0, 255];

			cool.CanvasTileUtils.updateImageFromDeltas(
				image,
				new Uint8Array([
					...span(0, 0, [red, red, red, red]),
					...copyRows(1, 0, 1),
					end,
				]),
				0,
				4,
			);

			assert.deepStrictEqual(rowOf(image, 0), [red, red, red, red].flat());
			assert.deepStrictEqual(rowOf(image, 1), rowOf(old, 0));
		});

		it('applies the deltas after the keyframe in order', function () {
			const image = smallImage();
			const red = [255, 0, 0, 255];
			const keyframe = [1, 2, 3, 4, 5];

			cool.CanvasTileUtils.updateImageFromDeltas(
				image,
				new Uint8Array([
					...keyframe,
					...span(0, 0, [red, red, red, red]),
					end,
					...copyRows(1, 0, 3),
					end,
				]),
				keyframe.length,
				4,
			);

			// The second delta copies the row that the first one painted.
			assert.deepStrictEqual(rowOf(image, 0), [red, red, red, red].flat());
			assert.deepStrictEqual(rowOf(image, 3), rowOf(image, 0));
		});

		// Encode the change from prev to cur the way kit/Delta.hpp does:
		// unchanged rows are skipped, a row found elsewhere in prev is copied,
		// and other rows get spans of changed pixels, at least 3 and at most
		// 254 pixels long.
		function encodeDelta(prev: Uint8Array, cur: Uint8Array): Uint8Array {
			const rowBytes = size * 4;
			const sameRow = (a: number, b: number) =>
				prev
					.subarray(a * rowBytes, (a + 1) * rowBytes)
					.every((v, i) => v === cur[b * rowBytes + i]);
			const samePixel = (o: number) =>
				prev[o] === cur[o] &&
				prev[o + 1] === cur[o + 1] &&
				prev[o + 2] === cur[o + 2] &&
				prev[o + 3] === cur[o + 3];
			const out: number[] = [];
			let lastCopy = -1;
			for (let y = 0; y < size; ++y) {
				if (sameRow(y, y)) continue;
				let match = -1;
				for (let m = 0; m < size && match < 0; ++m)
					if (sameRow(m, y)) match = m;
				if (match >= 0) {
					const count = lastCopy >= 0 ? out[lastCopy + 1] : 0;
					if (
						lastCopy >= 0 &&
						out[lastCopy + 2] + count === match &&
						out[lastCopy + 3] + count === y
					)
						out[lastCopy + 1]++;
					else {
						lastCopy = out.length;
						out.push(...copyRows(1, match, y));
					}
					continue;
				}
				lastCopy = -1;
				for (let x = 0; x < size;) {
					while (x < size && samePixel((y * size + x) * 4)) ++x;
					const pixels: number[][] = [];
					while (
						x + pixels.length < size &&
						pixels.length < 254 &&
						(pixels.length < 3 ||
							!samePixel((y * size + x + pixels.length) * 4))
					) {
						const o = (y * size + x + pixels.length) * 4;
						pixels.push(Array.from(cur.subarray(o, o + 4)));
					}
					if (pixels.length) out.push(...span(y, x, pixels));
					x += pixels.length;
				}
			}
			out.push(end);
			return new Uint8Array(out);
		}

		it('turns a full tile into the scrolled and edited one', function () {
			const prev = testTile(7);
			// Scrolled up by 40 rows, with new rows at the bottom and an edit.
			const cur = new Uint8Array(prev.length);
			cur.set(prev.subarray(40 * size * 4));
			cur.set(testTile(11).subarray(0, 40 * size * 4), (size - 40) * size * 4);
			for (let y = 100; y < 114; ++y)
				for (let x = 60; x < 80; ++x)
					cur.set([0, 0, 0, 255], (y * size + x) * 4);
			const image = new Uint8Array(prev);

			cool.CanvasTileUtils.updateImageFromDeltas(
				image,
				encodeDelta(prev, cur),
				0,
				size,
			);

			assert.deepStrictEqual(image, cur);
		});
	});
});
