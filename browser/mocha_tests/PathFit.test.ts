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

describe('PathFit', function () {
	// the places of a line drawn from one place to another, a place every step
	function placesAlong(
		from: number[],
		to: number[],
		step: number,
	): cool.Point[] {
		const length = Math.hypot(to[0] - from[0], to[1] - from[1]);
		const count = Math.max(1, Math.round(length / step));
		const places: cool.Point[] = [];
		for (let at = 0; at <= count; ++at)
			places.push(
				new cool.Point(
					from[0] + ((to[0] - from[0]) * at) / count,
					from[1] + ((to[1] - from[1]) * at) / count,
				),
			);
		return places;
	}

	// the point of a curve between two points of a path at that parameter
	function pointBetween(
		from: cool.ObjectPathPoint,
		to: cool.ObjectPathPoint,
		t: number,
	): cool.Point {
		const s = 1 - t;
		const x =
			s * s * s * from.x +
			3 * s * s * t * (from.aheadX ?? from.x) +
			3 * s * t * t * (to.behindX ?? to.x) +
			t * t * t * to.x;
		const y =
			s * s * s * from.y +
			3 * s * s * t * (from.aheadY ?? from.y) +
			3 * s * t * t * (to.behindY ?? to.y) +
			t * t * t * to.y;
		return new cool.Point(x, y);
	}

	// how far the place lies from the nearest of many points along the fitted path
	function strayOf(points: cool.ObjectPathPoint[], place: cool.Point): number {
		let nearest = Infinity;
		for (let edge = 0; edge + 1 < points.length; ++edge)
			for (let step = 0; step <= 200; ++step)
				nearest = Math.min(
					nearest,
					pointBetween(points[edge], points[edge + 1], step / 200).distanceTo(
						place,
					),
				);
		return nearest;
	}

	it('a straight stroke becomes one edge from its first place to its last', function () {
		const points = PathFit.fit(
			placesAlong([0, 0], [1000, 0], 20),
			30,
			60,
			false,
		);
		nodeassert.strictEqual(points.length, 2);
		nodeassert.deepStrictEqual([points[0].x, points[0].y], [0, 0]);
		nodeassert.deepStrictEqual([points[1].x, points[1].y], [1000, 0]);
	});

	it('a sharp turn stays a corner between two curves', function () {
		const places = placesAlong([0, 0], [500, 500], 20).concat(
			placesAlong([500, 500], [1000, 0], 20).slice(1),
		);
		const points = PathFit.fit(places, 30, 60, false);
		nodeassert.strictEqual(points.length, 3);
		nodeassert.deepStrictEqual([points[1].x, points[1].y], [500, 500]);
	});

	it('a bend is fitted with few curves that keep within the tolerance', function () {
		const places: cool.Point[] = [];
		for (let step = 0; step <= 60; ++step) {
			const angle = (step / 60) * Math.PI;
			places.push(
				new cool.Point(1000 * Math.cos(angle), 1000 * Math.sin(angle)),
			);
		}
		const points = PathFit.fit(places, 20, 60, false);
		nodeassert.ok(points.length <= 4, 'points: ' + points.length);
		for (const place of places)
			nodeassert.ok(
				strayOf(points, place) <= 20,
				'stray at ' + place.toString(),
			);
	});

	it('a closed round stroke runs smoothly through its first point', function () {
		const places: cool.Point[] = [];
		for (let step = 0; step < 40; ++step) {
			const angle = (step / 40) * 2 * Math.PI;
			places.push(
				new cool.Point(1000 * Math.sin(angle), -1000 * Math.cos(angle)),
			);
		}
		const first = PathFit.fit(places, 20, 60, true)[0];
		const behind = new cool.Point(
			(first.behindX as number) - first.x,
			(first.behindY as number) - first.y,
		);
		const ahead = new cool.Point(
			(first.aheadX as number) - first.x,
			(first.aheadY as number) - first.y,
		);
		nodeassert.ok(
			Math.abs(behind.cross(ahead)) < 1e-6 * behind.length() * ahead.length(),
		);
		nodeassert.ok(behind.dot(ahead) < 0);
	});

	it('a closed stroke does not repeat its first point', function () {
		const places = placesAlong([0, 0], [1000, 0], 20)
			.concat(placesAlong([1000, 0], [1000, 1000], 20).slice(1))
			.concat(placesAlong([1000, 1000], [0, 1000], 20).slice(1))
			.concat(placesAlong([0, 1000], [0, 20], 20).slice(1));
		const points = PathFit.fit(places, 30, 60, true);
		const first = points[0];
		const last = points[points.length - 1];
		nodeassert.ok(first.x !== last.x || first.y !== last.y);
		nodeassert.strictEqual(points.length, 4);
	});
});
