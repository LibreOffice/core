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

	it('multiplies its values into a canvas context', function () {
		const context = new CanvasRecorder(10, 10);
		cool.Matrix2D.fromArray([1, 2, 3, 4, 5, 6]).applyTo(context as any);
		const call = context.callsOf('transform')[0];
		assert.deepStrictEqual(call.args, [1, 2, 3, 4, 5, 6]);
	});
});
