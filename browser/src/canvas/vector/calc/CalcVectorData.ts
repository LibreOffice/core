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
 * The JSON for vector rendering of a sheet, which is cut into blocks of cells. Each position of a
 * block primitive is a cell and the twips into it, so a block stays right when the size of a
 * column or a row changes. A block arrives in parts: the borders of the whole block, and the other
 * layers of each row. Drawing objects are in twips from the top-left corner of their anchor cell.
 */

namespace cool {
	/// A rectangle in twips.
	export interface CalcVectorRect {
		x: number;
		y: number;
		width: number;
		height: number;
	}

	/// A position along one axis of the sheet: [column or row, twips into it]. A position before
	/// the sheet has a negative offset into its first cell, one after it an offset past its last.
	export type CalcPosition = [number, number];

	/// A rectangle with each edge a position, as the engine sends it.
	export interface CalcCellRect {
		x: CalcPosition;
		y: CalcPosition;
		right: CalcPosition;
		bottom: CalcPosition;
	}

	/// The primitives of a block by layer, listed from the bottom layer to the top one.
	export interface CalcBlockLayers {
		background?: Primitive[];
		borders?: Primitive[];
		text?: Primitive[];
		extras?: Primitive[];
		marks?: Primitive[];
		gridCover?: Primitive[];
	}

	export interface CalcBlockContent<Rect = CalcVectorRect> {
		/// The cells of the block.
		bounds: Rect;
		/// The painted area. Text overflow and merged cells can reach past bounds.
		ink?: Rect;
		layers: CalcBlockLayers;
	}

	/// One part of a block. A part without layers draws nothing.
	export interface CalcBlockPartContent<Rect = CalcVectorRect> {
		ink?: Rect;
		layers?: CalcBlockLayers;
	}

	/// One part of a block, with the version at which it last changed.
	export interface CalcBlockPart<Rect = CalcVectorRect> {
		version: number;
		content: CalcBlockPartContent<Rect>;
	}

	/// The parts that changed after version from. From 0 sends the whole block.
	export interface CalcBlockUpdate {
		kind: 'cells';
		blockRow: number;
		blockColumn: number;
		startRow: number;
		startColumn: number;
		endRow: number;
		endColumn: number;
		/// The newest version of the parts of the block.
		version: number;
		from: number;
		/// The cells of the block.
		bounds: CalcCellRect;
		/// The shadows and borders of the whole block.
		borders?: CalcBlockPart<CalcCellRect>;
		/// The other layers of the rows that changed, each named by its row.
		rows?: Array<CalcBlockPart<CalcCellRect> & { row: number }>;
		/// The objects anchored in the block that changed, in draw order.
		objects?: CalcDrawingEntry[];
		/// The objects that left the block, by id. Absent from a whole block.
		removed?: number[];
	}

	/// A block as the client holds it, with its rectangles as sent, or placed at the zoom of the
	/// view.
	export interface CalcBlockEntry<Rect = CalcVectorRect> {
		kind: 'cells';
		blockRow: number;
		blockColumn: number;
		startRow: number;
		startColumn: number;
		endRow: number;
		endColumn: number;
		/// The newest version of the parts held.
		version: number;
		/// The sheet version up to which the held parts match the engine.
		current: number;
		borders: CalcBlockPart<Rect> | undefined;
		/// The parts of the rows that draw something, by row.
		rows: Map<number, CalcBlockPart<Rect>>;
		/// All parts joined: the borders, then the rows in order.
		content: CalcBlockContent<Rect>;
		/// The ids of the objects held from this block.
		objects: Set<number>;
	}

	/// The layer is 'back' for an object under the cells, and 'front' or 'internal' for one over.
	/// Every position is in twips from the top-left corner of the anchor cell, which members of a
	/// group share with the group.
	export type CalcDrawingContent = Omit<SlideObject, 'layer'> & {
		layer: 'back' | 'front' | 'internal';
		primitives: Primitive[];
		anchorColumn: number;
		anchorRow: number;
	};

	/// A block that holds objects, with the cells all of them paint over.
	export interface CalcObjectBlock {
		blockRow: number;
		blockColumn: number;
		startColumn: number;
		startRow: number;
		endColumn: number;
		endRow: number;
	}

	export interface CalcDrawingEntry {
		kind: 'drawing';
		id: number;
		version: number;
		content: CalcDrawingContent;
	}

	/// Text a view edits in place, in twips. A cell edit names its cells and is relative to the
	/// first one. A shape edit names the shape as its parent and is relative to the top-left corner
	/// of the sheet.
	export interface CalcTextEdit {
		kind: 'texteditoverlay';
		viewId: number;
		parent?: number;
		column?: number;
		row?: number;
		endColumn?: number;
		endRow?: number;
		background?: string;
		primitives: Primitive[];
	}

