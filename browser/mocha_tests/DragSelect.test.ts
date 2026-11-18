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

describe('LassoSelect and the outline it takes', function () {
	const points = (places: number[][]) =>
		places.map((place: number[]) => new cool.Point(place[0], place[1]));
	const square = points([
		[40, 40],
		[60, 40],
		[60, 60],
		[40, 60],
	]);

	it('a lasso round a shape holds it', function () {
		const lasso = points([
			[0, 0],
			[100, 0],
			[100, 100],
			[0, 100],
		]);
		nodeassert.strictEqual(
			cool.VectorOutline.holds(lasso, [{ closed: true, points: square }]),
			true,
		);
	});

	it('a lasso across a shape does not hold it', function () {
		const lasso = points([
			[0, 0],
			[50, 0],
			[50, 100],
			[0, 100],
		]);
		nodeassert.strictEqual(
			cool.VectorOutline.holds(lasso, [{ closed: true, points: square }]),
			false,
		);
	});

	it('a lasso bending in between the corners of a shape does not hold it', function () {
		// every corner of the square lies inside, but a notch reaches into its middle
		const lasso = points([
			[0, 0],
			[100, 0],
			[100, 100],
			[52, 100],
			[50, 50],
			[48, 100],
			[0, 100],
		]);
		nodeassert.strictEqual(
			cool.VectorOutline.holds(lasso, [{ closed: true, points: square }]),
			false,
		);
	});

	it('what primitives draw is tested, through their transforms', function () {
		const lasso = points([
			[0, 0],
			[100, 0],
			[100, 100],
			[0, 100],
		]);
		// a filled triangle, moved by a transform to lie inside the lasso, or across its edge
		const triangle = (dx: number) => [
			{
				type: 'transform',
				matrix: [1, 0, 0, 1, dx, 0],
				children: [
					{
						type: 'polyPolygonColor',
						color: '#000000',
						path: 'M 10 10 L 40 10 L 25 40 Z',
					},
				],
			},
		];
		nodeassert.strictEqual(
			cool.VectorOutline.liesInside(triangle(20) as cool.Primitive[], lasso),
			true,
		);
		nodeassert.strictEqual(
			cool.VectorOutline.liesInside(triangle(80) as cool.Primitive[], lasso),
			false,
		);
	});

	it('a curve bulging out of the lasso is not held, though its ends are', function () {
		const lasso = points([
			[0, 0],
			[100, 0],
			[100, 100],
			[0, 100],
		]);
		const curve = [
			{
				type: 'polygonHairline',
				color: '#000000',
				path: 'M 20 50 C 20 200 80 200 80 50',
			},
		];
		nodeassert.strictEqual(
			cool.VectorOutline.liesInside(curve as cool.Primitive[], lasso),
			false,
		);
	});

	it('the geometry held for the hit test alone counts, as a text frame holds it', function () {
		const lasso = points([
			[0, 0],
			[100, 0],
			[100, 100],
			[0, 100],
		]);
		const frame = (width: number) => [
			{
				type: 'hiddenGeometry',
				children: [
					{
						type: 'polyPolygonColor',
						color: '#000000',
						path: 'M 10 10 L ' + width + ' 10 L ' + width + ' 40 L 10 40 Z',
					},
				],
			},
		];
		nodeassert.strictEqual(
			cool.VectorOutline.liesInside(frame(90) as cool.Primitive[], lasso),
			true,
		);
		nodeassert.strictEqual(
			cool.VectorOutline.liesInside(frame(120) as cool.Primitive[], lasso),
			false,
		);
	});

	it('primitives that draw no geometry leave the answer open', function () {
		const lasso = points([
			[0, 0],
			[100, 0],
			[100, 100],
		]);
		nodeassert.strictEqual(cool.VectorOutline.liesInside([], lasso), undefined);
	});

	it('a lasso of fewer than three places holds nothing', function () {
		nodeassert.strictEqual(
			cool.VectorOutline.holds(
				points([
					[0, 0],
					[100, 100],
				]),
				[{ closed: true, points: square }],
			),
			false,
		);
	});

	it('Ctrl or Cmd, with Shift or without, draws a lasso, and nothing else does', function () {
		// the numbers of the modifiers, as the document state gives them in the page
		const given = app.UNOModifier;
		app.UNOModifier = { SHIFT: 4096, CTRL: 8192, ALT: 16384, CTRLMAC: 32768 };
		const shift = app.UNOModifier.SHIFT;
		const ctrl = app.UNOModifier.CTRL;
		nodeassert.strictEqual(LassoSelect.drawnWith(ctrl), true);
		nodeassert.strictEqual(LassoSelect.drawnWith(ctrl | shift), true);
		nodeassert.strictEqual(
			LassoSelect.drawnWith(app.UNOModifier.CTRLMAC),
			true,
		);
		nodeassert.strictEqual(LassoSelect.drawnWith(0), false);
		nodeassert.strictEqual(LassoSelect.drawnWith(shift), false);
		nodeassert.strictEqual(
			LassoSelect.drawnWith(ctrl | app.UNOModifier.ALT),
			false,
		);
		app.UNOModifier = given;
	});
});
