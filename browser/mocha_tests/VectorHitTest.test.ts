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

describe('VectorHitTest', function () {
	const assert = require('assert');

	let originalPath2D: any;
	let originalContext: any;

	// Node has no drawing context. This stand-in answers from what the path
	// recorder holds: rectangles, and paths added through a transform.
	function isPointInPath(path: any, x: number, y: number): boolean {
		return path.calls.some((call: any) => {
			if (call.method === 'rect') {
				const [left, top, width, height] = call.args;
				return x >= left && x <= left + width && y >= top && y <= top + height;
			}
			if (call.method === 'addPath') {
				const [inner, m] = call.args;
				const back = new cool.Matrix2D(m.a, m.b, m.c, m.d, m.e, m.f).invert();
				if (!back) return false;
				const at = back.apply(x, y);
				return isPointInPath(inner, at.x, at.y);
			}
			return false;
		});
	}

	const fakeContext = {
		isPointInPath: (path: any, x: number, y: number) =>
			isPointInPath(path, x, y),
		isPointInStroke: () => false,
		save: () => {},
		restore: () => {},
	};

	before(function () {
		originalPath2D = (globalThis as any).Path2D;
		(globalThis as any).Path2D = Path2DRecorder;
		originalContext = (cool.VectorHitTest as any)._context;
		(cool.VectorHitTest as any)._context = fakeContext;
	});

	after(function () {
		(globalThis as any).Path2D = originalPath2D;
		(cool.VectorHitTest as any)._context = originalContext;
	});

	function at(x: number, y: number): cool.HitPosition {
		return { x: x, y: y, toleranceX: 0, toleranceY: 0 };
	}

	// A background covers the area of its object, however a transform moves
	// its children.
	it('hits a background inside a transform over the area of the object', function () {
		const primitives: any[] = [
			{
				type: cool.TransformPrimitive.type,
				matrix: [1, 0, 0, 1, 1000, 0],
				children: [{ type: cool.BackgroundColorPrimitive.type }],
			},
		];
		const range = new cool.Range2D(0, 0, 100, 100);

		const inside = cool.VectorHitTest.hit(primitives, at(50, 50), range);
		assert.ok(inside, 'a point over the object missed its background');
		assert.strictEqual(inside?.kind, 'fill');

		assert.strictEqual(
			cool.VectorHitTest.hit(primitives, at(1050, 50), range),
			undefined,
			'a point where the transform would move the area hit the background',
		);
	});

	it('hits a background with no transform over the area of the object', function () {
		const primitives: any[] = [{ type: cool.BackgroundColorPrimitive.type }];
		const range = new cool.Range2D(0, 0, 100, 100);

		assert.ok(cool.VectorHitTest.hit(primitives, at(50, 50), range));
		assert.strictEqual(
			cool.VectorHitTest.hit(primitives, at(150, 50), range),
			undefined,
		);
	});
});
