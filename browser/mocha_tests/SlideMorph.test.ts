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

describe('SlideMorph', function () {
	// A slide of the given objects in paint order after its page entry, each an id, a name
	// and a box [x, y, width, height] in twips.
	function slide(objects: [number, string, number[]][]): any {
		const entries: [number, any][] = [[1, { id: 1, parent: -1, kind: 'page' }]];
		for (const [id, name, [x, y, width, height]] of objects)
			entries.push([id, { id, name, parent: 0, x, y, width, height }]);
		return {
			order: entries.map(([id]) => id),
			objects: new Map(entries),
		};
	}

	// The point the six canvas matrix values take x, y to.
	function moved(transform: number[], x: number, y: number): number[] {
		return [0, 1].map(
			(axis) =>
				Math.round(
					(transform[axis] * x +
						transform[axis + 2] * y +
						transform[axis + 4]) *
						1000,
				) / 1000,
		);
	}

	it('pairs the objects of two slides that carry the same name', function () {
		const morph = new SlideMorph(
			slide([
				[10, 'Title', [0, 0, 100, 100]],
				[11, 'Rectangle 3', [0, 0, 100, 100]],
				[12, 'Gone', [0, 0, 100, 100]],
			]),
			slide([
				[20, 'Rectangle 3', [0, 0, 100, 100]],
				[21, 'New', [0, 0, 100, 100]],
				[22, 'Title', [0, 0, 100, 100]],
			]),
		);

		nodeassert.deepStrictEqual(morph.pairs, [
			{ leaving: 10, entering: 22 },
			{ leaving: 11, entering: 20 },
		]);
		nodeassert.deepStrictEqual(morph.leavingAlone, [12]);
		nodeassert.deepStrictEqual(morph.enteringAlone, [21]);
	});

	it('pairs objects that share a name on one slide in paint order', function () {
		const morph = new SlideMorph(
			slide([
				[10, 'Box', [0, 0, 100, 100]],
				[11, 'Box', [0, 0, 100, 100]],
			]),
			slide([
				[20, 'Box', [0, 0, 100, 100]],
				[21, 'Box', [0, 0, 100, 100]],
				[22, 'Box', [0, 0, 100, 100]],
			]),
		);

		nodeassert.deepStrictEqual(morph.pairs, [
			{ leaving: 10, entering: 20 },
			{ leaving: 11, entering: 21 },
		]);
		nodeassert.deepStrictEqual(morph.enteringAlone, [22]);
	});

	it('leaves objects without a name unpaired', function () {
		const morph = new SlideMorph(
			slide([[10, '', [0, 0, 100, 100]]]),
			slide([[20, '', [0, 0, 100, 100]]]),
		);

		nodeassert.deepStrictEqual(morph.pairs, []);
		nodeassert.deepStrictEqual(morph.leavingAlone, [10]);
		nodeassert.deepStrictEqual(morph.enteringAlone, [20]);
	});

	it('moves both copies of a pair halfway at the middle of the morph', function () {
		const morph = new SlideMorph(
			slide([[10, 'Box', [0, 0, 100, 100]]]),
			slide([[20, 'Box', [1000, 2000, 300, 100]]]),
		);

		const [leaving, entering] = morph.states(0.5);

		// The middle goes from 50, 50 to 1150, 2050, and the width from 100 to 300.
		nodeassert.deepStrictEqual(
			moved(leaving.get(10).transform, 50, 50),
			[600, 1050],
		);
		nodeassert.deepStrictEqual(
			moved(leaving.get(10).transform, 100, 50),
			[700, 1050],
		);
		nodeassert.deepStrictEqual(
			moved(entering.get(20).transform, 1150, 2050),
			[600, 1050],
		);
		nodeassert.deepStrictEqual(
			moved(entering.get(20).transform, 1300, 2050),
			[700, 1050],
		);
		nodeassert.strictEqual(leaving.get(10).alpha, 1);
		nodeassert.strictEqual(entering.get(20).alpha, 1);
	});

	it('shows the entering slide as it is at the end of the morph', function () {
		const morph = new SlideMorph(
			slide([
				[10, 'Box', [0, 0, 100, 100]],
				[11, 'Gone', [0, 0, 100, 100]],
			]),
			slide([
				[20, 'Box', [1000, 2000, 300, 100]],
				[21, 'New', [0, 0, 100, 100]],
			]),
		);

		const [leaving, entering] = morph.states(1);

		nodeassert.deepStrictEqual(
			moved(entering.get(20).transform, 1000, 2000),
			[1000, 2000],
		);
		nodeassert.deepStrictEqual(
			moved(entering.get(20).transform, 1300, 2100),
			[1300, 2100],
		);
		nodeassert.strictEqual(entering.get(20).alpha, 1);
		nodeassert.strictEqual(entering.get(21).alpha, 1);
		nodeassert.strictEqual(leaving.get(10).alpha, 0);
		nodeassert.strictEqual(leaving.get(11).alpha, 0);
	});

	it('turns an object the shorter way round', function () {
		const leavingSlide = slide([[10, 'Box', [0, 0, 100, 100]]]);
		const enteringSlide = slide([[20, 'Box', [0, 0, 100, 100]]]);
		// Turned by 350 degrees, which is 10 degrees back from where it started.
		const angle = (350 * Math.PI) / 180;
		enteringSlide.objects.get(20).transform = cool.Matrix2D.fromRange(
			new cool.Range2D(0, 0, 100, 100),
		)
			.rotateAround(50, 50, angle)
			.toArray();
		const morph = new SlideMorph(leavingSlide, enteringSlide);

		const [leaving] = morph.states(0.5);

		// Halfway is 5 degrees back, so the right edge's middle goes up, not round the far side.
		const [x, y] = moved(leaving.get(10).transform, 100, 50);
		const back = (5 * Math.PI) / 180;
		nodeassert.ok(Math.abs(x - (50 + 50 * Math.cos(back))) < 0.01, 'x ' + x);
		nodeassert.ok(Math.abs(y - (50 - 50 * Math.sin(back))) < 0.01, 'y ' + y);
	});

	it('draws the two copies of a pair together in the paint order of the entering slide', function () {
		const morph = new SlideMorph(
			slide([
				[10, 'A', [0, 0, 100, 100]],
				[11, 'Gone', [0, 0, 100, 100]],
				[12, 'B', [0, 0, 100, 100]],
			]),
			slide([
				[20, 'B', [0, 0, 100, 100]],
				[21, 'New', [0, 0, 100, 100]],
				[22, 'A', [0, 0, 100, 100]],
			]),
		);

		nodeassert.deepStrictEqual(morph.paintOrder, [
			{ leaving: true, id: 11 },
			{ leaving: true, id: 12 },
			{ leaving: false, id: 20 },
			{ leaving: false, id: 21 },
			{ leaving: true, id: 10 },
			{ leaving: false, id: 22 },
		]);
	});

	it('flips an object that is mirrored on one slide only without turning it', function () {
		const leavingSlide = slide([[10, 'Box', [0, 0, 100, 100]]]);
		const enteringSlide = slide([[20, 'Box', [0, 0, 100, 100]]]);
		// Mirrored left to right in the same box.
		enteringSlide.objects.get(20).transform = [-100, 0, 0, 100, 100, 0];
		const morph = new SlideMorph(leavingSlide, enteringSlide);

		const [leaving] = morph.states(0.5);

		// Halfway the box is flat on its vertical middle line, and its top edge stays level.
		nodeassert.deepStrictEqual(
			moved(leaving.get(10).transform, 100, 50),
			[50, 50],
		);
		nodeassert.deepStrictEqual(
			moved(leaving.get(10).transform, 50, 0),
			[50, 0],
		);
	});
});
