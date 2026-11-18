/* -*- js-indent-level: 8 -*- */
/*
 * Copyright the Collabora Office contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

describe('Point', function () {
	const assert = require('assert');

	it('measures how far it lies from the origin', function () {
		assert.strictEqual(new cool.Point(3, 4).length(), 5);
		assert.strictEqual(new cool.Point(0, 0).length(), 0);
	});

	it('measures how far it reaches along another direction', function () {
		assert.strictEqual(new cool.Point(3, 4).dot(new cool.Point(1, 0)), 3);
		assert.strictEqual(new cool.Point(3, 4).dot(new cool.Point(0, -1)), -4);
	});

	it('tells which side of it another direction lies on', function () {
		const along = new cool.Point(10, 0);
		assert.ok(along.cross(new cool.Point(3, 2)) > 0);
		assert.ok(along.cross(new cool.Point(3, -2)) < 0);
		assert.strictEqual(along.cross(new cool.Point(-4, 0)), 0);
	});
});
