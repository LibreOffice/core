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

describe('MorphMatcher', function () {
	const rectangle = { objectKind: 33, shapeType: 'rect' };
	const ellipse = { objectKind: 33, shapeType: 'ellipse' };
	const textBox = { objectKind: 16 };
	const picture = { objectKind: 22 };

	// A slide with a page entry first, then the given objects in paint order.
	function slide(objects: any[]): any {
		const all = [{ id: 0, kind: 'page', parent: -1 }].concat(
			objects.map((object) => Object.assign({ parent: 0 }, object)),
		);
		return {
			order: all.map((object) => object.id),
			objects: new Map(all.map((object) => [object.id, object])),
		};
	}

	// An object of the given kind with its painted rectangle at x, y.
	function at(id: number, kind: any, x: number, y: number, extra?: any): any {
		return Object.assign({ id, x, y, width: 1000, height: 1000 }, kind, extra);
	}

	// The primitives of a text with one paragraph per string.
	function text(...paragraphs: string[]): any {
		return {
			primitives: [
				{
					type: 'textHierarchyBlock',
					children: paragraphs.map((paragraph) => ({
						type: 'textHierarchyParagraph',
						children: [
							{
								type: 'textHierarchyLine',
								children: [
									{
										type: 'textSimplePortion',
										text: paragraph,
										textPosition: 0,
										textLength: paragraph.length,
									},
								],
							},
						],
					})),
				},
			],
		};
	}

	function assertFrame(frame: cool.Matrix2D, expected: number[]) {
		const actual = frame.toArray();
		for (let i = 0; i < 6; i++)
			nodeassert.ok(
				Math.abs(actual[i] - expected[i]) < 1e-6,
				'value ' + i + ' is ' + actual[i] + ', expected ' + expected[i],
			);
	}

	describe('match', function () {
		it('takes part with the objects on the slide only', function () {
			const leaving = slide([
				at(10, rectangle, 0, 0),
				at(11, rectangle, 0, 0, { emptyPlaceholder: true }),
				at(12, rectangle, 0, 0, { parent: 10 }),
				at(13, rectangle, 0, 0, { kind: 'texteditoverlay' }),
				at(14, rectangle, 0, 0),
			]);

			nodeassert.deepStrictEqual(MorphMatcher.candidates(leaving), [10, 14]);
			nodeassert.deepStrictEqual(
				MorphMatcher.candidates(leaving, new Set([14])),
				[10],
			);
		});

		it('pairs a forced name over a nearer object of the same kind', function () {
			const leaving = slide([at(10, rectangle, 0, 0, { name: '!!Logo' })]);
			const entering = slide([
				at(20, rectangle, 0, 0),
				at(21, ellipse, 9000, 9000, { name: '!!Logo' }),
			]);

			nodeassert.deepStrictEqual(MorphMatcher.match(leaving, entering), {
				pairs: [[10, 21]],
				leavingOnly: [],
				enteringOnly: [20],
			});
		});

		it('pairs equal names of the same kind only', function () {
			const leaving = slide([
				at(10, rectangle, 0, 0, { name: 'Shape' }),
				at(11, textBox, 0, 0, { name: 'Title' }),
			]);
			const entering = slide([
				at(20, ellipse, 0, 0, { name: 'Shape' }),
				at(21, textBox, 5000, 5000, { name: 'Title' }),
			]);

			nodeassert.deepStrictEqual(MorphMatcher.match(leaving, entering), {
				pairs: [[11, 21]],
				leavingOnly: [10],
				enteringOnly: [20],
			});
		});

		it('pairs the same text before the nearest text box', function () {
			const leaving = slide([
				at(10, textBox, 0, 0, text('One')),
				at(11, textBox, 2000, 0, text('Two')),
			]);
			const entering = slide([
				at(20, textBox, 0, 0, text('Two')),
				at(21, textBox, 2000, 0, text('Three')),
			]);

			nodeassert.deepStrictEqual(MorphMatcher.match(leaving, entering).pairs, [
				[11, 20],
				[10, 21],
			]);
		});

		it('pairs the same picture wherever it is', function () {
			const leaving = slide([
				at(10, picture, 0, 0, { graphicChecksum: 7 }),
				at(11, picture, 9000, 0, { graphicChecksum: 8 }),
			]);
			const entering = slide([
				at(20, picture, 9000, 0, { graphicChecksum: 7 }),
				at(21, picture, 0, 0, { graphicChecksum: 8 }),
			]);

			nodeassert.deepStrictEqual(MorphMatcher.match(leaving, entering).pairs, [
				[10, 20],
				[11, 21],
			]);
		});

		it('pairs the nearest objects of the same kind first', function () {
			const leaving = slide([
				at(10, rectangle, 0, 0),
				at(11, rectangle, 5000, 0),
			]);
			const entering = slide([
				at(20, rectangle, 4000, 0),
				at(21, rectangle, 500, 0),
			]);

			nodeassert.deepStrictEqual(MorphMatcher.match(leaving, entering).pairs, [
				[10, 21],
				[11, 20],
			]);
		});

		it('leaves a rectangle and an ellipse apart', function () {
			const leaving = slide([at(10, rectangle, 0, 0)]);
			const entering = slide([at(20, ellipse, 0, 0)]);

			nodeassert.deepStrictEqual(MorphMatcher.match(leaving, entering), {
				pairs: [],
				leavingOnly: [10],
				enteringOnly: [20],
			});
		});

		it('leaves out the hidden objects on either slide', function () {
			const leaving = slide([at(10, rectangle, 0, 0)]);
			const entering = slide([
				at(20, rectangle, 0, 0),
				at(21, rectangle, 0, 0),
			]);

			nodeassert.deepStrictEqual(
				MorphMatcher.match(leaving, entering, undefined, new Set([20])),
				{ pairs: [[10, 21]], leavingOnly: [], enteringOnly: [] },
			);
		});
	});

	describe('contentOf', function () {
		it('reads the text paragraph by paragraph', function () {
			nodeassert.strictEqual(
				MorphMatcher.contentOf(at(10, textBox, 0, 0, text('One', 'Two'))),
				'text One\nTwo',
			);
			nodeassert.strictEqual(
				MorphMatcher.contentOf(at(10, rectangle, 0, 0)),
				undefined,
			);
		});
	});

	describe('geometry', function () {
		it('moves and scales a frame along the way', function () {
			const from = new cool.Matrix2D(1000, 0, 0, 500, 0, 0);
			const to = new cool.Matrix2D(3000, 0, 0, 1500, 4000, 2000);

			assertFrame(MorphMatcher.interpolate(from, to, 0), from.toArray());
			assertFrame(
				MorphMatcher.interpolate(from, to, 0.5),
				[2000, 0, 0, 1000, 2000, 1000],
			);
			assertFrame(MorphMatcher.interpolate(from, to, 1), to.toArray());
		});

		it('turns the shorter way round', function () {
			const turned = (angle: number) =>
				new cool.Matrix2D(
					Math.cos(angle) * 1000,
					Math.sin(angle) * 1000,
					-Math.sin(angle) * 1000,
					Math.cos(angle) * 1000,
					0,
					0,
				);
			const from = turned((170 * Math.PI) / 180);
			const to = turned((-170 * Math.PI) / 180);

			// Halfway between 170 and -170 degrees is 180, not 0.
			assertFrame(
				MorphMatcher.interpolate(from, to, 0.5),
				turned(Math.PI).toArray(),
			);
		});

		it('takes a frame apart and puts it back together', function () {
			const frame = new cool.Matrix2D(800, 600, -300, 1400, 25, 75);

			assertFrame(
				MorphMatcher.frameOf(MorphMatcher.partsOf(frame)),
				frame.toArray(),
			);
		});

		it('maps a frame onto itself with the identity', function () {
			const frame = new cool.Matrix2D(1000, 200, -100, 500, 300, 400);
			const mapping = MorphMatcher.mapping(frame, frame);

			nodeassert.ok(mapping);
			assertFrame(mapping, [1, 0, 0, 1, 0, 0]);
		});

		it('maps a frame onto another one', function () {
			const source = new cool.Matrix2D(1000, 0, 0, 1000, 0, 0);
			const target = new cool.Matrix2D(2000, 0, 0, 500, 100, 200);
			const mapping = MorphMatcher.mapping(source, target);

			nodeassert.ok(mapping);
			assertFrame(source.then(mapping), target.toArray());
		});
	});

	describe('text pieces', function () {
		// A text box with one upright run of the text, at a font size of 100 twips with its
		// baseline start at 1000, 2000, and the given glyph advances.
		function run(textOfRun: string, advances: number[], extra?: any): any {
			return {
				objectKind: 16,
				primitives: [
					{
						type: 'textHierarchyBlock',
						children: [
							{
								type: 'textHierarchyParagraph',
								children: [
									Object.assign(
										{
											type: 'textSimplePortion',
											text: textOfRun,
											matrix: [100, 0, 0, 100, 1000, 2000],
											advances,
										},
										extra,
									),
								],
							},
						],
					},
				],
			};
		}

		// A piece from x, as wide as width, from above the baseline to below it.
		function box(x: number, width: number): number[] {
			return [width, 0, 0, 155, x, 1885];
		}

		it('splits the text at the blanks', function () {
			nodeassert.deepStrictEqual(MorphMatcher.rangesOf('ab  cd', false), [
				[0, 2],
				[4, 6],
			]);
		});

		it('takes each character but the blanks, a pair of surrogates as one', function () {
			nodeassert.deepStrictEqual(
				MorphMatcher.rangesOf('a b\ud83d\ude00', true),
				[
					[0, 1],
					[2, 3],
					[3, 5],
				],
			);
		});

		it('places each word along the run, its drawing reaching into the blanks', function () {
			const text = MorphMatcher.textPiecesOf(
				run('ab cd', [50, 50, 30, 50, 50]),
				false,
			);

			nodeassert.strictEqual(text.paragraphCount, 1);
			nodeassert.deepStrictEqual(
				text.pieces.map((piece: MorphPiece) => piece.text),
				['ab', 'cd'],
			);
			// The box of a word spans its glyphs, so the same word has the same box wherever it
			// stands. The part of the drawing it takes reaches halfway into the blanks.
			assertFrame(text.pieces[0].frame, box(1000, 100));
			assertFrame(text.pieces[0].clip, box(1000, 115));
			assertFrame(text.pieces[1].frame, box(1130, 100));
			assertFrame(text.pieces[1].clip, box(1115, 115));
		});

		it('places each character at its own advance', function () {
			const text = MorphMatcher.textPiecesOf(
				run('ab cd', [50, 50, 30, 50, 50]),
				true,
			);

			nodeassert.deepStrictEqual(
				text.pieces.map((piece: MorphPiece) => piece.text),
				['a', 'b', 'c', 'd'],
			);
			assertFrame(text.pieces[0].frame, box(1000, 50));
			assertFrame(text.pieces[1].frame, box(1050, 50));
			assertFrame(text.pieces[2].frame, box(1130, 50));
			assertFrame(text.pieces[3].frame, box(1180, 50));
			assertFrame(text.pieces[3].clip, box(1180, 50));
		});

		it('leaves a text whole when a run has no place for each character', function () {
			nodeassert.strictEqual(
				MorphMatcher.textPiecesOf(run('ab', [50, 50], { rtl: true }), false),
				undefined,
			);
			nodeassert.strictEqual(
				MorphMatcher.textPiecesOf(run('ab', [50]), false),
				undefined,
			);
		});

		it('pairs the pieces of one text in the order of the text', function () {
			const piece = (text: string, x: number) => ({
				text,
				paragraph: 0,
				frame: new cool.Matrix2D(10, 0, 0, 10, x, 0),
				clip: new cool.Matrix2D(10, 0, 0, 10, x, 0),
			});
			// The second ABC is nearer to the first ABC of the entering text, but the two
			// words keep their order instead of changing places.
			const leaving = [piece('ABC', 0), piece('ABC', 100), piece('EFG', 200)];
			const entering = [
				piece('ABC', 150),
				piece('ABC', 250),
				piece('EFG', 350),
			];

			nodeassert.deepStrictEqual(MorphMatcher.matchPieces(leaving, entering), {
				pairs: [
					[0, 0],
					[1, 1],
					[2, 2],
				],
				leavingOnly: [],
				enteringOnly: [],
			});
		});

		it('lets the pieces of different texts trade places', function () {
			const piece = (text: string, x: number) => ({
				text,
				paragraph: 0,
				frame: new cool.Matrix2D(10, 0, 0, 10, x, 0),
				clip: new cool.Matrix2D(10, 0, 0, 10, x, 0),
			});
			const leaving = [piece('A', 0), piece('B', 10), piece('C', 20)];
			const entering = [piece('C', 0), piece('B', 10), piece('A', 20)];

			nodeassert.deepStrictEqual(MorphMatcher.matchPieces(leaving, entering), {
				pairs: [
					[0, 2],
					[1, 1],
					[2, 0],
				],
				leavingOnly: [],
				enteringOnly: [],
			});
		});

		it('pairs the nearest of more pieces of one text, in their order', function () {
			const piece = (text: string, x: number) => ({
				text,
				paragraph: 0,
				frame: new cool.Matrix2D(10, 0, 0, 10, x, 0),
				clip: new cool.Matrix2D(10, 0, 0, 10, x, 0),
			});
			const leaving = [piece('A', 0), piece('A', 100), piece('A', 200)];
			const entering = [piece('A', 90), piece('A', 210)];

			nodeassert.deepStrictEqual(MorphMatcher.matchPieces(leaving, entering), {
				pairs: [
					[1, 0],
					[2, 1],
				],
				leavingOnly: [0],
				enteringOnly: [],
			});
		});

		it('pairs around a piece that was put in or taken out', function () {
			const piece = (text: string) => ({
				text,
				paragraph: 0,
				frame: new cool.Matrix2D(10, 0, 0, 10, 0, 0),
				clip: new cool.Matrix2D(10, 0, 0, 10, 0, 0),
			});
			const leaving = [piece('ABC'), piece('CDE'), piece('EFG')];
			const entering = [piece('XYZ'), piece('ABC'), piece('EFG')];

			nodeassert.deepStrictEqual(MorphMatcher.matchPieces(leaving, entering), {
				pairs: [
					[0, 1],
					[2, 2],
				],
				leavingOnly: [1],
				enteringOnly: [0],
			});
		});
	});
});
