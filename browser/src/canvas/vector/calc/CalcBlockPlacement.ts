/* -*- js-indent-level: 8; fill-column: 100 -*- */
/*
 * Copyright the Collabora Online contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

/*
 * Places block content at the zoom of the view. The engine names each position by the cell it
 * falls in and the print twips into that cell, which depend neither on the zoom nor on the size of
 * other columns and rows. The client rounds each column and row to whole pixels, in tile twips. A
 * position maps to its cell: the cell edges go to the rounded edges, and a position inside keeps
 * its share of the width. So fills, borders and clips end on the grid lines, and a zoom change or
 * a column or a row that changes size only places the content again. Text keeps its glyph
 * advances, and only its origin is mapped.
 */

namespace cool {
	/// A column or a row and the print twips into it, mapped to tile twips on the sheet.
	export type CalcCellMap = (cell: number, offset: number) => number;

	/// The start of a column or a row in print twips on the sheet.
	export type CalcCellStart = (cell: number) => number;

	/// The origin is the first cell of the block in tile twips. The pixel size is in tile twips.
	export interface CalcBlockFrame {
		mapColumn: CalcCellMap;
		mapRow: CalcCellMap;
		printColumn: CalcCellStart;
		printRow: CalcCellStart;
		tileOrigin: Point;
		pixel: number;
	}

	/// A rectangle as a primitive writes it.
	type Range = CalcVectorRect;

	/// Maps primitive coordinates to block coordinates: block x = a * x + e, block y = d * y + f.
	interface Axes {
		a: number;
		d: number;
		e: number;
		f: number;
	}

	const identity: Axes = { a: 1, d: 1, e: 0, f: 0 };

	/// The types whose matrix places glyphs, of which only the origin is mapped.
	const textTypes = new Set(['textSimplePortion', 'textDecoratedPortion']);

	/// The types whose matrix maps the unit square onto an image.
	const imageTypes = new Set([
		'bitmap',
		'bitmapAlpha',
		'graphic',
		'fillGraphic',
	]);

	/// The types that stroke a path with a line of a width.
	const strokeTypes = new Set(['polygonStroke', 'polyPolygonStroke']);

	/// The names of the fields of a text that hold its lines, apart from its children.
	const decorationFields = ['decorations', 'decorationsInTextColor'];

	/// The names of the fields that hold a rectangle.
	const rangeFields = [
		'bounds',
		'outputRange',
		'definitionRange',
		'referenceRange',
		'graphicRange',
		'clip',
	];

	/// The commands and the coordinates of a path: "column:offset,row:offset" points.
	const cellPathTokens =
		/[MLCZ]|[-+]?(?:\d+\.?\d*|\.\d+)(?:[eE][-+]?\d+)?:[-+]?(?:\d+\.?\d*|\.\d+)(?:[eE][-+]?\d+)?/g;

	/// The names of the fields that the placer moves. A field of the engine's primitives that
	/// holds a position and is not one of these stays where it was.
	export const calcPlacedFields: ReadonlySet<string> = new Set([
		'matrix',
		'path',
		'startX',
		'startY',
		'endX',
		'endY',
		'areas',
		'children',
		'highlight',
		...rangeFields,
		...decorationFields,
	]);

	function rounded(value: number): number {
		return Math.round(value * 100) / 100;
	}

	class Placer {
		constructor(private readonly frame: CalcBlockFrame) {}

		/// A position is of the content, before the axes, and is given back in the space of the
		/// axes.
		x(axes: Axes, position: CalcPosition): number {
			const frame = this.frame;
			const placed =
				frame.mapColumn(position[0], position[1]) - frame.tileOrigin.x;
			return (placed - axes.e) / axes.a;
		}

		y(axes: Axes, position: CalcPosition): number {
			const frame = this.frame;
			const placed =
				frame.mapRow(position[0], position[1]) - frame.tileOrigin.y;
			return (placed - axes.f) / axes.d;
		}

		range(axes: Axes, range: CalcCellRect): Range {
			const x = this.x(axes, range.x);
			const y = this.y(axes, range.y);
			return {
				x,
				y,
				width: this.x(axes, range.right) - x,
				height: this.y(axes, range.bottom) - y,
			};
		}

		/// The painted area of a part. Each column and row is rounded down to whole pixels, so it
		/// is at most as large as in print twips, while text keeps the advances of its glyphs in
		/// print twips. A text that reaches over several columns or lines then ends past its
		/// placed ink, so the ink keeps at least its size in print twips.
		ink(range: CalcCellRect): Range {
			const placed = this.range(identity, range);
			return {
				x: placed.x,
				y: placed.y,
				width: Math.max(
					placed.width,
					this.printX(range.right) - this.printX(range.x),
				),
				height: Math.max(
					placed.height,
					this.printY(range.bottom) - this.printY(range.y),
				),
			};
		}

		/// A position of the block content in print twips on the sheet.
		printX(position: CalcPosition): number {
			return this.frame.printColumn(position[0]) + position[1];
		}

