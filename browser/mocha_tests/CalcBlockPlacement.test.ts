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

describe('calcPlacedLayers', function () {
	// Columns are 1000 print twips wide and drawn at half, rows are 250 high and drawn as they
	// are. The block starts at column 1, row 8. A pixel is 10 tile twips.
	const frame: cool.CalcBlockFrame = {
		mapColumn: (column: number, offset: number) => (column * 1000 + offset) / 2,
		mapRow: (row: number, offset: number) => row * 250 + offset,
		printColumn: (column: number) => column * 1000,
		printRow: (row: number) => row * 250,
		tileOrigin: new cool.Point(500, 2000),
		pixel: 10,
	};

	function placed(layers: cool.CalcBlockLayers, at = frame): any {
		return cool.calcPlacedLayers(layers, at);
	}

	function placedText(primitive: unknown): any {
		return placed({ text: [primitive as cool.Primitive] }).text[0];
	}

	it('puts the corners of a fill on the placed cell edges', function () {
		const fill = placed({
			background: [
				{
					type: 'polyPolygonColor',
					color: '#ff0000',
					path: 'M1:0,8:0L1:200,8:0L1:200,8:100L1:0,8:100Z',
					bounds: [
						[1, 0],
						[8, 0],
						[1, 200],
						[8, 100],
					],
				} as unknown as cool.Primitive,
			],
		}).background[0];

		nodeassert.strictEqual(fill.path, 'M0 0L100 0L100 100L0 100Z');
		nodeassert.deepStrictEqual(fill.bounds, [0, 0, 100, 100]);
	});

	it('places a curve and the next polygon of a path', function () {
		const line = placedText({
			type: 'polygonHairline',
			path: 'M1:0,8:0C1:100,8:0 1:200,8:50 1:200,8:100ZM2:0,8:0L2:200,9:0',
		});

		nodeassert.strictEqual(
			line.path,
			'M0 0C50 0 100 50 100 100ZM500 0L600 250',
		);
	});

	it('moves the origin of a text and keeps the size of its glyphs', function () {
		const text = placedText({
			type: 'textSimplePortion',
			text: '12',
			matrix: [200, 0, 0, 200, [1, 400], [8, 240]],
			advances: [100, 100],
		});

		nodeassert.deepStrictEqual(text.matrix, [200, 0, 0, 200, 200, 240]);
		nodeassert.deepStrictEqual(text.advances, [100, 100]);
	});

	it('moves the lines and the highlight of a text with it', function () {
		// The underline, the strikeout and the highlight are drawn in the same space as the
		// glyphs, so a text placed elsewhere leaves them behind unless they move with it.
		const text = placedText({
			type: 'textDecoratedPortion',
			text: '12',
			matrix: [200, 0, 0, 200, [1, 400], [8, 240]],
			underline: 1,
			highlight: { path: 'M1:400,8:40L1:800,8:40L1:800,9:10L1:400,9:10Z' },
			decorations: [
				{
					type: 'polygonStroke',
					path: 'M1:400,9:10L1:800,9:10',
					line: { width: 10 },
				},
			],
			decorationsInTextColor: [
				{
					type: 'polygonStroke',
					path: 'M1:400,8:180L1:800,8:180',
					line: { width: 10 },
				},
			],
		});

		nodeassert.deepStrictEqual(text.matrix, [200, 0, 0, 200, 200, 240]);
		nodeassert.strictEqual(text.decorations[0].path, 'M200 260L400 260');
		nodeassert.strictEqual(
			text.decorationsInTextColor[0].path,
			'M200 180L400 180',
		);
		nodeassert.strictEqual(
			text.highlight.path,
			'M200 40L400 40L400 260L200 260Z',
		);
	});

	it('stretches an upright image with its cells', function () {
		const image = placedText({
			type: 'bitmap',
			matrix: [[2, 600], 0, 0, [8, 100], [1, 400], [8, 0]],
		});

		nodeassert.deepStrictEqual(image.matrix, [600, 0, 0, 100, 200, 0]);
	});

	it('places the content of a transform, which holds positions of the content', function () {
		// The text is at 600 in the block, placed at 300: -100 inside the transform, which moves
		// its children by 400.
		const transform = placedText({
			type: 'transform',
			matrix: [1, 0, 0, 1, 400, 0],
			children: [
				{
					type: 'textSimplePortion',
					text: 'A',
					matrix: [200, 0, 0, 200, [1, 600], [8, 100]],
				},
			],
		});

		nodeassert.deepStrictEqual(transform.matrix, [1, 0, 0, 1, 400, 0]);
		nodeassert.deepStrictEqual(
			transform.children[0].matrix,
			[200, 0, 0, 200, -100, 100],
		);
	});

	it('moves rotated content by its origin and keeps its shape', function () {
		const child = { type: 'polygonHairline', path: 'M0 0L100 0' };
		const transform = placedText({
			type: 'transform',
			matrix: [0, -1, 1, 0, [1, 400], [9, 50]],
			children: [child],
		});

		nodeassert.deepStrictEqual(transform.matrix, [0, -1, 1, 0, 200, 300]);
		nodeassert.strictEqual(transform.children[0], child);
	});

	it('draws a border line whole pixels wide, at least one', function () {
		const stroke = (width: number): cool.Primitive =>
			({
				type: 'polygonStroke',
				path: 'M1:0,8:0L1:0,8:100',
				line: { color: '#000000', width },
			}) as cool.Primitive;
		const layers = placed({ borders: [stroke(4), stroke(26)] });

		nodeassert.strictEqual(layers.borders[0].line.width, 10);
		nodeassert.strictEqual(layers.borders[1].line.width, 30);
	});

	it('keeps the width of a line that is not a border', function () {
		const layers = placed({
			background: [
				{
					type: 'polygonStroke',
					path: 'M1:0,8:0L1:0,8:100',
					line: { color: '#000000', width: 4 },
				} as cool.Primitive,
			],
		});

		nodeassert.strictEqual(layers.background[0].line.width, 4);
	});

	it('places the clip of a text and the areas without grid lines', function () {
		const layers = placed({
			text: [
				{
					type: 'mask',
					clip: 'M1:0,8:0L1:400,8:0L1:400,8:100L1:0,8:100Z',
					children: [],
				} as cool.Primitive,
			],
			gridCover: [
				{
					type: 'calcGridCover',
					areas: [
						{
							startColumn: 1,
							rect: { x: [1, 0], y: [8, 0], right: [1, 400], bottom: [8, 100] },
						},
					],
				} as unknown as cool.Primitive,
			],
		});

		nodeassert.strictEqual(layers.text[0].clip, 'M0 0L200 0L200 100L0 100Z');
		nodeassert.deepStrictEqual(layers.gridCover[0].areas[0].rect, {
			x: 0,
			y: 0,
			width: 200,
			height: 100,
		});
	});

	it('places the clip of a cell text, with edges outside the sheet in its first and last row', function () {
		// A clip that does not clip vertically reaches 1e9 twips above and below the block. The
		// engine names those edges in row 0, before its start, and in the last row, past its end.
		const text = placedText({
			type: 'calcCellText',
			clip: {
				x: [1, 0],
				y: [0, -999998000],
				right: [1, 400],
				bottom: [1048575, 737858250],
			},
		});

		nodeassert.deepStrictEqual(text.clip, {
			x: 0,
			y: -1000000000,
			width: 200,
			height: 2000000000,
		});
	});

	it('places the ends of a gradient', function () {
		const gradient = placedText({
			type: 'linearGradient',
			startX: [1, 0],
			startY: [8, 0],
			endX: [1, 400],
			endY: [8, 100],
		});

		nodeassert.deepStrictEqual(
			[gradient.startX, gradient.startY, gradient.endX, gradient.endY],
			[0, 0, 200, 100],
		);
	});

	it('follows a column that got wider', function () {
		// Column 1 is 600 tile twips wider, so column 2 starts 600 further right.
		const wider: cool.CalcBlockFrame = {
			...frame,
			mapColumn: (column: number, offset: number) =>
				(column * 1000 + offset) / 2 + (column > 1 ? 600 : 0),
		};
		const text = {
			type: 'textSimplePortion',
			matrix: [200, 0, 0, 200, [2, 100], [8, 240]],
		} as unknown as cool.Primitive;

		nodeassert.strictEqual(placed({ text: [text] }).text[0].matrix[4], 550);
		nodeassert.strictEqual(
			placed({ text: [text] }, wider).text[0].matrix[4],
			1150,
		);
	});

	it('leaves the layers it places from as they are', function () {
		const text = {
			type: 'textSimplePortion',
			text: '1',
			matrix: [200, 0, 0, 200, [1, 400], [8, 240]],
		};
		placed({ text: [text as unknown as cool.Primitive] });

		nodeassert.deepStrictEqual(text.matrix, [
			200,
			0,
			0,
			200,
			[1, 400],
			[8, 240],
		]);
	});
});

