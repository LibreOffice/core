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
});
