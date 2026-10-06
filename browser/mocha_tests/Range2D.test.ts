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

describe('Range2D', function () {
	const assert = require('assert');

	it('measures its width, height and center from the corners', function () {
		const range = new cool.Range2D(1000, 2000, 3000, 5000);
		assert.strictEqual(range.width, 2000);
		assert.strictEqual(range.height, 3000);
		assert.strictEqual(range.centerX, 2000);
		assert.strictEqual(range.centerY, 3500);
	});

	it('round-trips the four corner values through an array', function () {
		const values = [1, 2, 3, 4];
		assert.deepStrictEqual(cool.Range2D.fromArray(values).toArray(), values);
	});

	it('gives no range for a short or missing array', function () {
		assert.strictEqual(cool.Range2D.fromArray([1, 2, 3]), null);
		assert.strictEqual(cool.Range2D.fromArray(undefined), null);
	});

	it('holds all the points it is made from', function () {
		const range = cool.Range2D.fromPoints([
			{ x: 30, y: 5 },
			{ x: 10, y: 40 },
			{ x: 20, y: 20 },
		]);
		assert.deepStrictEqual(range.toArray(), [10, 5, 30, 40]);
	});

	it('grows by as much on each side as it is asked', function () {
		const grown = new cool.Range2D(10, 20, 30, 40).expand(5, 2);
		assert.deepStrictEqual(grown.toArray(), [5, 18, 35, 42]);
	});

	it('brings a point outside it back to its edge', function () {
		const range = new cool.Range2D(10, 20, 30, 40);
		const inside = range.clamp(new cool.Point(15, 25));
		assert.deepStrictEqual([inside.x, inside.y], [15, 25]);
		const outside = range.clamp(new cool.Point(5, 50));
		assert.deepStrictEqual([outside.x, outside.y], [10, 40]);
	});

	it('holds both ranges in their union', function () {
		const union = new cool.Range2D(10, 20, 30, 40).union(
			new cool.Range2D(0, 25, 20, 50),
		);
		assert.deepStrictEqual(union.toArray(), [0, 20, 30, 50]);
	});

	it('holds its four corners after a mapping', function () {
		const moved = new cool.Range2D(10, 20, 30, 40).transform(
			cool.Matrix2D.IDENTITY.scale(2, 3).translate(5, 6),
		);
		assert.deepStrictEqual(moved.toArray(), [25, 66, 65, 126]);

		const turned = new cool.Range2D(0, 0, 2, 1).transform(
			cool.Matrix2D.IDENTITY.rotateAround(0, 0, Math.PI / 2),
		);
		assert.deepStrictEqual(
			turned.toArray().map((value: number) => Math.round(value) + 0),
			[-1, 0, 0, 2],
		);
	});

	it('is empty when it has no area', function () {
		assert.ok(new cool.Range2D(10, 10, 10, 20).isEmpty());
		assert.ok(new cool.Range2D(10, 10, 20, 10).isEmpty());
		assert.ok(new cool.Range2D(20, 10, 10, 20).isEmpty());
		assert.ok(new cool.Range2D(NaN, 10, 20, 20).isEmpty());
		assert.ok(!new cool.Range2D(10, 10, 20, 20).isEmpty());
	});
});