describe('calcPlacedBlock', function () {
	// Columns are 1000 print twips wide and 66 pixels, rows 250 and 16, at 15 twips a pixel:
	// each loses what the rounding to whole pixels takes.
	function dimension(print: number, pixels: number): cool.SheetDimension {
		return {
			getElementDataAny: (index: number, unit: string) =>
				unit === 'printtwips'
					? { startpos: index * print, size: print }
					: { startpos: index * pixels, size: pixels },
		} as unknown as cool.SheetDimension;
	}
	const geometry = {
		getColumnsGeometry: () => dimension(1000, 66),
		getRowsGeometry: () => dimension(250, 16),
	} as unknown as cool.SheetGeometry;

	function placedInk(
		ink: cool.CalcCellRect,
		shifts: cool.CalcShift[] = [],
	): cool.CalcVectorRect | undefined {
		const part = {
			version: 1,
			content: {
				ink,
				layers: { text: [{ type: 'group' } as cool.Primitive] },
			},
		};
		const block: cool.CalcBlockEntry<cool.CalcCellRect> = {
			kind: 'cells',
			blockRow: 0,
			blockColumn: 0,
			startRow: 8,
			startColumn: 1,
			endRow: 63,
			endColumn: 15,
			version: 1,
			current: 1,
			borders: undefined,
			rows: new Map([[8, part]]),
			objects: new Set(),
			content: {
				bounds: { x: [1, 0], y: [8, 0], right: [2, 0], bottom: [9, 0] },
				layers: {},
			},
		};
		return cool.calcPlacedBlock(block, geometry, 15, shifts)?.rows.get(8)
			?.content.ink;
	}

	it('keeps the ink of text over several columns as wide as in print twips', function () {
		// The text keeps its print advances, so it ends 4500 twips after its start. Placed by
		// its cells, the far edge would be at 4455 and cut the last letter off.
		nodeassert.deepStrictEqual(
			placedInk({ x: [1, 0], y: [8, 0], right: [5, 500], bottom: [8, 250] }),
			{ x: 0, y: 0, width: 4500, height: 250 },
		);
	});

	it('places the content of a block older than inserted rows where its cells moved', function () {
		// Two rows inserted at row 3 moved row 8 to row 10, two rows of 240 tile twips down.
		nodeassert.deepStrictEqual(
			placedInk({ x: [1, 0], y: [8, 0], right: [5, 500], bottom: [8, 250] }, [
				{ axis: 'rows', at: 3, by: 2, across: [0, 15] },
			]),
			{ x: 0, y: 480, width: 4500, height: 250 },
		);
	});

	it('leaves out the rows of a block that were removed', function () {
		// Rows 8 and 9 were removed: row 10 moved to row 8.
		nodeassert.strictEqual(
			placedInk({ x: [1, 0], y: [8, 0], right: [5, 500], bottom: [8, 250] }, [
				{ axis: 'rows', at: 10, by: -2, across: [0, 15] },
			]),
			undefined,
		);
	});
});

