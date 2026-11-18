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

describe('PathDrawing', function () {
	const filled: PathDrawingKind = { multi: true, closes: true, inSteps: false };
	const open: PathDrawingKind = { multi: true, closes: false, inSteps: false };
	const single: PathDrawingKind = {
		multi: false,
		closes: false,
		inSteps: false,
	};

	// sets a point at each place, as clicks there do
	function click(drawing: PathDrawing, places: number[][]): PathDrawingOutcome {
		let outcome: PathDrawingOutcome = 'goesOn';
		for (const place of places) {
			drawing.moveTo(new cool.Point(place[0], place[1]), false);
			outcome = drawing.setThePoint(false);
		}
		return outcome;
	}

	it('a right click ends a filled part closed and the next part follows', function () {
		const drawing = new PathDrawing(filled, 10);
		click(drawing, [
			[0, 0],
			[100, 0],
		]);
		drawing.moveTo(new cool.Point(100, 100), false);
		nodeassert.strictEqual(drawing.setThePoint(true), 'partEnded');
		nodeassert.ok(drawing.beforeThePart());
		nodeassert.strictEqual(
			PathDrawing.asSvg(drawing.path()),
			'M 0 0 L 100 0 L 100 100 Z',
		);
	});

	it('closing the first unfilled part on its start closes the later parts too', function () {
		const drawing = new PathDrawing(open, 10);
		click(drawing, [
			[0, 0],
			[100, 0],
			[100, 100],
		]);
		// within reach of the start, so the mouse is drawn onto it
		const placed = drawing.moveTo(new cool.Point(5, 3), false);
		nodeassert.deepStrictEqual([placed.x, placed.y], [0, 0]);
		nodeassert.strictEqual(drawing.setThePoint(false), 'partEnded');
		// a right click ends the next part closed as well
		click(drawing, [
			[300, 0],
			[400, 0],
		]);
		drawing.moveTo(new cool.Point(400, 100), false);
		drawing.setThePoint(true);
		nodeassert.strictEqual(
			PathDrawing.asSvg(drawing.path()),
			'M 0 0 L 100 0 L 100 100 Z M 300 0 L 400 0 L 400 100 Z',
		);
	});

	it('after an open first unfilled part, the start point draws no mouse onto it', function () {
		const drawing = new PathDrawing(open, 10);
		click(drawing, [
			[0, 0],
			[100, 0],
		]);
		drawing.moveTo(new cool.Point(100, 100), false);
		drawing.setThePoint(true);
		click(drawing, [
			[300, 0],
			[400, 0],
			[400, 100],
		]);
		const placed = drawing.moveTo(new cool.Point(305, 3), false);
		nodeassert.deepStrictEqual([placed.x, placed.y], [305, 3]);
		nodeassert.strictEqual(drawing.startOnTheMouse(), null);
	});

	it('a click on the start point finishes a path of one polygon, closed', function () {
		const drawing = new PathDrawing(single, 10);
		click(drawing, [
			[0, 0],
			[100, 0],
			[100, 100],
		]);
		drawing.moveTo(new cool.Point(3, 2), false);
		nodeassert.strictEqual(drawing.setThePoint(false), 'finished');
		nodeassert.strictEqual(
			PathDrawing.asSvg(drawing.path()),
			'M 0 0 L 100 0 L 100 100 Z',
		);
	});

	it('a click on the start point ends a filled kind closed, with no second point there', function () {
		const drawing = new PathDrawing(filled, 10);
		click(drawing, [
			[0, 0],
			[100, 0],
			[100, 100],
			[5, 3],
		]);
		nodeassert.ok(drawing.beforeThePart());
		nodeassert.strictEqual(
			PathDrawing.asSvg(drawing.path()),
			'M 0 0 L 100 0 L 100 100 Z',
		);
	});

	it('the path goes back point by point, with the weights in use only', function () {
		nodeassert.strictEqual(
			PathDrawing.asJson([
				{
					closed: true,
					points: [
						{ x: 0.4, y: 0, aheadX: 50.6, aheadY: -20 },
						{ x: 100, y: 0 },
					],
				},
				{
					points: [
						{ x: 0, y: 200 },
						{ x: 100, y: 200 },
					],
				},
			]),
			'{"path":[{"closed":true,"points":[{"x":0,"y":0,"aheadX":51,"aheadY":-20},' +
				'{"x":100,"y":0}]},{"points":[{"x":0,"y":200},{"x":100,"y":200}]}]}',
		);
	});

	const curveFilled: PathDrawingKind = {
		multi: true,
		closes: true,
		inSteps: false,
		curves: true,
	};
	const curveOpen: PathDrawingKind = {
		multi: true,
		closes: false,
		inSteps: false,
		curves: true,
	};

	// presses at a place and lets go where the mouse pulled to, as a click or a drag does
	function pressAndPull(
		drawing: PathDrawing,
		at: number[],
		to: number[],
	): PathDrawingOutcome {
		drawing.moveTo(new cool.Point(at[0], at[1]), false);
		drawing.press(new cool.Point(at[0], at[1]));
		drawing.pull(new cool.Point(to[0], to[1]), false);
		return drawing.release(false);
	}

	it('a drag at a point of a curve pulls out mirrored weights, a click gives a corner', function () {
		const drawing = new PathDrawing(curveOpen, 10);
		pressAndPull(drawing, [0, 0], [0, 0]);
		pressAndPull(drawing, [100, 0], [150, 50]);
		pressAndPull(drawing, [200, 0], [204, 3]);
		drawing.finish();
		nodeassert.strictEqual(
			JSON.stringify(drawing.path()),
			'[{"closed":false,"points":[{"x":0,"y":0},' +
				'{"x":100,"y":0,"aheadX":150,"aheadY":50,"behindX":50,"behindY":-50},' +
				'{"x":200,"y":0}]}]',
		);
	});

	it('a press that snapped a point of a curve away from the mouse is no pull', function () {
		const drawing = new PathDrawing(curveFilled, 10);
		pressAndPull(drawing, [0, 0], [0, 0]);
		pressAndPull(drawing, [100, 0], [100, 0]);
		pressAndPull(drawing, [100, 100], [100, 100]);
		// pressed and let go 8 away from the start, which the start drew onto it
		nodeassert.strictEqual(pressAndPull(drawing, [8, 0], [8, 0]), 'partEnded');
		const start = drawing.path()[0].points[0];
		nodeassert.strictEqual(start.aheadX, undefined);
		nodeassert.strictEqual(start.behindX, undefined);
	});

	it('closing a curve on its start without a drag carries on through the start', function () {
		const drawing = new PathDrawing(curveFilled, 10);
		pressAndPull(drawing, [0, 0], [50, -50]);
		pressAndPull(drawing, [100, 0], [100, 0]);
		pressAndPull(drawing, [100, 100], [100, 100]);
		nodeassert.strictEqual(pressAndPull(drawing, [3, 2], [3, 2]), 'partEnded');
		const start = drawing.path()[0].points[0];
		nodeassert.deepStrictEqual(
			[start.behindX, start.behindY, start.aheadX, start.aheadY],
			[-50, 50, 50, -50],
		);
		nodeassert.strictEqual(drawing.path()[0].points.length, 3);
	});

	it('a drag on the start point of a closing curve pulls its weights', function () {
		const drawing = new PathDrawing(curveFilled, 10);
		pressAndPull(drawing, [0, 0], [0, 0]);
		pressAndPull(drawing, [100, 0], [100, 0]);
		nodeassert.strictEqual(pressAndPull(drawing, [2, 1], [0, 40]), 'partEnded');
		const part = drawing.path()[0];
		nodeassert.strictEqual(part.closed, true);
		nodeassert.deepStrictEqual(
			[part.points[0].behindX, part.points[0].behindY],
			[0, -40],
		);
	});

	it('an open curve keeps no weight beyond its ends', function () {
		const drawing = new PathDrawing(curveOpen, 10);
		pressAndPull(drawing, [0, 0], [50, 50]);
		pressAndPull(drawing, [100, 0], [150, 50]);
		drawing.finish();
		const points = drawing.path()[0].points;
		nodeassert.strictEqual(points[0].behindX, undefined);
		nodeassert.strictEqual(points[1].aheadX, undefined);
		nodeassert.strictEqual(points[0].aheadX, 50);
		nodeassert.strictEqual(points[1].behindX, 50);
	});

	it('a stroke is fitted; dropping one being drawn keeps the parts before', function () {
		const drawing = new PathDrawing(
			{ multi: true, closes: false, inSteps: false, freehand: true },
			10,
		);
		drawing.strokesTaken(2, 3, 20);
		drawing.strokeFrom(new cool.Point(0, 0));
		for (let x = 10; x <= 300; x += 10) drawing.strokeTo(new cool.Point(x, 0));
		nodeassert.strictEqual(drawing.strokeEnd(), 'partEnded');
		nodeassert.strictEqual(
			PathDrawing.asSvg(drawing.path()),
			'M 0 0 C 100 0 200 0 300 0',
		);

		drawing.strokeFrom(new cool.Point(0, 100));
		drawing.strokeTo(new cool.Point(50, 100));
		nodeassert.strictEqual(drawing.dropThePart(), 'goesOn');
		nodeassert.strictEqual(drawing.path().length, 1);
		nodeassert.strictEqual(drawing.stroking(), false);
	});

	it('a stroke of a kind that closes ends by itself when it comes back to its start', function () {
		const drawing = new PathDrawing(
			{ multi: true, closes: true, inSteps: false, freehand: true },
			10,
		);
		drawing.strokesTaken(2, 3, 20);
		drawing.strokeFrom(new cool.Point(0, 0));
		const way = [
			[100, 0],
			[200, 0],
			[200, 100],
			[200, 200],
			[100, 200],
			[0, 200],
			[0, 100],
			[0, 30],
		];
		for (const at of way)
			nodeassert.strictEqual(
				drawing.strokeTo(new cool.Point(at[0], at[1])),
				'goesOn',
			);
		nodeassert.strictEqual(drawing.strokeTo(new cool.Point(0, 5)), 'partEnded');
		nodeassert.strictEqual(drawing.path()[0].closed, true);
		nodeassert.strictEqual(drawing.stroking(), false);
	});

	it('escape makes the object of the parts ended, without the one being drawn', function () {
		const drawing = new PathDrawing(open, 10);
		click(drawing, [
			[0, 0],
			[100, 0],
		]);
		drawing.moveTo(new cool.Point(100, 100), false);
		drawing.setThePoint(true);
		click(drawing, [[300, 0]]);
		nodeassert.strictEqual(drawing.stopHere(), 'finished');
		nodeassert.strictEqual(
			PathDrawing.asSvg(drawing.path()),
			'M 0 0 L 100 0 L 100 100',
		);
		nodeassert.strictEqual(new PathDrawing(open, 10).stopHere(), 'nothingLeft');
	});

	it('a start point held off draws no mouse onto it', function () {
		const drawing = new PathDrawing(filled, 10);
		click(drawing, [
			[0, 0],
			[100, 0],
			[100, 100],
		]);
		drawing.holdOffTheStart(true);
		const placed = drawing.moveTo(new cool.Point(5, 3), false);
		nodeassert.deepStrictEqual([placed.x, placed.y], [5, 3]);
		nodeassert.strictEqual(drawing.startOnTheMouse(), null);
		nodeassert.strictEqual(drawing.setThePoint(false), 'goesOn');
	});

	it('a part with too few points is dropped', function () {
		const drawing = new PathDrawing(filled, 10);
		click(drawing, [[0, 0]]);
		drawing.moveTo(new cool.Point(100, 0), false);
		nodeassert.strictEqual(drawing.setThePoint(true), 'goesOn');
		nodeassert.strictEqual(drawing.path().length, 0);
		nodeassert.strictEqual(drawing.finish(), 'nothingLeft');
	});

	it('finishing makes one object of every part', function () {
		const drawing = new PathDrawing(open, 10);
		click(drawing, [
			[0, 0],
			[100, 0],
		]);
		drawing.moveTo(new cool.Point(100, 100), false);
		drawing.setThePoint(true);
		click(drawing, [
			[300, 300],
			[400, 300],
		]);
		nodeassert.strictEqual(drawing.finish(), 'finished');
		nodeassert.strictEqual(
			PathDrawing.asSvg(drawing.path()),
			'M 0 0 L 100 0 L 100 100 M 300 300 L 400 300',
		);
	});

	it('a single path is finished as soon as its part ends', function () {
		const drawing = new PathDrawing(single, 10);
		click(drawing, [[0, 0]]);
		drawing.moveTo(new cool.Point(100, 0), false);
		nodeassert.strictEqual(drawing.setThePoint(true), 'finished');
	});

	it('the edge keeps to steps of 45 degrees where asked', function () {
		const drawing = new PathDrawing(open, 10);
		click(drawing, [[0, 0]]);
		const placed = drawing.moveTo(new cool.Point(100, 10), true);
		nodeassert.ok(Math.abs(placed.y) < 1e-9);
		nodeassert.ok(Math.abs(placed.x - Math.hypot(100, 10)) < 1e-9);
	});

	it('taking back points goes into the part before, and ends with nothing left', function () {
		const drawing = new PathDrawing(open, 10);
		click(drawing, [
			[0, 0],
			[100, 0],
		]);
		drawing.moveTo(new cool.Point(100, 100), false);
		drawing.setThePoint(true);
		// the first one goes into the part before and takes back its last point at once
		nodeassert.strictEqual(drawing.back(), 'goesOn');
		nodeassert.strictEqual(drawing.path().length, 0);
		nodeassert.strictEqual(
			PathDrawing.asSvg(drawing.shown()),
			'M 0 0 L 100 0 L 100 100',
		);
		nodeassert.strictEqual(drawing.back(), 'goesOn');
		nodeassert.strictEqual(drawing.back(), 'nothingLeft');
	});

	it('dropping takes the part being drawn, then the parts before', function () {
		const drawing = new PathDrawing(open, 10);
		click(drawing, [
			[0, 0],
			[100, 0],
		]);
		drawing.moveTo(new cool.Point(100, 100), false);
		drawing.setThePoint(true);
		click(drawing, [[300, 300]]);
		nodeassert.strictEqual(drawing.dropThePart(), 'goesOn');
		nodeassert.strictEqual(drawing.path().length, 1);
		nodeassert.strictEqual(drawing.dropThePart(), 'nothingLeft');
	});
});
