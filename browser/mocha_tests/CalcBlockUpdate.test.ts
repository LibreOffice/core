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

describe('calcUpdatedBlock', function () {
	// A primitive that names the part it comes from.
	function marker(name: string): cool.Primitive {
		return { type: 'group', name } as cool.Primitive;
	}

	// The text of a row paints into column 0 and its own row.
	function rowPart(row: number, version: number, name: string) {
		return {
			row,
			version,
			content: {
				ink: {
					x: [0, 0] as cool.CalcPosition,
					y: [row, 0] as cool.CalcPosition,
					right: [0, 50] as cool.CalcPosition,
					bottom: [row, 100] as cool.CalcPosition,
				},
				layers: { text: [marker(name)] },
			},
		};
	}

	function update(
		from: number,
		version: number,
		rows: Array<cool.CalcBlockPart<cool.CalcCellRect> & { row: number }>,
		borders?: cool.CalcBlockPart<cool.CalcCellRect>,
	): cool.CalcBlockUpdate {
		return {
			kind: 'cells',
			blockRow: 0,
			blockColumn: 0,
			startRow: 0,
			startColumn: 0,
			endRow: 63,
			endColumn: 15,
			version,
			from,
			bounds: { x: [0, 0], y: [0, 0], right: [16, 0], bottom: [64, 0] },
			borders,
			rows,
		};
	}

	function names(primitives: cool.Primitive[] | undefined): string[] {
		return (primitives ?? []).map(
			(primitive) => (primitive as unknown as { name: string }).name,
		);
	}

	it('draws the borders first and then the rows in their order', function () {
		const block = cool.calcUpdatedBlock(
			undefined,
			update(0, 5, [rowPart(2, 5, 'two'), rowPart(0, 4, 'zero')], {
				version: 3,
				content: { layers: { borders: [marker('borders')] } },
			}),
			5,
		);

		nodeassert.deepStrictEqual(names(block?.content.layers.text), [
			'zero',
			'two',
		]);
		nodeassert.deepStrictEqual(names(block?.content.layers.borders), [
			'borders',
		]);
	});

	it('paints into the ranges of all its rows', function () {
		const block = cool.calcUpdatedBlock(
			undefined,
			update(0, 5, [rowPart(0, 5, 'zero'), rowPart(2, 5, 'two')]),
			5,
		);

		nodeassert.deepStrictEqual(block?.content.ink, {
			x: [0, 0],
			y: [0, 0],
			right: [0, 50],
			bottom: [2, 100],
		});
	});

	it('paints into the ranges of all its rows, wherever each reaches', function () {
		// The second row reaches left into column 0 and right past the end of the sheet.
		const part = (row: number, ink: cool.CalcCellRect) => ({
			row,
			version: 5,
			content: { ink, layers: { text: [marker(String(row))] } },
		});
		const block = cool.calcUpdatedBlock(
			undefined,
			update(0, 5, [
				part(8, { x: [1, 0], y: [8, 0], right: [2, 50], bottom: [8, 250] }),
				part(9, {
					x: [0, 900],
					y: [9, 0],
					right: [16383, 1000000000],
					bottom: [9, 250],
				}),
			]),
			5,
		);

		nodeassert.deepStrictEqual(block?.content.ink, {
			x: [0, 900],
			y: [8, 0],
			right: [16383, 1000000000],
			bottom: [9, 250],
		});
	});

	it('keeps the rows that an update leaves out', function () {
		const held = cool.calcUpdatedBlock(
			undefined,
			update(0, 5, [rowPart(0, 5, 'zero'), rowPart(1, 5, 'one')]),
			5,
		);
		const block = cool.calcUpdatedBlock(
			held,
			update(5, 7, [rowPart(1, 7, 'changed')]),
			7,
		);

		nodeassert.deepStrictEqual(names(block?.content.layers.text), [
			'zero',
			'changed',
		]);
		nodeassert.strictEqual(block?.version, 7);
		nodeassert.strictEqual(block?.current, 7);
	});

	it('drops a row that the update sends without layers', function () {
		const held = cool.calcUpdatedBlock(
			undefined,
			update(0, 5, [rowPart(0, 5, 'zero'), rowPart(1, 5, 'one')]),
			5,
		);
		const block = cool.calcUpdatedBlock(
			held,
			update(5, 7, [{ row: 0, version: 7, content: {} }]),
			7,
		);

		nodeassert.deepStrictEqual(names(block?.content.layers.text), ['one']);
		nodeassert.strictEqual(block?.rows.has(0), false);
	});

	it('leaves the block held as it was', function () {
		const held = cool.calcUpdatedBlock(
			undefined,
			update(0, 5, [rowPart(0, 5, 'zero')]),
			5,
		);
		cool.calcUpdatedBlock(held, update(5, 7, [rowPart(0, 7, 'changed')]), 7);

		nodeassert.deepStrictEqual(names(held?.content.layers.text), ['zero']);
	});

	it('takes no update that builds on a newer version than the one held', function () {
		const held = cool.calcUpdatedBlock(
			undefined,
			update(0, 5, [rowPart(0, 5, 'zero')]),
			5,
		);

		nodeassert.strictEqual(
			cool.calcUpdatedBlock(held, update(6, 7, [rowPart(1, 7, 'one')]), 7),
			undefined,
		);
	});

	it('takes no partial update without a block held', function () {
		nodeassert.strictEqual(
			cool.calcUpdatedBlock(undefined, update(5, 7, [rowPart(1, 7, 'one')]), 7),
			undefined,
		);
	});

	it('is current up to the sheet version of the answer', function () {
		const block = cool.calcUpdatedBlock(
			undefined,
			update(0, 5, [rowPart(0, 5, 'zero')]),
			9,
		);

		nodeassert.strictEqual(block?.version, 5);
		nodeassert.strictEqual(block?.current, 9);
	});
});