		printY(position: CalcPosition): number {
			return this.frame.printRow(position[0]) + position[1];
		}

		/// The path data, which is absolute and has only M, L, C and Z, with every point and
		/// control point mapped and written as SVG path data.
		path(axes: Axes, data: string): string {
			let out = '';
			let vertical = false;
			let pointsInCommand = 0;
			for (const token of data.match(cellPathTokens) ?? []) {
				if (/^[MLCZ]$/.test(token)) {
					out += token;
					pointsInCommand = 0;
					continue;
				}
				const colon = token.indexOf(':');
				const position: CalcPosition = [
					parseFloat(token.substring(0, colon)),
					parseFloat(token.substring(colon + 1)),
				];
				if (vertical) {
					out += ' ' + String(rounded(this.y(axes, position)));
					pointsInCommand++;
				} else {
					if (pointsInCommand) out += ' ';
					out += String(rounded(this.x(axes, position)));
				}
				vertical = !vertical;
			}
			return out;
		}

		primitives(
			primitives: Primitive[] | undefined,
			axes: Axes,
			borders: boolean,
		): Primitive[] | undefined {
			return primitives?.map((primitive) =>
				this.primitive(primitive, axes, borders),
			);
		}

		primitive(primitive: Primitive, axes: Axes, borders: boolean): Primitive {
			const type = primitive.type ?? '';
			const source = primitive as unknown as Record<string, unknown>;
			const placed: Record<string, unknown> = { ...source };
			const matrix = source.matrix as number[] | undefined;
			const hasMatrix = Array.isArray(matrix) && matrix.length === 6;

			if (type === TransformPrimitive.type && hasMatrix) {
				const [a, b, c, d, e, f] = matrix;
				if (b === 0 && c === 0 && a !== 0 && d !== 0) {
					// The transform keeps its matrix, and its children are positions of the
					// content mapped through it.
					const inner: Axes = {
						a: axes.a * a,
						d: axes.d * d,
						e: axes.a * e + axes.e,
						f: axes.d * f + axes.f,
					};
					placed.children = this.primitives(primitive.children, inner, borders);
				} else {
					// A rotated or sheared content moves with its origin and keeps its shape.
					const origin = matrix as unknown as CalcPosition[];
					placed.matrix = [
						a,
						b,
						c,
						d,
						this.x(axes, origin[4]),
						this.y(axes, origin[5]),
					];
				}
				return placed as unknown as Primitive;
			}

			if (hasMatrix) {
				// The origin is a position. An upright image has its far corner as a position
				// too, in the places of the scale, so it stretches with its cells.
				const positions = matrix as unknown as CalcPosition[];
				const [a, b, c, d] = matrix;
				const x = this.x(axes, positions[4]);
				const y = this.y(axes, positions[5]);
				if (imageTypes.has(type) && Array.isArray(positions[0]))
					placed.matrix = [
						this.x(axes, positions[0]) - x,
						0,
						0,
						this.y(axes, positions[3]) - y,
						x,
						y,
					];
				else if (textTypes.has(type) || imageTypes.has(type))
					placed.matrix = [a, b, c, d, x, y];
			}

			this.shapes(source, placed, axes);

			// A border line is drawn whole pixels wide, at least one, as the tiles draw it.
			const line = source.line as { width?: number } | undefined;
			if (borders && strokeTypes.has(type) && line && line.width) {
				const pixel = this.frame.pixel;
				const pixels = Math.max(
					1,
					Math.round((line.width * Math.abs(axes.a)) / pixel),
				);
				placed.line = { ...line, width: (pixels * pixel) / Math.abs(axes.a) };
			}

			if (primitive.children)
				placed.children = this.primitives(primitive.children, axes, borders);

			// The lines and the highlight of a text are in the space of its glyphs, apart from
			// its children, and move with the text.
			for (const field of decorationFields) {
				const decorations = source[field];
				if (Array.isArray(decorations))
					placed[field] = this.primitives(decorations, axes, false);
			}
			const highlight = source.highlight;
			if (highlight && typeof highlight === 'object') {
				const placedHighlight: Record<string, unknown> = { ...highlight };
				this.shapes(
					highlight as Record<string, unknown>,
					placedHighlight,
					axes,
				);
				placed.highlight = placedHighlight;
			}
			return placed as unknown as Primitive;
		}

