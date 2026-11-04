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

describe('VectorImageBlur', function () {
	const assert = require('assert');

	/// An image filled with one RGBA value.
	function filled(width: number, height: number, rgba: number[]) {
		const data = new Uint8ClampedArray(width * height * 4);
		for (let index = 0; index < data.length; index += 4) data.set(rgba, index);
		return data;
	}

	function pixel(data: Uint8ClampedArray, width: number, x: number, y: number) {
		const index = 4 * (y * width + x);
		return Array.from(data.slice(index, index + 4));
	}

	it('spreads as far as a Gaussian of the deviation', function () {
		// A box of width w adds (w * w - 1) / 12 to the variance.
		for (const deviation of [1, 2.5, 4, 10, 31]) {
			let variance = 0;
			for (const size of cool.VectorImageBlur.boxSizes(deviation))
				variance += (size * size - 1) / 12;
			assert.ok(
				Math.abs(Math.sqrt(variance) - deviation) < 0.5,
				'deviation ' + deviation + ' gives ' + Math.sqrt(variance),
			);
		}
	});

	it('uses odd box widths, so a blurred image does not shift', function () {
		for (const size of cool.VectorImageBlur.boxSizes(7))
			assert.strictEqual(size % 2, 1);
	});

	it('leaves the inside of an even area as it was', function () {
		const width = 40;
		const data = filled(width, width, [200, 100, 50, 255]);
		cool.VectorImageBlur.blur(data, width, width, 2);
		assert.deepStrictEqual(pixel(data, width, 20, 20), [200, 100, 50, 255]);
	});

	it('fades the edge of an image into the transparent outside', function () {
		const width = 40;
		const data = filled(width, width, [200, 100, 50, 255]);
		cool.VectorImageBlur.blur(data, width, width, 2);
		const corner = pixel(data, width, 0, 0);
		assert.ok(corner[3] < 255 && corner[3] > 0, 'the corner is partly clear');
	});

	it('keeps the color where an object fades into transparent pixels', function () {
		// Red on the left, transparent black on the right. The red stays
		// red where it fades out.
		const width = 40;
		const data = new Uint8ClampedArray(width * width * 4);
		for (let y = 0; y < width; y++)
			for (let x = 0; x < width / 2; x++)
				data.set([255, 0, 0, 255], 4 * (y * width + x));

		cool.VectorImageBlur.blur(data, width, width, 3);

		const edge = pixel(data, width, width / 2 + 1, 20);
		assert.ok(edge[3] > 0 && edge[3] < 255, 'the red fades out past its edge');
		assert.deepStrictEqual(edge.slice(0, 3), [255, 0, 0]);
	});

	it('spreads one dot evenly in every direction', function () {
		const width = 31;
		const data = new Uint8ClampedArray(width * width * 4);
		data.set([0, 0, 255, 255], 4 * (15 * width + 15));

		cool.VectorImageBlur.blur(data, width, width, 2);

		// Rows and columns sum in a different order, so values can differ
		// by one.
		const left = pixel(data, width, 12, 15);
		for (const [x, y] of [
			[18, 15],
			[15, 12],
			[15, 18],
		]) {
			const other = pixel(data, width, x, y);
			for (let channel = 0; channel < 4; channel++)
				assert.ok(Math.abs(other[channel] - left[channel]) <= 1);
		}
		assert.ok(left[3] > 0, 'the dot reaches three pixels away');
	});

	it('changes nothing for a deviation of zero', function () {
		const data = new Uint8ClampedArray([1, 2, 3, 4, 5, 6, 7, 8]);
		cool.VectorImageBlur.blur(data, 2, 1, 0);
		assert.deepStrictEqual(Array.from(data), [1, 2, 3, 4, 5, 6, 7, 8]);
	});
});