describe('calcApplyTextStyles', function () {
	const styles = new Map<number, cool.CalcTextStyle>([
		[
			3,
			{
				fontSize: 200,
				familyname: 'Liberation Sans',
				fontId: 'abc',
				weight: 5,
			},
		],
	]);

	function portion(fields: object): cool.Primitive {
		return {
			type: 'textSimplePortion',
			text: 'Hello',
			...fields,
		} as cool.Primitive;
	}

	it('gives a text the font attributes of its style', function () {
		const text = portion({ style: 3, x: 100, y: 240 });
		cool.calcApplyTextStyles([text], styles);

		const run = text as cool.TextSimplePortionPrimitive;
		nodeassert.strictEqual(run.familyname, 'Liberation Sans');
		nodeassert.strictEqual(run.fontId, 'abc');
		nodeassert.strictEqual(run.fontSize, 200);
		nodeassert.strictEqual(
			(text as unknown as { style?: number }).style,
			undefined,
		);
	});

	it('gives the strikeout character of a text its style too', function () {
		// A strikeout drawn with a character is a text among the lines of the run.
		const strikeout = portion({ text: 'X', style: 3, x: 100, y: 240 });
		const text = portion({
			type: 'textDecoratedPortion',
			style: 3,
			x: 100,
			y: 240,
			decorations: [strikeout],
		});
		cool.calcApplyTextStyles([text], styles);

		const run = strikeout as cool.TextSimplePortionPrimitive;
		nodeassert.strictEqual(run.fontSize, 200);
		nodeassert.deepStrictEqual(run.matrix, [200, 0, 0, 200, 100, 240]);
	});

	it('places a text at its origin at the size of its font', function () {
		const text = portion({ style: 3, x: 100, y: 240 });
		cool.calcApplyTextStyles([text], styles);

		nodeassert.deepStrictEqual(
			(text as cool.TextSimplePortionPrimitive).matrix,
			[200, 0, 0, 200, 100, 240],
		);
	});

	it('keeps the origin of a text in cell positions for the placer', function () {
		const text = portion({ style: 3, x: [2, 100], y: [8, 240] });
		cool.calcApplyTextStyles([text], styles);

		nodeassert.deepStrictEqual(
			(text as cool.TextSimplePortionPrimitive).matrix,
			[200, 0, 0, 200, [2, 100], [8, 240]],
		);
	});

	it('keeps the matrix of a text that has one', function () {
		const text = portion({ style: 3, matrix: [200, 0, 0, 100, 5, 6] });
		cool.calcApplyTextStyles([text], styles);

		nodeassert.deepStrictEqual(
			(text as cool.TextSimplePortionPrimitive).matrix,
			[200, 0, 0, 100, 5, 6],
		);
	});

	it('reaches the text inside a clip', function () {
		const text = portion({ style: 3, x: 1, y: 2 });
		cool.calcApplyTextStyles(
			[{ type: 'mask', children: [text] } as cool.Primitive],
			styles,
		);

		nodeassert.strictEqual(
			(text as cool.TextSimplePortionPrimitive).familyname,
			'Liberation Sans',
		);
	});

	it('leaves a text whose style is not known as it is', function () {
		const text = portion({ style: 9, x: 1, y: 2 });
		cool.calcApplyTextStyles([text], styles);

		nodeassert.strictEqual((text as unknown as { style?: number }).style, 9);
		nodeassert.strictEqual(
			(text as cool.TextSimplePortionPrimitive).matrix,
			undefined,
		);
	});
});

