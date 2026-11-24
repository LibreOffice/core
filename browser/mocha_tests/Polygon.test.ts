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

describe('Polygon2D and Polygon3D', function () {
	const assert = require('assert');

	function square(from: number, to: number): cool.Polygon2D {
		return cool.Polygon2D.fromFlat(
			[from, from, to, from, to, to, from, to],
			true,
		);
	}

	it('a point inside a square is inside, one beside it is not', function () {
		const area = square(0, 10);
		assert.strictEqual(area.isInside(5, 5), true);
		assert.strictEqual(area.isInside(15, 5), false);
	});

	it('a point in the hole of an area is outside it, by the even-odd rule', function () {
		const ring = new cool.PolyPolygon2D([square(0, 10), square(3, 7)]);
		assert.strictEqual(ring.isInside(1, 1), true, 'on the ring');
		assert.strictEqual(ring.isInside(5, 5), false, 'in the hole');
	});

	it('growing a square moves each corner that far along the middle of its edges', function () {
		const grown = square(0, 10).grownInNormalDirection(Math.SQRT2);
		const outwards = grown.range().width > 10;
		const range = grown.range();
		// a corner goes along the diagonal, so it moves one along each axis
		assert.ok(
			Math.abs(range.width - (outwards ? 12 : 8)) < 1e-9,
			String(range.width),
		);
		assert.ok(
			Math.abs(
				square(0, 10).grownInNormalDirection(-Math.SQRT2).range().width -
					(outwards ? 8 : 12),
			) < 1e-9,
		);
	});

	it('scaling about the middle keeps the middle where it was', function () {
		const scaled = new cool.PolyPolygon2D([square(0, 10)]).scaledOnTheCenter(
			0.5,
		);
		const range = scaled.range();
		assert.deepStrictEqual(
			[range.minX, range.minY, range.maxX, range.maxY],
			[2.5, 2.5, 7.5, 7.5],
		);
	});

	it('the way between two polygons is taken point by point', function () {
		const half = square(0, 10).interpolated(square(10, 20), 0.5);
		assert.deepStrictEqual([half.points[0].x, half.points[0].y], [5, 5]);
		assert.deepStrictEqual([half.points[2].x, half.points[2].y], [15, 15]);
	});

	it('a 2D outline placed in space lies in the plane the transform takes it to', function () {
		const placed = new cool.PolyPolygon2D([square(0, 10)]).inSpace(
			cool.Matrix3D.translation(0, 0, 7),
		);
		for (const point of placed.polygons[0].points)
			assert.strictEqual(point.z, 7);
	});

	it('the normal of a square in the plane stands on it, twice as long as its area', function () {
		const flat = cool.Polygon3D.fromFlat(
			[0, 0, 0, 2, 0, 0, 2, 2, 0, 0, 2, 0],
			true,
		);
		const normal = flat.normal();
		assert.deepStrictEqual([normal.x, normal.y, Math.abs(normal.z)], [0, 0, 8]);
		assert.strictEqual(flat.isFlat(), false);
	});

	it('a polygon with all its points on one line has no area', function () {
		const line = cool.Polygon3D.fromFlat(
			[0, 0, 0, 1, 1, 1, 2, 2, 2, 1, 1, 1],
			true,
		);
		assert.strictEqual(line.isFlat(), true);
	});

	it('two polygons are equal within a tolerance, and a turned one is not', function () {
		const one = cool.Polygon3D.fromFlat([0, 0, 0, 1, 0, 0, 1, 1, 0], true);
		const near = cool.Polygon3D.fromFlat([0, 0, 1e-12, 1, 0, 0, 1, 1, 0], true);
		assert.strictEqual(one.equals(near, 1e-9), true);
		assert.strictEqual(one.equals(one.reversed(), 1e-9), false);
	});

	it('a turn about y takes the x axis towards minus z', function () {
		const turned = cool.Matrix3D.turnAboutY(Math.PI / 2).apply(
			new cool.Point3D(1, 0, 0),
		);
		assert.ok(
			Math.abs(turned.x) < 1e-12 && Math.abs(turned.z + 1) < 1e-12,
			String(turned.z),
		);
	});
});
