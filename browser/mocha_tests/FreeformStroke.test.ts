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

describe('FreeformStroke', function () {
	const assert = require('assert');

	function placesOf(points: number[][]): cool.Point[] {
		return points.map((point: number[]) => new cool.Point(point[0], point[1]));
	}

	function coordinatesOf(places: ReadonlyArray<cool.Point>): number[][] {
		return places.map((place: cool.Point) => [place.x, place.y]);
	}

	it('leaves out a place nearer than the least step to the place kept before it', function () {
		const stroke = new cool.FreeformStroke(10);
		assert.strictEqual(stroke.add(new cool.Point(0, 0)), true);
		assert.strictEqual(stroke.add(new cool.Point(5, 5)), false);
		assert.strictEqual(stroke.drawsALine(), false);
		assert.strictEqual(stroke.add(new cool.Point(10, 0)), true);
		assert.strictEqual(stroke.drawsALine(), true);
		assert.deepStrictEqual(coordinatesOf(stroke.placesSoFar()), [
			[0, 0],
			[10, 0],
		]);
	});

	it('keeps only the ends of a run of places that lie on a line', function () {
		const places = placesOf([
			[0, 0],
			[100, 1],
			[200, -1],
			[300, 0],
		]);
		assert.deepStrictEqual(
			coordinatesOf(cool.FreeformStroke.simplified(places, 5)),
			[
				[0, 0],
				[300, 0],
			],
		);
	});

	it('keeps the corners of the drawing', function () {
		const places = placesOf([
			[0, 0],
			[100, 0],
			[200, 0],
			[200, 100],
			[200, 200],
			[100, 200],
		]);
		assert.deepStrictEqual(
			coordinatesOf(cool.FreeformStroke.simplified(places, 5)),
			[
				[0, 0],
				[200, 0],
				[200, 200],
				[100, 200],
			],
		);
	});

	it('keeps a line that comes back to where it started', function () {
		// The two ends coincide, so the place furthest from them is kept.
		const places = placesOf([
			[0, 0],
			[100, 0],
			[200, 0],
			[100, 0],
			[0, 0],
		]);
		assert.deepStrictEqual(
			coordinatesOf(cool.FreeformStroke.simplified(places, 5)),
			[
				[0, 0],
				[200, 0],
				[0, 0],
			],
		);
	});

	it('asks for an object drawn along the places, in whole twips', function () {
		const places = placesOf([
			[10.4, 20.6],
			[300, 400],
		]);
		assert.strictEqual(
			cool.FreeformStroke.message(places, false),
			'insertpolygonobject closed=false points=10,21;300,400',
		);
		assert.strictEqual(
			cool.FreeformStroke.message(places, true),
			'insertpolygonobject closed=true points=10,21;300,400',
		);
	});
});
