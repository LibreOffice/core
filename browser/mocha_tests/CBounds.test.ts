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

describe('CBounds', function () {
	describe('Bounds.parse() call with an empty string argument', function () {
		it('should return undefined', function () {
			nodeassert.equal(cool.Bounds.parse(''), undefined);
		});
	});

	describe('Bounds.parse() call with an string argument with 3 numbers', function () {
		it('should return undefined', function () {
			nodeassert.equal(cool.Bounds.parse('10 20 30'), undefined);
		});
	});

	describe('Bounds.parse() call with an string argument with 4 numbers', function () {
		var bounds = cool.Bounds.parse('10 20 30 40');
		it('should return a valid Bounds', function () {
			nodeassert.ok(bounds instanceof cool.Bounds);
			nodeassert.ok(bounds.isValid());
		});

		it('and the Bounds should be correct in position and size', function () {
			nodeassert.ok(
				bounds.equals(
					new cool.Bounds(new cool.Point(10, 20), new cool.Point(40, 60)),
				),
			);
		});
	});

	describe('Bounds constructor call', function () {
		it('correctness check with PointConstructable[] argument', function () {
			var bounds = new cool.Bounds([
				[10, 20],
				{ x: 5, y: 50 },
				[1, 2],
				{ x: -1, y: 7 },
			]);
			var expected = new cool.Bounds(
				new cool.Point(-1, 2),
				new cool.Point(10, 50),
			);
			nodeassert.deepEqual(expected, bounds);
		});
	});
});
