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

describe('VectorHitTest', function () {
	const assert = require('assert');

	let originalPath2D: any;
	let originalContext: any;

	const fakeContext = new HitTestContext();

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

	// Two entries that overlap from (100, 100) to (200, 200), the second painted over the first.
	function overlappingEntries(coordinates?: cool.VectorCoordinates): any {
		const entry = (id: number, x: number, y: number): any => ({
			id,
			x,
			y,
			width: 200,
			height: 200,
			primitives: [],
		});
		return {
			partId: 'P',
			mode: 0,
			coordinates,
			slideWidth: 1000,
			slideHeight: 1000,
			objects: new Map([
				[1, entry(1, 0, 0)],
				[2, entry(2, 100, 100)],
			]),
			order: [1, 2],
		};
	}

	const originOf =
		(data: cool.VectorPrimitivesData) =>
		(entry: cool.SlideObject): { x: number; y: number } | null =>
			VectorPartManagerBase.entryOrigin(data, entry);

	it('finds the entry painted last among the entries whose box holds the point', function () {
		const data = overlappingEntries();

		const top = cool.VectorHitTest.topmostEntryAt(
			data,
			at(150, 150),
			() => true,
			originOf(data),
			() => true,
		);

		assert.strictEqual(top?.id, 2);
		// For a part in page coordinates, the primitives are tested at the point given for the part.
		assert.strictEqual(top?.position.x, 150);
		assert.strictEqual(top?.position.y, 150);
	});

	it('asks the primitives of an entry in entry coordinates from the top left of its box', function () {
		const data = overlappingEntries('entry');

		const top = cool.VectorHitTest.topmostEntryAt(
			data,
			at(150, 160),
			() => true,
			originOf(data),
			() => true,
		);

		assert.strictEqual(top?.id, 2);
		assert.strictEqual(top?.position.x, 50);
		assert.strictEqual(top?.position.y, 60);
		assert.strictEqual(top?.range.minX, 0);
		assert.strictEqual(top?.range.maxX, 200);
	});

	it('goes on to the entry behind one that is turned down or does not take the point', function () {
		const data = overlappingEntries();

		const accepted = cool.VectorHitTest.topmostEntryAt(
			data,
			at(150, 150),
			(_entry, id) => id !== 2,
			originOf(data),
			() => true,
		);
		assert.strictEqual(accepted?.id, 1);

		const taken = cool.VectorHitTest.topmostEntryAt(
			data,
			at(150, 150),
			() => true,
			originOf(data),
			(entryAt) => entryAt.id !== 2,
		);
		assert.strictEqual(taken?.id, 1);

		assert.strictEqual(
			cool.VectorHitTest.topmostEntryAt(
				data,
				at(500, 500),
				() => true,
				originOf(data),
				() => true,
			),
			undefined,
		);
	});
});
