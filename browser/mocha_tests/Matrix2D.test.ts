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

describe('Matrix2D', function () {
	const assert = require('assert');

	function assertPoint(
		actual: { x: number; y: number },
		x: number,
		y: number,
	): void {
		assert.ok(
			Math.abs(actual.x - x) < 1e-9 && Math.abs(actual.y - y) < 1e-9,
			'expected (' +
				x +
				', ' +
				y +
				') but got (' +
				actual.x +
				', ' +
				actual.y +
				')',
		);
	}

	it('leaves a point where it is under the identity', function () {
		assertPoint(cool.Matrix2D.IDENTITY.apply(3, 4), 3, 4);
	});

	it('round-trips the six canvas values through an array', function () {
		const values = [1, 2, 3, 4, 5, 6];
		assert.deepStrictEqual(cool.Matrix2D.fromArray(values).toArray(), values);
	});

	it('gives no matrix for a short or missing array', function () {
		assert.strictEqual(cool.Matrix2D.fromArray([1, 2, 3, 4, 5]), null);
		assert.strictEqual(cool.Matrix2D.fromArray(undefined), null);
	});

	it('scales about the origin', function () {
		assertPoint(cool.Matrix2D.IDENTITY.scale(2, 3).apply(1, 1), 2, 3);
	});

	it('moves a point by the translation', function () {
		assertPoint(cool.Matrix2D.IDENTITY.translate(10, -5).apply(1, 1), 11, -4);
	});

	it('applies operations in the order they are chained', function () {
		// Scale first, then move: the translation is not scaled.
		const scaleThenMove = cool.Matrix2D.IDENTITY.scale(2, 2).translate(10, 0);
		assertPoint(scaleThenMove.apply(1, 0), 12, 0);
		// Move first, then scale: the translation is doubled too.
		const moveThenScale = cool.Matrix2D.IDENTITY.translate(10, 0).scale(2, 2);
		assertPoint(moveThenScale.apply(1, 0), 22, 0);
	});

	it('keeps the pivot in place when rotating around it', function () {
		const quarter = cool.Matrix2D.IDENTITY.rotateAround(5, 5, Math.PI / 2);
		assertPoint(quarter.apply(5, 5), 5, 5);
		// A quarter turn takes a point to the right of the pivot to
		// below it, with y growing downwards.
		assertPoint(quarter.apply(6, 5), 5, 6);
	});

	it('composes a later matrix after an earlier one', function () {
		const scale = cool.Matrix2D.IDENTITY.scale(2, 2);
		const move = cool.Matrix2D.IDENTITY.translate(1, 1);
		assertPoint(scale.then(move).apply(1, 1), 3, 3);
		assertPoint(move.then(scale).apply(1, 1), 4, 4);
	});

	it('does not change a matrix an operation was taken from', function () {
		const base = cool.Matrix2D.IDENTITY.translate(1, 1);
		base.scale(5, 5);
		assertPoint(base.apply(0, 0), 1, 1);
	});

	it('answers a point it applies to as a cool.Point', function () {
		const point = cool.Matrix2D.IDENTITY.translate(1, 2).apply(3, 4);
		assert.ok(point instanceof cool.Point);
		assertPoint(point, 4, 6);
	});

	it('takes the unit square onto a range', function () {
		const onto = cool.Matrix2D.fromRange(new cool.Range2D(10, 20, 40, 30));
		assertPoint(onto.apply(0, 0), 10, 20);
		assertPoint(onto.apply(1, 1), 40, 30);
		assertPoint(onto.apply(0.5, 0.5), 25, 25);
	});

	it('says how long it makes a step along each axis', function () {
		const matrix = cool.Matrix2D.IDENTITY.scale(3, 4).rotateAround(0, 0, 1.1);
		assert.ok(Math.abs(matrix.lengthOfXAxis() - 3) < 1e-12);
		assert.ok(Math.abs(matrix.lengthOfYAxis() - 4) < 1e-12);
	});

	it('says by how much it turns the x axis', function () {
		const quarter = cool.Matrix2D.IDENTITY.rotateAround(3, 4, Math.PI / 2);
		assert.ok(Math.abs(quarter.rotation() - Math.PI / 2) < 1e-12);
		assert.strictEqual(cool.Matrix2D.IDENTITY.scale(2, 5).rotation(), 0);
	});

	it('is the same as another to within a difference in each value', function () {
		const one = new cool.Matrix2D(1, 0, 0, 1, 100, 200);
		assert.ok(one.equals(new cool.Matrix2D(1, 0, 0, 1, 101, 199), 1.5));
		assert.ok(!one.equals(new cool.Matrix2D(1, 0, 0, 1, 102, 200), 1.5));
	});

	it('takes a point back where it came from under the inverse', function () {
		const there = cool.Matrix2D.IDENTITY.scale(2, 3)
			.rotateAround(5, 5, 0.7)
			.translate(10, -4);
		const back = there.invert();
		const moved = there.apply(7, 11);
		assertPoint(back.apply(moved.x, moved.y), 7, 11);
	});

	it('has no inverse when it flattens everything onto a line', function () {
		assert.strictEqual(new cool.Matrix2D(1, 2, 2, 4, 0, 0).invert(), null);
	});

	it('multiplies its values into a canvas context', function () {
		const context = new CanvasRecorder(10, 10);
		cool.Matrix2D.fromArray([1, 2, 3, 4, 5, 6]).applyTo(context as any);
		const call = context.callsOf('transform')[0];
		assert.deepStrictEqual(call.args, [1, 2, 3, 4, 5, 6]);
	});
});
