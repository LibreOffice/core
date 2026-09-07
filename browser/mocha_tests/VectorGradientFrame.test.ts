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

describe('VectorGradientFrame', function () {
	const assert = require('assert');

	// The matrix maps the ramp's own space to twips, so applying it to a
	// point in ramp space says where that point lands on the slide.
	function mapPoint(
		frame: cool.GradientFrame,
		x: number,
		y: number,
	): { x: number; y: number } {
		return frame.matrix.apply(x, y);
	}

	function assertNear(actual: number, expected: number, what: string): void {
		assert.ok(
			Math.abs(actual - expected) < 1e-9,
			what + ': expected ' + expected + ' but got ' + actual,
		);
	}

	const range = new cool.Range2D(1000, 2000, 3000, 5000);

	it('gives no frame for a range with no area', function () {
		assert.strictEqual(
			cool.VectorGradientFrame.create(
				{ style: 'linear' },
				new cool.Range2D(10, 10, 10, 20),
			),
			null,
		);
		assert.strictEqual(
			cool.VectorGradientFrame.create({ style: 'linear' }, null),
			null,
		);
	});

	it('runs a linear ramp down the range from y zero to y one', function () {
		const frame = cool.VectorGradientFrame.create({ style: 'linear' }, range);
		assert.ok(frame);
		const start = mapPoint(frame, 0, 0);
		const end = mapPoint(frame, 0, 1);
		assertNear(start.x, range.minX, 'ramp start x');
		assertNear(start.y, range.minY, 'ramp start y');
		assertNear(end.y, range.maxY, 'ramp end y');
		assertNear(frame.aspectRatio, 2000 / 3000, 'aspect ratio');
	});

	it('holds the first color flat over the border of a linear ramp', function () {
		const frame = cool.VectorGradientFrame.create(
			{ style: 'linear', border: 0.25 },
			range,
		);
		assert.ok(frame);
		// The ramp now starts a quarter of the way down and still ends
		// at the bottom edge, leaving the border to the flat color.
		assertNear(mapPoint(frame, 0, 0).y, 2000 + 0.25 * 3000, 'ramp start y');
		assertNear(mapPoint(frame, 0, 1).y, range.maxY, 'ramp end y');
	});

	it('runs an axial ramp out to both edges from the middle', function () {
		const frame = cool.VectorGradientFrame.create({ style: 'axial' }, range);
		assert.ok(frame);
		assertNear(mapPoint(frame, 0, 0).y, (2000 + 5000) / 2, 'middle');
		assertNear(mapPoint(frame, 0, -1).y, range.minY, 'top edge');
		assertNear(mapPoint(frame, 0, 1).y, range.maxY, 'bottom edge');
	});

	it('grows a radial ramp to the diagonal so the ring reaches the corners', function () {
		const frame = cool.VectorGradientFrame.create({ style: 'radial' }, range);
		assert.ok(frame);
		const diagonal = Math.hypot(2000, 3000);
		// The circle stays a circle, so both axes carry the diagonal.
		assertNear(frame.aspectRatio, 1, 'aspect ratio');
		const center = mapPoint(frame, 0, 0);
		assertNear(center.x, (1000 + 3000) / 2, 'center x');
		assertNear(center.y, (2000 + 5000) / 2, 'center y');
		assertNear(mapPoint(frame, 1, 0).x - center.x, diagonal / 2, 'reach');
	});

	it('moves the center of a round ramp by the offsets', function () {
		const plain = cool.VectorGradientFrame.create({ style: 'radial' }, range);
		const moved = cool.VectorGradientFrame.create(
			{ style: 'radial', offsetX: 0.75, offsetY: 0.25 },
			range,
		);
		assert.ok(plain && moved);
		// The offsets are fractions of the range as it was given.
		assertNear(
			mapPoint(moved, 0, 0).x - mapPoint(plain, 0, 0).x,
			0.25 * 2000,
			'center shift x',
		);
		assertNear(
			mapPoint(moved, 0, 0).y - mapPoint(plain, 0, 0).y,
			-0.25 * 3000,
			'center shift y',
		);
	});

	it('grows a square ramp to the longer side of the range', function () {
		const frame = cool.VectorGradientFrame.create({ style: 'square' }, range);
		assert.ok(frame);
		assertNear(frame.aspectRatio, 1, 'aspect ratio');
		const center = mapPoint(frame, 0, 0);
		assertNear(mapPoint(frame, 1, 0).x - center.x, 3000 / 2, 'reach in x');
		assertNear(mapPoint(frame, 0, 1).y - center.y, 3000 / 2, 'reach in y');
	});

	it('keeps a square ramp at the longer side when the angle turns it', function () {
		const frame = cool.VectorGradientFrame.create(
			{ style: 'square', angle: 45 },
			range,
		);
		assert.ok(frame);
		const center = mapPoint(frame, 0, 0);
		const edge = mapPoint(frame, 1, 0);
		assertNear(center.x, (1000 + 3000) / 2, 'center x');
		assertNear(center.y, (2000 + 5000) / 2, 'center y');
		assertNear(
			Math.hypot(edge.x - center.x, edge.y - center.y),
			3000 / 2,
			'reach',
		);
	});

	it('widens a rect ramp so it still covers the range when it is turned', function () {
		const frame = cool.VectorGradientFrame.create(
			{ style: 'rect', angle: 45 },
			range,
		);
		assert.ok(frame);
		const center = mapPoint(frame, 0, 0);
		const edge = mapPoint(frame, 1, 0);
		assertNear(
			Math.hypot(edge.x - center.x, edge.y - center.y),
			((2000 + 3000) * Math.SQRT1_2) / 2,
			'reach',
		);
	});

	it('keeps the shape of the range for a rect ramp', function () {
		const frame = cool.VectorGradientFrame.create({ style: 'rect' }, range);
		assert.ok(frame);
		assertNear(frame.aspectRatio, 2000 / 3000, 'aspect ratio');
	});

	it('turns a linear ramp by the angle and grows it to keep covering', function () {
		// A quarter turn swaps which side of the range the ramp runs
		// along, so it now spans the width.
		const frame = cool.VectorGradientFrame.create(
			{ style: 'linear', angle: 90 },
			range,
		);
		assert.ok(frame);
		const start = mapPoint(frame, 0, 0);
		const end = mapPoint(frame, 0, 1);
		assertNear(end.y - start.y, 0, 'ramp no longer runs in y');
		assertNear(Math.abs(end.x - start.x), 2000, 'ramp runs across the width');
	});
});