		/// Move the paths, rectangles, line ends and areas of the source into placed.
		shapes(
			source: Record<string, unknown>,
			placed: Record<string, unknown>,
			axes: Axes,
		): void {
			if (typeof source.path === 'string')
				placed.path = this.path(axes, source.path);
			if (typeof source.clip === 'string')
				placed.clip = this.path(axes, source.clip);
			for (const field of rangeFields) {
				const value = source[field];
				if (Array.isArray(value) && value.length === 4) {
					placed[field] = [
						this.x(axes, value[0]),
						this.y(axes, value[1]),
						this.x(axes, value[2]),
						this.y(axes, value[3]),
					];
				} else if (value && typeof value === 'object' && 'right' in value)
					placed[field] = this.range(axes, value as CalcCellRect);
			}
			if (source.startX !== undefined) {
				placed.startX = this.x(axes, source.startX as CalcPosition);
				placed.startY = this.y(axes, source.startY as CalcPosition);
				placed.endX = this.x(axes, source.endX as CalcPosition);
				placed.endY = this.y(axes, source.endY as CalcPosition);
			}
			if (Array.isArray(source.areas))
				placed.areas = (source.areas as Array<{ rect?: CalcCellRect }>).map(
					(area) =>
						area.rect ? { ...area, rect: this.range(axes, area.rect) } : area,
				);
		}
	}

	function placeLayers(
		placer: Placer,
		layers: CalcBlockLayers,
	): CalcBlockLayers {
		const placed: CalcBlockLayers = {};
		for (const name of Object.keys(layers) as Array<keyof CalcBlockLayers>)
			placed[name] = placer.primitives(
				layers[name],
				identity,
				name === 'borders',
			);
		return placed;
	}

	/// The layers of a block placed by the frame.
	export function calcPlacedLayers(
		layers: CalcBlockLayers,
		frame: CalcBlockFrame,
	): CalcBlockLayers {
		return placeLayers(new Placer(frame), layers);
	}

	/// Maps a cell and the print twips into it to tile twips, with the start of the cell on its
	/// whole pixel, where the grid lines are: fills of cells next to each other then meet on one
	/// pixel and leave no seam, which they do where an edge falls between two pixels. The share
	/// of the cell is kept, and a hidden cell draws nothing beyond its edge. pixel is the twips in
	/// a pixel. A cell is looked up once.
	export function calcCellMap(
		dimension: SheetDimension,
		pixel: number,
	): CalcCellMap {
		const known = new Map<number, { start: number; scale: number }>();
		return (cell: number, offset: number): number => {
			let mapping = known.get(cell);
			if (mapping === undefined) {
				const pixels = dimension.getElementDataAny(cell, 'corepixels');
				const print = dimension.getElementDataAny(cell, 'printtwips');
				mapping = {
					start: pixels ? pixels.startpos * pixel : NaN,
					scale:
						pixels && print && print.size > 0
							? (pixels.size * pixel) / print.size
							: 0,
				};
				known.set(cell, mapping);
			}
			return mapping.start + Math.round(offset * mapping.scale);
		};
	}

	/// The start of each column or row in print twips, looked up once.
	export function calcCellStart(dimension: SheetDimension): CalcCellStart {
		const known = new Map<number, number>();
		return (cell: number): number => {
			let start = known.get(cell);
			if (start === undefined) {
				const data = dimension.getElementDataAny(cell, 'printtwips');
				start = data ? data.startpos : NaN;
				known.set(cell, start);
			}
			return start;
		};
	}

	/// The block with its parts placed at the sheet zoom and its joined layers left empty.
	export function calcPlacedBlock(
		block: CalcBlockEntry<CalcCellRect>,
		geometry: SheetGeometry,
		pixel: number,
	): CalcBlockEntry | undefined {
		const columns = geometry.getColumnsGeometry();
		const rows = geometry.getRowsGeometry();
		const pixelColumn = columns.getElementDataAny(
			block.startColumn,
			'corepixels',
		);
		const pixelRow = rows.getElementDataAny(block.startRow, 'corepixels');
		if (!pixelColumn || !pixelRow) return undefined;
		// The origin lies on the pixel of the first cell, as every cell start does.
		const tileOrigin = new Point(
			pixelColumn.startpos * pixel,
			pixelRow.startpos * pixel,
		);

		const frame: CalcBlockFrame = {
			mapColumn: calcCellMap(columns, pixel),
			mapRow: calcCellMap(rows, pixel),
			printColumn: calcCellStart(columns),
			printRow: calcCellStart(rows),
			tileOrigin,
			pixel,
		};
		const placer = new Placer(frame);
		const placePart = (part: CalcBlockPart<CalcCellRect>): CalcBlockPart => ({
			version: part.version,
			content: {
				ink: part.content.ink ? placer.ink(part.content.ink) : undefined,
				layers: part.content.layers
					? placeLayers(placer, part.content.layers)
					: undefined,
			},
		});
		const rowsInOrder = Array.from(block.rows.keys()).sort((a, b) => a - b);
		const content = block.content;
		return {
			...block,
			borders: block.borders ? placePart(block.borders) : undefined,
			rows: new Map(
				rowsInOrder.map((row) => [
					row,
					placePart(block.rows.get(row) as CalcBlockPart<CalcCellRect>),
				]),
			),
			content: {
				bounds: placer.range(identity, content.bounds),
				ink: content.ink ? placer.ink(content.ink) : undefined,
				layers: {},
			},
		};
	}
}