	export interface CalcSheetEntry {
		kind: 'sheet';
		name?: string;
		rtl?: boolean;
		backdrop?: string;
		tabColor?: string;
	}

	/// A .uno:VectorPrimitives answer, or a pushed delta of changes after from.
	export interface CalcVectorResponse {
		type: 'vectorprimitives' | 'vectorprimitivesdelta';
		docType: 'spreadsheet';
		mode?: number;
		partId: string;
		epoch: number;
		/// The id of the request this answers. A pushed delta has none.
		requestId?: number;
		/// The keys of the blocks the request asked for. A pushed delta has none.
		blocks?: string[];
		from?: number;
		version?: number;
		blockRows?: number;
		blockColumns?: number;
		sheet?: CalcSheetEntry;
		objects?: CalcBlockUpdate[];
		/// The blocks that hold objects, whole, whenever one of them changed.
		objectBlocks?: CalcObjectBlock[];
		drawOrder?: number[];
		textEdits?: CalcTextEdit[];
		/// The font attributes that the cell text of the answer names by id.
		textStyles?: { [id: string]: CalcTextStyle };
		/// The newest input message of this view that the answer includes.
		inputSequence?: number;
	}

	/// What the client holds of one sheet in one mode.
	export interface CalcSheetData {
		part: number;
		mode: number;
		/// The newest version of the sheet the client has heard of.
		version: number;
		/// True once a whole answer arrived, with drawing objects and settings.
		complete: boolean;
		blockRows: number;
		blockColumns: number;
		sheet: CalcSheetEntry | undefined;
		blocks: Map<string, CalcBlockEntry<CalcCellRect>>;
		/// Blocks held or asked for that may be out of date.
		staleBlocks: Set<string>;
		/// The objects held, from the blocks held.
		objects: Map<number, CalcDrawingEntry>;
		drawOrder: number[];
		objectBlocks: CalcObjectBlock[];
		textEdits: CalcTextEdit[];
	}

	/// Font attributes of a text portion, with the face the engine resolved.
	export type CalcTextStyle = Partial<TextSimplePortionPrimitive>;

	/// A text portion with a style id and, for plain upright text, an origin.
	interface CompactTextPortion {
		style?: number;
		x?: CalcPosition;
		y?: CalcPosition;
	}

	/// Expands compact text portions in place. Unknown styles stay as they are.
	export function calcApplyTextStyles(
		primitives: Primitive[] | undefined,
		styles: Map<number, CalcTextStyle>,
	): void {
		for (const primitive of primitives ?? []) {
			const portion = primitive as TextSimplePortionPrimitive &
				CompactTextPortion;
			const style =
				portion.style !== undefined ? styles.get(portion.style) : undefined;
			if (style) {
				Object.assign(portion, style);
				delete portion.style;
				if (
					!portion.matrix &&
					portion.x !== undefined &&
					portion.y !== undefined
				) {
					const size = portion.fontSize ?? 0;
					// The origin stays a position, which the placer reads.
					portion.matrix = [
						size,
						0,
						0,
						size,
						portion.x as unknown as number,
						portion.y as unknown as number,
					];
					delete portion.x;
					delete portion.y;
				}
			}
			calcApplyTextStyles(primitive.children, styles);
			// A strikeout drawn with a character is a text of its own among the lines.
			const decorated = primitive as TextDecoratedPortionPrimitive;
			calcApplyTextStyles(decorated.decorations, styles);
			calcApplyTextStyles(decorated.decorationsInTextColor, styles);
		}
	}

	function joinRanges(
		first: CalcVectorRect | undefined,
		second: CalcVectorRect | undefined,
	): CalcVectorRect | undefined {
		if (!first) return second;
		if (!second) return first;
		const x = Math.min(first.x, second.x);
		const y = Math.min(first.y, second.y);
		return {
			x,
			y,
			width: Math.max(first.x + first.width, second.x + second.width) - x,
			height: Math.max(first.y + first.height, second.y + second.height) - y,
		};
	}

	function isBefore(first: CalcPosition, second: CalcPosition): boolean {
		return (
			first[0] < second[0] || (first[0] === second[0] && first[1] < second[1])
		);
	}

	/// The smallest rectangle that holds both.
	function joinCellRanges(
		first: CalcCellRect | undefined,
		second: CalcCellRect | undefined,
	): CalcCellRect | undefined {
		if (!first) return second;
		if (!second) return first;
		const earlier = (a: CalcPosition, b: CalcPosition): CalcPosition =>
			isBefore(b, a) ? b : a;
		const later = (a: CalcPosition, b: CalcPosition): CalcPosition =>
			isBefore(a, b) ? b : a;
		return {
			x: earlier(first.x, second.x),
			y: earlier(first.y, second.y),
			right: later(first.right, second.right),
			bottom: later(first.bottom, second.bottom),
		};
	}

