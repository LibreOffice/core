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

	it('a click on the start point ends an unfilled part open, with no point set there', function () {
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
		nodeassert.strictEqual(
			PathDrawing.asSvg(drawing.path()),
			'M 0 0 L 100 0 L 100 100',
		);
	});

	it('a click on the start point finishes a path of one polygon', function () {
		const drawing = new PathDrawing(single, 10);
		click(drawing, [
			[0, 0],
			[100, 0],
		]);
		drawing.moveTo(new cool.Point(3, 2), false);
		nodeassert.strictEqual(drawing.setThePoint(false), 'finished');
		nodeassert.strictEqual(PathDrawing.asSvg(drawing.path()), 'M 0 0 L 100 0');
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