describe('calcBlockWithoutCellText', function () {
	function cellText(column: number, row: number): cool.Primitive {
		return { type: 'calcCellText', column, row } as unknown as cool.Primitive;
	}

	function block(): cool.CalcBlockEntry {
		const part = (row: number): cool.CalcBlockPart => ({
			version: 1,
			content: {
				layers: {
					background: [{ type: 'group' } as cool.Primitive],
					text: [cellText(2, row), cellText(3, row)],
				},
			},
		});
		return {
			kind: 'cells',
			blockRow: 0,
			blockColumn: 0,
			startRow: 0,
			startColumn: 0,
			endRow: 63,
			endColumn: 15,
			version: 1,
			current: 1,
			borders: undefined,
			rows: new Map([
				[4, part(4)],
				[5, part(5)],
			]),
			objects: new Set(),
			content: { bounds: { x: 0, y: 0, width: 1, height: 1 }, layers: {} },
		};
	}

	function columnsOfText(part: cool.CalcBlockPart | undefined): number[] {
		return (part?.content.layers?.text ?? []).map(
			(primitive) => (primitive as unknown as { column: number }).column,
		);
	}

	it('leaves out the stored text of an edited cell only', function () {
		const held = block();
		const shown = cool.calcBlockWithoutCellText(held, [{ column: 3, row: 4 }]);

		nodeassert.deepStrictEqual(columnsOfText(shown.rows.get(4)), [2]);
		nodeassert.deepStrictEqual(columnsOfText(shown.rows.get(5)), [2, 3]);
		// The rest of the row stays, and the other rows are shared.
		nodeassert.strictEqual(
			shown.rows.get(4)?.content.layers?.background,
			held.rows.get(4)?.content.layers?.background,
		);
		nodeassert.strictEqual(shown.rows.get(5), held.rows.get(5));
	});

	it('leaves the block it starts from as it is', function () {
		const held = block();
		cool.calcBlockWithoutCellText(held, [{ column: 3, row: 4 }]);

		nodeassert.deepStrictEqual(columnsOfText(held.rows.get(4)), [2, 3]);
	});
});

describe('calcTextStrips', function () {
	function part(name: string, y: number, layers?: cool.CalcBlockLayers) {
		return {
			version: 1,
			content: {
				ink: { x: 0, y, width: 100, height: 50 },
				layers: layers ?? {
					text: [{ type: 'group', name } as cool.Primitive],
				},
			},
		};
	}

	function block(
		rows: Array<[number, cool.CalcBlockPart]>,
	): cool.CalcBlockEntry {
		return {
			kind: 'cells',
			blockRow: 1,
			blockColumn: 0,
			startRow: 64,
			startColumn: 0,
			endRow: 127,
			endColumn: 15,
			version: 1,
			current: 1,
			borders: undefined,
			rows: new Map(rows),
			objects: new Set(),
			content: { bounds: { x: 0, y: 0, width: 100, height: 100 }, layers: {} },
		};
	}

	it('cuts the rows of a block into strips from its first row', function () {
		const strips = cool.calcTextStrips(
			block([
				[64, part('a', 0)],
				[79, part('b', 300)],
				[80, part('c', 320)],
			]),
			16,
		);

		nodeassert.deepStrictEqual(
			strips.map((strip) => [strip.strip, strip.parts.length]),
			[
				[0, 2],
				[1, 1],
			],
		);
	});

	it('paints a strip into the ranges of all its rows', function () {
		const strips = cool.calcTextStrips(
			block([
				[64, part('a', 0)],
				[70, part('b', 120)],
			]),
			16,
		);

		nodeassert.deepStrictEqual(strips[0].ink, {
			x: 0,
			y: 0,
			width: 100,
			height: 170,
		});
	});

	it('leaves out the rows that draw no text', function () {
		const strips = cool.calcTextStrips(
			block([
				[
					64,
					part('a', 0, { background: [{ type: 'group' } as cool.Primitive] }),
				],
				[96, part('b', 640)],
			]),
			16,
		);

		nodeassert.deepStrictEqual(
			strips.map((strip) => strip.strip),
			[2],
		);
	});
});
