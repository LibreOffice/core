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

	it('a dragged point moves with its weights, and the other paths stay', function () {
		const curved: cool.ObjectMotionPath = {
			path: [
				{
					points: [
						{ x: 0, y: 0, aheadX: 100, aheadY: 0 },
						{ x: 500, y: 0, behindX: 400, behindY: 50 },
					],
				},
			],
		};
		// the second point of the first path lies at 1700, 2100 on the page
		const moved = MotionPathHandles.withPointAt(
			object([curved, line(0, 300)]),
			'motion 0 9.0.1',
			new cool.Point(1750, 2200),
		);
		const point = moved?.motionPaths?.[0].path[0].points[1];
		nodeassert.deepStrictEqual(
			[point?.x, point?.y, point?.behindX, point?.behindY],
			[550, 100, 450, 150],
		);
		nodeassert.deepStrictEqual(moved?.motionPaths?.[1], line(0, 300));
		// the start point and its weight stay where they were
		nodeassert.deepStrictEqual(moved?.motionPaths?.[0].path[0].points[0], {
			x: 0,
			y: 0,
			aheadX: 100,
			aheadY: 0,
		});
	});

	it('the start of a path drawn beside its object is dragged with its weight', function () {
		const beside: cool.ObjectMotionPath = {
			path: [
				{
					points: [
						{ x: 600, y: 300, aheadX: 700, aheadY: 300 },
						{ x: 1200, y: 300 },
					],
				},
			],
		};
		// the start lies at 1800, 2400 on the page, away from the middle of the object
		const moved = MotionPathHandles.withPointAt(
			object([beside]),
			'motion 0 9.0.0',
			new cool.Point(1700, 2500),
		);
		const start = moved?.motionPaths?.[0].path[0].points[0];
		nodeassert.deepStrictEqual(
			[start?.x, start?.y, start?.aheadX, start?.aheadY],
			[500, 400, 600, 400],
		);
		nodeassert.deepStrictEqual(moved?.motionPaths?.[0].path[0].points[1], {
			x: 1200,
			y: 300,
		});
	});

	it('a name that is no point of the paths moves nothing', function () {
		nodeassert.strictEqual(
			MotionPathHandles.withPointAt(
				object([line(500, 0)]),
				'motion 3 9.0.0',
				new cool.Point(0, 0),
			),
			null,
		);
		nodeassert.strictEqual(MotionPathHandles.placeOf('path 0 9.0.1'), null);
	});

	it('a dragged weight of a symmetric point swings the other weight with it', function () {
		const symmetric: cool.ObjectMotionPath = {
			path: [
				{
					points: [
						{ x: 0, y: 0 },
						{
							x: 500,
							y: 0,
							behindX: 400,
							behindY: 0,
							aheadX: 600,
							aheadY: 0,
							continuity: 2,
						},
						{ x: 1000, y: 0 },
					],
				},
			],
		};
		// the point lies at 1700, 2100 on the page; its weight coming in is dragged straight up
		const moved = MotionPathHandles.withPointAt(
			object([symmetric]),
			'motion 0 10.0.1.in',
			new cool.Point(1700, 1900),
		);
		const point = moved?.motionPaths?.[0].path[0].points[1];
		nodeassert.deepStrictEqual(
			[
				point?.x,
				point?.y,
				point?.behindX,
				point?.behindY,
				point?.aheadX,
				point?.aheadY,
			],
			[500, 0, 500, -200, 500, 200],
		);
	});
});