describe('calcIndexShift', function () {
	it('moves the rows after an insert and collapses the removed ones', function () {
		const shift = cool.calcIndexShift(
			[
				{ axis: 'rows', at: 10, by: 3, across: [0, 15] },
				{ axis: 'rows', at: 5, by: -2, across: [0, 15] },
			],
			'rows',
		);
		// Before both shifts a row stays. Row 12 moved to 15 with the insert and to 13 with the
		// removal of rows 3 and 4; row 4 was removed and lies on row 3.
		nodeassert.deepStrictEqual(shift(2), { index: 2, removed: false });
		nodeassert.deepStrictEqual(shift(12), { index: 13, removed: false });
		nodeassert.deepStrictEqual(shift(4), { index: 3, removed: true });
		// A shift of the columns leaves the rows alone.
		nodeassert.deepStrictEqual(
			cool.calcIndexShift(
				[{ axis: 'columns', at: 0, by: 1, across: [0, 9] }],
				'rows',
			)(7),
			{ index: 7, removed: false },
		);
	});
});

describe('calcCellMap', function () {
	// Column 3 starts at 3000 print twips and at pixel 240, and is 1000 twips and 80 pixels
	// wide. A pixel is 10 twips.
	const dimension = {
		getElementDataAny: (index: number, unit: string) =>
			unit === 'printtwips'
				? { startpos: index * 1000, size: 1000 }
				: { startpos: index * 80, size: 80 },
	} as unknown as cool.SheetDimension;

	it('keeps the share of the cell', function () {
		nodeassert.strictEqual(cool.calcCellMap(dimension, 10)(3, 500), 2800);
	});

	it('puts the start of a cell on its tile edge', function () {
		nodeassert.strictEqual(cool.calcCellMap(dimension, 10)(3, 0), 2400);
	});

	it('puts the edge of a cell on a whole pixel where a pixel is not whole twips', function () {
		// Rows 42 pixels high at 3.75 twips a pixel: row 3 starts at pixel 126. Tile twips
		// rounded down to whole twips would put it at 472, a seventh of a pixel short.
		const rows = {
			getElementDataAny: (index: number, unit: string) =>
				unit === 'printtwips'
					? { startpos: index * 256, size: 256 }
					: { startpos: index * 42, size: 42 },
		} as unknown as cool.SheetDimension;
		nodeassert.strictEqual(cool.calcCellMap(rows, 3.75)(3, 0) / 3.75, 126);
	});
});

