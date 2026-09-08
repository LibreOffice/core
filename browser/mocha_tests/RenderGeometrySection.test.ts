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

describe('RenderGeometrySection', function () {
	// An object keeps its color for as long as it lives, so its rectangle
	// stays recognizable as it moves, and two neighboring ids come out far
	// apart rather than nearly the same.
	it('gives an object a color of its own', function () {
		const first = RenderGeometrySection.colorOfObject(7);
		nodeassert.strictEqual(first, RenderGeometrySection.colorOfObject(7));
		nodeassert.notStrictEqual(first, RenderGeometrySection.colorOfObject(8));

		const hueOf = (color: string) => parseFloat(color.slice(4));
		const apart = Math.abs(
			hueOf(first) - hueOf(RenderGeometrySection.colorOfObject(8)),
		);
		nodeassert.ok(
			Math.min(apart, 360 - apart) > 60,
			'neighboring ids are far apart in hue, got ' + apart,
		);
	});

	// The outline follows the object's own axes, so a rotated object gets a
	// rotated rectangle rather than the upright box around it.
	it('walks the unit rectangle corners through the transform', function () {
		nodeassert.deepStrictEqual(
			RenderGeometrySection.unitRectangleCorners([300, 0, 0, 200, 10, 20]),
			[
				[10, 20],
				[310, 20],
				[310, 220],
				[10, 220],
			],
		);

		// A quarter turn of a 100 by 100 square anchored at (100, 0).
		nodeassert.deepStrictEqual(
			RenderGeometrySection.unitRectangleCorners([0, 100, -100, 0, 100, 0]),
			[
				[100, 0],
				[100, 100],
				[0, 100],
				[0, 0],
			],
		);
	});
});
