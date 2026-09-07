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

	it('is empty when it has no area', function () {
		assert.ok(new cool.Range2D(10, 10, 10, 20).isEmpty());
		assert.ok(new cool.Range2D(10, 10, 20, 10).isEmpty());
		assert.ok(new cool.Range2D(20, 10, 10, 20).isEmpty());
		assert.ok(new cool.Range2D(NaN, 10, 20, 20).isEmpty());
		assert.ok(!new cool.Range2D(10, 10, 20, 20).isEmpty());
	});
});
