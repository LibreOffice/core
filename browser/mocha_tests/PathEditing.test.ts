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

describe('Moving a polygon of a path by its line', function () {
	// what a sink was told, the points moved among it
	class RecordingSink implements PathSink {
		public readonly wholeTwips: boolean = false;
		public moves: PathPointMove[][] = [];
		public movePoints(moves: PathPointMove[]): void {
			this.moves.push(moves);
		}
		public deletePoints(): void {}
		public deleteObject(): void {}
		public addPoint(): void {}
		public changeWeights(): void {}
		public split(): void {}
		public shownPathChanged(): void {}
		public setClosed(): void {}
		public simplify(): void {}
		public pathChangedHere(): void {}
	}

	// a curve from 0, 0 to 1000, 0 bent up, and a closed square from 2000, 0 to 3000, 1000
	const twoParts = (): cool.ObjectPathPolygon[] => [
		{
			points: [
				{ x: 0, y: 0, aheadX: 300, aheadY: -600 },
				{ x: 1000, y: 0, behindX: 700, behindY: -600 },
			],
		},
		{
			closed: true,
			points: [
				{ x: 2000, y: 0 },
				{ x: 3000, y: 0 },
				{ x: 3000, y: 1000 },
				{ x: 2000, y: 1000 },
			],
		},
	];

	let editing: PathEditing;
	let sink: RecordingSink;
	let key = 0;
	beforeEach(function () {
		sink = new RecordingSink();
		const path = twoParts();
		// every test edits a path of its own, as a path changed here is kept by its key
		editing = new PathEditing(
			'polygon test ' + String(++key),
			() => path,
			sink,
		);
	});

	it('a press near the bent line of a curve finds that polygon', function () {
		// the curve runs at 0.75 times 600 above the middle of its ends
		nodeassert.strictEqual(editing.polygonAt(new cool.Point(500, -440), 30), 0);
		nodeassert.strictEqual(editing.polygonAt(new cool.Point(500, 0), 30), null);
	});

	it('a press near the edge that closes a polygon finds that polygon', function () {
		nodeassert.strictEqual(editing.polygonAt(new cool.Point(2010, 500), 30), 1);
		nodeassert.strictEqual(
			editing.polygonAt(new cool.Point(2500, 500), 30),
			null,
		);
	});

	it('a moved polygon takes its weights along, and the other polygon stays', function () {
		const moved = editing.pathWithPolygonMoved(0, new cool.Point(100, 200));
		nodeassert.deepStrictEqual(moved?.[0].points, [
			{ x: 100, y: 200, aheadX: 400, aheadY: -400 },
			{ x: 1100, y: 200, behindX: 800, behindY: -400 },
		]);
		nodeassert.deepStrictEqual(moved?.[1], twoParts()[1]);
		nodeassert.strictEqual(
			editing.pathWithPolygonMoved(2, new cool.Point(1, 1)),
			null,
		);
	});

	it('a moved polygon goes out as one change of all its points, and shows at once', function () {
		editing.movePolygon(1, new cool.Point(-500, 50));
		nodeassert.deepStrictEqual(
			sink.moves.map((moves: PathPointMove[]) =>
				moves.map((move: PathPointMove) => [
					move.polygon,
					move.point,
					move.part,
					move.x,
					move.y,
				]),
			),
			[
				[
					[1, 0, 0, 1500, 50],
					[1, 1, 0, 2500, 50],
					[1, 2, 0, 2500, 1050],
					[1, 3, 0, 1500, 1050],
				],
			],
		);
		nodeassert.deepStrictEqual(editing.path()?.[1].points[0], {
			x: 1500,
			y: 50,
		});
	});
});

describe('The number of each polygon of a path', function () {
	// where the number of that polygon goes, from the points as they are shown
	const numberOf = (polygon: cool.ObjectPathPolygon, which: number) => {
		const shown = new Map<string, SelectionHandle>();
		polygon.points.forEach((point: cool.ObjectPathPoint, at: number) =>
			shown.set('9.' + String(which) + '.' + String(at), {
				name: '9.' + String(which) + '.' + String(at),
				kind: '9',
				point: new cool.Point(point.x, point.y),
			} as SelectionHandle),
		);
		return (PathEditing as any).partNumberAt(polygon, which, shown);
	};
	const rounded = (away: cool.Point) => [
		Math.round(away.x * 1000) / 1000,
		Math.round(away.y * 1000) / 1000,
	];

	it('stands at the start of an open polygon, away from the edge going out', function () {
		const at = numberOf(
			{
				points: [
					{ x: 100, y: 100 },
					{ x: 600, y: 100 },
				],
			},
			1,
		);
		nodeassert.deepStrictEqual(
			[at.x, at.y, ...rounded(at.away)],
			[100, 100, -1, 0],
		);
	});

	it('stands outside the corner a closed polygon starts at', function () {
		const at = numberOf(
			{
				closed: true,
				points: [
					{ x: 0, y: 0 },
					{ x: 1000, y: 0 },
					{ x: 1000, y: 1000 },
					{ x: 0, y: 1000 },
				],
			},
			0,
		);
		const half = Math.round(Math.SQRT1_2 * 1000) / 1000;
		nodeassert.deepStrictEqual(rounded(at.away), [-half, -half]);
	});

	it('stands beside a start that lies on a straight line between its two edges', function () {
		const at = numberOf(
			{
				closed: true,
				points: [
					{ x: 0, y: 0 },
					{ x: 1000, y: 0 },
					{ x: 0, y: 1000 },
					{ x: -1000, y: 0 },
				],
			},
			0,
		);
		nodeassert.deepStrictEqual(rounded(at.away), [0, -1]);
	});
});

describe('Moving a polygon of a motion path by its line', function () {
	// a box from 1000, 2000 that is 400 wide and 200 high, so its middle is at 1200, 2100
	const object = (motionPaths: cool.ObjectMotionPath[]): cool.SlideObject => ({
		x: 1000,
		y: 2000,
		width: 400,
		height: 200,
		motionPaths: motionPaths,
	});

	it('only the polygon taken hold of moves, in the offsets from the middle', function () {
		const twoParts: cool.ObjectMotionPath = {
			path: [
				{
					points: [
						{ x: 0, y: 0, aheadX: 100, aheadY: 0 },
						{ x: 500, y: 0 },
					],
				},
				{
					points: [
						{ x: 600, y: 0 },
						{ x: 900, y: 300 },
					],
				},
			],
		};
		const moved = MotionPathHandles.withPolygonMovedBy(
			object([twoParts]),
			0,
			1,
			new cool.Point(-50, 100),
		);
		nodeassert.deepStrictEqual(moved?.motionPaths?.[0].path, [
			twoParts.path[0],
			{
				points: [
					{ x: 550, y: 100 },
					{ x: 850, y: 400 },
				],
			},
		]);
		nodeassert.strictEqual(
			MotionPathHandles.withPolygonMovedBy(
				object([twoParts]),
				1,
				0,
				new cool.Point(1, 1),
			),
			null,
		);
	});
});