	/// The held block with the update applied. Undefined if its base is not held.
	export function calcUpdatedBlock(
		held: CalcBlockEntry<CalcCellRect> | undefined,
		update: CalcBlockUpdate,
		current: number,
	): CalcBlockEntry<CalcCellRect> | undefined {
		if (update.from > 0 && (!held || held.current < update.from))
			return undefined;
		const base = update.from > 0 ? held : undefined;

		const rows = new Map<number, CalcBlockPart<CalcCellRect>>(
			base ? base.rows : [],
		);
		for (const row of update.rows ?? []) {
			if (row.content.layers) rows.set(row.row, row);
			else rows.delete(row.row);
		}
		let borders = base ? base.borders : undefined;
		if (update.borders)
			borders = update.borders.content.layers ? update.borders : undefined;

		const parts: CalcBlockPart<CalcCellRect>[] = [];
		if (borders) parts.push(borders);
		for (const row of Array.from(rows.keys()).sort((a, b) => a - b))
			parts.push(rows.get(row) as CalcBlockPart<CalcCellRect>);

		const layers: CalcBlockLayers = {};
		let ink: CalcCellRect | undefined = undefined;
		for (const part of parts) {
			ink = joinCellRanges(ink, part.content.ink);
			const partLayers = part.content.layers ?? {};
			for (const name of Object.keys(partLayers) as Array<
				keyof CalcBlockLayers
			>) {
				const primitives = partLayers[name];
				if (!primitives) continue;
				layers[name] = (layers[name] ?? []).concat(primitives);
			}
		}

		return {
			kind: 'cells',
			blockRow: update.blockRow,
			blockColumn: update.blockColumn,
			startRow: update.startRow,
			startColumn: update.startColumn,
			endRow: update.endRow,
			endColumn: update.endColumn,
			version: update.version,
			current: Math.max(current, update.version),
			borders,
			rows,
			content: { bounds: update.bounds, ink, layers },
			// The objects follow in the answer. A whole block names all of its own.
			objects: new Set(base ? base.objects : []),
		};
	}

	/// The block without the stored text of the given cells, which views edit: the edit is drawn
	/// over such a cell instead, and its stored text would show where the edit does not reach.
	/// The parts of other rows stay shared.
	export function calcBlockWithoutCellText(
		block: CalcBlockEntry,
		cells: Array<{ column: number; row: number }>,
	): CalcBlockEntry {
		const rows = new Map(block.rows);
		for (const cell of cells) {
			const part = rows.get(cell.row);
			const text = part?.content.layers?.text;
			if (!part || !text) continue;
			const kept = text.filter((primitive) => {
				const fields = primitive as unknown as {
					type?: string;
					column?: number;
					row?: number;
				};
				return !(
					fields.type === 'calcCellText' &&
					fields.column === cell.column &&
					fields.row === cell.row
				);
			});
			if (kept.length === text.length) continue;
			rows.set(cell.row, {
				version: part.version,
				content: {
					...part.content,
					layers: { ...part.content.layers, text: kept },
				},
			});
		}
		return { ...block, rows };
	}

	/// A strip of rows that draw text, sparklines or marks, and its painted area.
	export interface CalcTextStrip {
		/// The index of the strip in its block, from the first row on.
		strip: number;
		parts: CalcBlockPart[];
		ink: CalcVectorRect;
	}

	/// The text strips of a block in row order. Rows without ink are skipped.
	export function calcTextStrips(
		block: CalcBlockEntry,
		stripRows: number,
	): CalcTextStrip[] {
		const strips = new Map<number, CalcTextStrip>();
		for (const [row, part] of block.rows) {
			const layers = part.content.layers;
			const ink = part.content.ink;
			if (!layers || !ink) continue;
			if (
				!layers.text?.length &&
				!layers.extras?.length &&
				!layers.marks?.length
			)
				continue;
			const index = Math.floor((row - block.startRow) / stripRows);
			const held = strips.get(index);
			if (held) {
				held.parts.push(part);
				held.ink = joinRanges(held.ink, ink) as CalcVectorRect;
			} else strips.set(index, { strip: index, parts: [part], ink });
		}
		return Array.from(strips.values()).sort((a, b) => a.strip - b.strip);
	}

	/// The key a block is filed under: its block row and block column.
	export function calcBlockKey(blockRow: number, blockColumn: number): string {
		return String(blockRow) + ':' + String(blockColumn);
	}
}
