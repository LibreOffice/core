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

describe('Motion paths laid onto the slide', function () {
	// a box from 1000, 2000 that is 400 wide and 200 high, so its middle is at 1200, 2100
	const object = (motionPaths: cool.ObjectMotionPath[]): cool.SlideObject => ({
		x: 1000,
		y: 2000,
		width: 400,
		height: 200,
		motionPaths: motionPaths,
		primitives: [
			{
				type: 'textHierarchyBlock',
				children: [
					{ type: 'textHierarchyParagraph', bounds: [1000, 2000, 1400, 2040] },
					{ type: 'textHierarchyParagraph', bounds: [1000, 2040, 1200, 2080] },
				],
			} as unknown as cool.Primitive,
		],
	});
	const line = (x: number, y: number): cool.ObjectMotionPath => ({
		path: [
			{
				points: [
					{ x: 0, y: 0 },
					{ x: x, y: y },
				],
			},
		],
	});
	const ends = (placed: cool.PlacedMotionPath) =>
		placed.polygons[0].points.map((point: cool.ObjectPathPoint) => [
			point.x,
			point.y,
		]);

	it('a path starts at the middle of its object', function () {
		const placed = cool.MotionPaths.placedOf(object([line(500, 0)]));
		nodeassert.deepStrictEqual(ends(placed[0]), [
			[1200, 2100],
			[1700, 2100],
		]);
	});

	it('every path starts at the object itself, not where another one ends', function () {
		const placed = cool.MotionPaths.placedOf(
			object([line(500, 0), line(0, 300)]),
		);
		nodeassert.deepStrictEqual(ends(placed[1]), [
			[1200, 2100],
			[1200, 2400],
		]);
	});

	it('a path of one paragraph starts at the middle of that paragraph', function () {
		const placed = cool.MotionPaths.placedOf(
			object([{ paragraph: 1, ...line(0, 100) }]),
		);
		nodeassert.deepStrictEqual(ends(placed[0]), [
			[1100, 2060],
			[1100, 2160],
		]);
	});

	it('a dragged object takes its paths along without turning them', function () {
		// a quarter turn round the origin, then a move
		const drag = new cool.Matrix2D(0, 1, -1, 0, 50, 70);
		const placed = cool.MotionPaths.placedOf(object([line(500, 0)]), drag);
		nodeassert.deepStrictEqual(ends(placed[0]), [
			[-2050, 1270],
			[-1550, 1270],
		]);
	});

	it('the first path is gray and the others differ from it and from each other', function () {
		const colours = [0, 1, 2, 3].map((index: number) =>
			cool.MotionPaths.colourOf(index),
		);
		nodeassert.strictEqual(new Set(colours).size, colours.length);
		nodeassert.strictEqual(colours[0], '#77767b');
	});
});
