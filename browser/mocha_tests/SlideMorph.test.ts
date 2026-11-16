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

describe('SlideMorph', function () {
	// A slide of the given objects in paint order after its page entry, each an id, a name
	// and a box [x, y, width, height] in twips. An object is a rectangle, unless a geometry
	// type of a custom shape is given after its box.
	function slide(objects: [number, string, number[], string?][]): any {
		const entries: [number, any][] = [[1, { id: 1, parent: -1, kind: 'page' }]];
		for (const [id, name, [x, y, width, height], shapeType] of objects)
			entries.push([
				id,
				{
					id,
					name,
					parent: 0,
					x,
					y,
					width,
					height,
					objectKind: 33,
					shapeType: shapeType ?? 'rect',
				},
			]);
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
				[12, 'Gone', [0, 0, 100, 100], 'ellipse'],
			]),
			slide([
				[20, 'Rectangle 3', [0, 0, 100, 100]],
				[21, 'New', [0, 0, 100, 100], 'star5'],
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

	it('pairs objects without a name by their kind', function () {
		const morph = new SlideMorph(
			slide([
				[10, '', [0, 0, 100, 100]],
				[11, '', [0, 0, 100, 100], 'ellipse'],
			]),
			slide([[20, '', [500, 0, 100, 100]]]),
		);

		nodeassert.deepStrictEqual(morph.pairs, [{ leaving: 10, entering: 20 }]);
		nodeassert.deepStrictEqual(morph.leavingAlone, [11]);
		nodeassert.deepStrictEqual(morph.enteringAlone, []);
	});

	it('leaves out the objects that an animation hides', function () {
		const morph = new SlideMorph(
			slide([[10, 'Box', [0, 0, 100, 100]]]),
			slide([
				[20, 'Box', [0, 0, 100, 100]],
				[21, '', [0, 0, 100, 100]],
			]),
			undefined,
			new Set([20]),
		);

		nodeassert.deepStrictEqual(morph.pairs, [{ leaving: 10, entering: 21 }]);
		nodeassert.deepStrictEqual(morph.enteringAlone, []);
		nodeassert.ok(!morph.paintOrder.some((step) => step.id === 20));
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
				[11, 'Gone', [0, 0, 100, 100], 'ellipse'],
			]),
			slide([
				[20, 'Box', [1000, 2000, 300, 100]],
				[21, 'New', [0, 0, 100, 100], 'star5'],
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
				[11, 'Gone', [0, 0, 100, 100], 'ellipse'],
				[12, 'B', [0, 0, 100, 100]],
			]),
			slide([
				[20, 'B', [0, 0, 100, 100]],
				[21, 'New', [0, 0, 100, 100], 'star5'],
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

	it('moves the words that read the same from one object into another', function () {
		// A word of one line of text in slide twips, its box 100 wide and 50 high at x, y.
		const word = (key: number, owner: number, text: string, x: number) => ({
			key,
			owner,
			piece: {
				text,
				paragraph: 0,
				frame: new cool.Matrix2D(100, 0, 0, 50, x, 0),
				clip: new cool.Matrix2D(100, 0, 0, 50, x, 0),
			},
		});
		const leavingPieces = [word(-1, 10, 'Hello', 0)];
		const enteringPieces = [
			word(-1, 20, 'World', 0),
			word(-2, 20, 'Hello', 1000),
		];
		const morph = new SlideMorph(
			slide([[10, 'Box', [0, 0, 500, 100]]]),
			slide([[20, 'Box', [0, 0, 1500, 100]]]),
			undefined,
			undefined,
			{ leaving: leavingPieces, entering: enteringPieces, splitsText: true },
		);

		// The objects stay where they are, without their text.
		nodeassert.deepStrictEqual(morph.pairs, []);
		nodeassert.deepStrictEqual(Array.from(morph.leavingSplit), [10]);
		nodeassert.deepStrictEqual(Array.from(morph.enteringSplit), [20]);

		const [leaving, entering] = morph.states(0.5);
		nodeassert.deepStrictEqual(
			moved(leaving.get(-1).transform, 0, 0),
			[500, 0],
		);
		nodeassert.deepStrictEqual(
			moved(entering.get(-2).transform, 1000, 0),
			[500, 0],
		);
		// The word on the entering slide only shows from the middle of the morph on.
		nodeassert.strictEqual(entering.get(-1).alpha, 0);
		nodeassert.strictEqual(morph.states(1)[1].get(-1).alpha, 1);
	});

	it('keeps the size of the words while their object is resized', function () {
		// A word in slide twips, its box 200 wide and 100 high at x, y.
		const word = (
			key: number,
			owner: number,
			text: string,
			x: number,
			y: number,
		) => ({
			key,
			owner,
			piece: {
				text,
				paragraph: 0,
				frame: new cool.Matrix2D(200, 0, 0, 100, x, y),
				clip: new cool.Matrix2D(200, 0, 0, 100, x, y),
			},
		});
		const morph = new SlideMorph(
			slide([[10, 'Box', [0, 0, 1000, 500]]]),
			slide([[20, 'Box', [2000, 0, 2000, 1000]]]),
			undefined,
			undefined,
			{
				leaving: [word(-1, 10, 'Hello', 100, 200)],
				entering: [
					word(-1, 20, 'Hello', 2100, 400),
					word(-2, 20, 'World', 2400, 400),
				],
				splitsText: false,
			},
		);

		nodeassert.deepStrictEqual(morph.pairs, [{ leaving: 10, entering: 20 }]);
		nodeassert.deepStrictEqual(Array.from(morph.leavingSplit), [10]);

		const [leaving, entering] = morph.states(0.5);
		// The object doubles in size, while the word keeps its width of 200 and goes halfway
		// from its place in the leaving text to its place in the entering one.
		nodeassert.deepStrictEqual(
			moved(leaving.get(-1).transform, 100, 200),
			[1100, 300],
		);
		nodeassert.deepStrictEqual(
			moved(leaving.get(-1).transform, 300, 200),
			[1300, 300],
		);
		nodeassert.deepStrictEqual(
			moved(entering.get(-1).transform, 2100, 400),
			[1100, 300],
		);
		// The word that only the entering text has stays in its place in the text and shows
		// from the middle of the morph on.
		nodeassert.strictEqual(entering.get(-2).alpha, 0);
		nodeassert.deepStrictEqual(
			moved(morph.states(1)[1].get(-2).transform, 2400, 400),
			[2400, 400],
		);
	});
});