// The placer moves the fields of the engine's primitives that it knows by name. A field that
// holds a position and is new to it stays where the engine put it, apart from the rest of the
// cell: the lines of a text did, until the placer learned them. This test reads the fields
// that the engine writes and fails on any that the placer neither moves nor lists here as
// holding no position.
describe('calcPlacedFields', function () {
	const fs = require('fs');
	const path = require('path');

	// The fields that hold no position of their own, by what they are.
	const notPositions: { [kind: string]: string[] } = {
		'colors and transparency': [
			'alpha',
			'backdrop',
			'backdropTransparency',
			'backgroundColor',
			'blue',
			'color',
			'colorA',
			'colorB',
			'colorModifiers',
			'colorStops',
			'contrast',
			'fillcolor',
			'fontcolor',
			'green',
			'luminance',
			'modifier',
			'opacity',
			'overlineColor',
			'red',
			'transparence',
			'transparency',
			'underlineColor',
			'value',
		],
		'text attributes': [
			'align',
			'emphasisMark',
			'emphasisMarkAbove',
			'emphasisMarkBelow',
			'familyname',
			'fieldType',
			'fontHeight',
			'fontId',
			'fontSize',
			'italic',
			'letterSpacing',
			'monospaced',
			'outline',
			'overline',
			'paragraph',
			'relief',
			'rtl',
			'shadow',
			'strikeout',
			'style',
			'stylename',
			'syntheticBold',
			'syntheticItalic',
			'text',
			'textLength',
			'textPosition',
			'underline',
			'underlineAbove',
			'weight',
			'wordLineMode',
		],
		'sizes and lengths, which move with their origin': [
			'advances',
			'blur',
			'distance',
			'dotDashArray',
			'fullDotDashLen',
			'height',
			'lineAscent',
			'lineHeight',
			'lineIndent',
			'lineWidth',
			'radius',
			'textAreaHeight',
			'textAreaWidth',
			'width',
		],
		'line, stroke, gradient and fill settings': [
			'angle',
			'border',
			'fillBackground',
			'focalStep',
			'hatch',
			'line',
			'linecap',
			'linejoin',
			'mirror',
			'offset',
			'rotation',
			'spreadMethod',
			'steps',
			'stroke',
			'tiling',
		],
		'relative to their own image or area': [
			'fillGraphic',
			'imageRect',
			'offsetA',
			'offsetB',
			'offsetX',
			'offsetY',
		],
		'object data and the outer list': [
			'checksum',
			'data',
			'desc',
			'error',
			'name',
			'primitives',
			'title',
			'type',
			'vector',
		],
		'the values of a form control model': [
			'background',
			'bold',
			'columns',
			'date',
			'defaultButton',
			'disabled',
			'dropdown',
			'font',
			'items',
			'kind',
			'label',
			'max',
			'min',
			'multiLine',
			'multiSelection',
			'pageStep',
			'selected',
			'spin',
			'state',
			'step',
			'textColor',
			'vertical',
			'visible',
		],
		// The transparency of a reflection runs between fractions of its height.
		'the fractions of a reflection': [
			'endPosition',
			'endTransparency',
			'startPosition',
			'startTransparency',
		],
		// The compact origin of a text becomes its matrix before the placer sees it.
		'the compact origin of a text': ['x', 'y'],
		// Positions that cell content does not produce: SVG gradients, point arrays and the
		// range of a reflection. The placer has to learn them before the cells use them.
		'not in cell content': [
			'centerX',
			'centerY',
			'focalX',
			'focalY',
			'points',
			'range',
			'scaleA',
			'scaleB',
			'translateAX',
			'translateAY',
			'translateBX',
			'translateBY',
			'unitToObject',
		],
	};

	it('knows every field of the engine primitives that holds a position', function () {
		const file = path.join(
			__dirname,
			'..',
			'..',
			'..',
			'engine',
			'drawinglayer',
			'source',
			'processor2d',
			'Primitive2dJsonProcessor.cxx',
		);
		// A tree without the engine has nothing to compare with.
		if (!fs.existsSync(file)) this.skip();
		const source: string = fs.readFileSync(file, 'utf8');

		const written = new Set<string>();
		const pattern =
			/\b(?:put|putPosition|putRange|putPath|putMatrix|startArray|startNode|writeRange|writeMatrixScaled|writeMatrixTranslationScaled)\("([A-Za-z]+)"/g;
		let match: RegExpExecArray | null;
		while ((match = pattern.exec(source)) !== null) written.add(match[1]);
		// The pattern still finds the fields where the engine writes them.
		nodeassert.ok(written.has('matrix') && written.has('decorations'));

		const known = new Set<string>();
		cool.calcPlacedFields.forEach((field) => known.add(field));
		for (const kind of Object.keys(notPositions))
			for (const field of notPositions[kind]) known.add(field);

		const unknown: string[] = [];
		written.forEach((field) => {
			if (!known.has(field)) unknown.push(field);
		});
		nodeassert.deepStrictEqual(
			unknown.sort(),
			[],
			'Fields of Primitive2dJsonProcessor.cxx that the Calc placer does not know. ' +
				'One that holds a position needs mapping in CalcBlockPlacement.ts and a place ' +
				'in calcPlacedFields; one that does not goes into notPositions here.',
		);
	});
});
