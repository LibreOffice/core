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
 * The drawing objects of a sheet in tile twips. The engine writes an object in print twips from the
 * top-left corner of its anchor cell. The client maps the two corners of the object's box through
 * the cell edges rounded to pixels, and places the object by the line through them, so both edges
 * land where the grid puts them and the inside is stretched evenly between.
 */

namespace cool {
	/// A line from print twips to tile twips along each axis: tile = scale * print + move. The
	/// object form takes print twips from the anchor cell, the sheet form takes print twips of the
	/// sheet.
	export interface CalcObjectPlacing {
		scaleX: number;
		scaleY: number;
		moveX: number;
		moveY: number;
	}

	export const calcIdentityPlacing: CalcObjectPlacing = {
		scaleX: 1,
		scaleY: 1,
		moveX: 0,
		moveY: 0,
	};

	function placedX(placing: CalcObjectPlacing, x: number): number {
		return placing.scaleX * x + placing.moveX;
	}

	function placedY(placing: CalcObjectPlacing, y: number): number {
		return placing.scaleY * y + placing.moveY;
	}

	/// A mapping of the unit square placed: each axis of the matrix is scaled and its origin moved.
	export function calcPlacedMatrix(
		placing: CalcObjectPlacing,
		matrix: number[],
	): number[] {
		const [a, b, c, d, e, f] = matrix;
		return [
			placing.scaleX * a,
			placing.scaleY * b,
			placing.scaleX * c,
			placing.scaleY * d,
			placedX(placing, e),
			placedY(placing, f),
		];
	}

	/// The point in print twips that the placing takes to the tile twips point.
	export function calcPointInPrintTwips(
		placing: CalcObjectPlacing,
		x: number,
		y: number,
	): Point {
		return new Point(
			(x - placing.moveX) / placing.scaleX,
			(y - placing.moveY) / placing.scaleY,
		);
	}

	/// The mapping of the unit square in print twips that the placing takes to the given one in
	/// tile twips.
	export function calcMatrixInPrintTwips(
		placing: CalcObjectPlacing,
		matrix: number[],
	): number[] {
		const [a, b, c, d, e, f] = matrix;
		const origin = calcPointInPrintTwips(placing, e, f);
		return [
			a / placing.scaleX,
			b / placing.scaleY,
			c / placing.scaleX,
			d / placing.scaleY,
			origin.x,
			origin.y,
		];
	}
	/// The id of a drawing layer of a sheet, as the engine numbers it.
	export function calcLayerId(layer: CalcDrawingContent['layer']): number {
		switch (layer) {
			case 'front':
				return 0;
			case 'back':
				return 1;
			default:
				return 2;
		}
	}

	function placedPathPoint(
		placing: CalcObjectPlacing,
		point: ObjectPathPoint,
	): ObjectPathPoint {
		const placed: ObjectPathPoint = {
			...point,
			x: placedX(placing, point.x),
			y: placedY(placing, point.y),
		};
		if (point.behindX !== undefined)
			placed.behindX = placedX(placing, point.behindX);
		if (point.behindY !== undefined)
			placed.behindY = placedY(placing, point.behindY);
		if (point.aheadX !== undefined)
			placed.aheadX = placedX(placing, point.aheadX);
		if (point.aheadY !== undefined)
			placed.aheadY = placedY(placing, point.aheadY);
		return placed;
	}

	function placedTie(
		placing: CalcObjectPlacing,
		tie: ConnectorTie,
	): ConnectorTie {
		const placed: ConnectorTie = { ...tie };
		if (tie.x !== undefined) placed.x = placedX(placing, tie.x);
		if (tie.y !== undefined) placed.y = placedY(placing, tie.y);
		return placed;
	}

	function samePlacing(a: CalcObjectPlacing, b: CalcObjectPlacing): boolean {
		return (
			a.scaleX === b.scaleX &&
			a.scaleY === b.scaleY &&
			a.moveX === b.moveX &&
			a.moveY === b.moveY
		);
	}

	/// The object with every position taken through the placing, in its object form. Glue points
	/// and handle ranges are given on the object, so they stay.
	export function calcPlacedObject(
		content: CalcDrawingContent,
		placing: CalcObjectPlacing,
	): SlideObject {
		const object: SlideObject = {
			...content,
			layer: calcLayerId(content.layer),
			primitives: content.primitives,
		};
		if (samePlacing(placing, calcIdentityPlacing)) return object;

		if (object.x !== undefined) object.x = placedX(placing, object.x);
		if (object.y !== undefined) object.y = placedY(placing, object.y);
		if (object.width !== undefined) object.width *= placing.scaleX;
		if (object.height !== undefined) object.height *= placing.scaleY;
		if (content.transform && content.transform.length >= 6)
			object.transform = calcPlacedMatrix(placing, content.transform);
		if (content.handles)
			object.handles = content.handles.map((handle) => ({
				...handle,
				x: placedX(placing, handle.x),
				y: placedY(placing, handle.y),
			}));
		if (content.path)
			object.path = content.path.map((polygon) => ({
				...polygon,
				points: polygon.points.map((point) => placedPathPoint(placing, point)),
			}));
		if (content.routing)
			object.routing = {
				...content.routing,
				tieAtStart: placedTie(placing, content.routing.tieAtStart),
				tieAtEnd: placedTie(placing, content.routing.tieAtEnd),
			};
		const place: TransformPrimitive = {
			type: TransformPrimitive.type,
			matrix: [
				placing.scaleX,
				0,
				0,
				placing.scaleY,
				placing.moveX,
				placing.moveY,
			],
			children: content.primitives,
		};
		object.primitives = [place];
		return object;
	}

	/// A position along one dimension of the sheet in print twips, mapped to tile twips.
	export type CalcTwipsMap = (position: number) => number;

	/// Maps print twips to tile twips, for the drawing objects, which are in twips from their anchor
	/// cell. A position maps by the cell it falls in and keeps its share of the cell. The same cell
	/// edges repeat, so they are cached.
	export function calcTwipsMap(dimension: SheetDimension): CalcTwipsMap {
		const printSize = dimension.getSize('printtwips') ?? 0;
		const tileSize = dimension.getSize('tiletwips') ?? 0;
		const known = new Map<number, number>();
		return (position: number): number => {
			if (position <= 0) return position;
			if (position >= printSize) return tileSize + position - printSize;
			let mapped = known.get(position);
			if (mapped === undefined) {
				mapped = dimension.getTileTwipsPosFromPrint(position);
				known.set(position, mapped);
			}
			return mapped;
		};
	}

	/// The line along one axis through the two edges of a box: each edge in print twips of the
	/// sheet goes to its place in tile twips, and the inside is stretched evenly. A box without
	/// extent keeps the scale of one.
	function fittedAxis(
		map: CalcTwipsMap,
		anchorPrint: number,
		start: number,
		extent: number,
	): { scale: number; move: number } {
		const near = anchorPrint + start;
		const nearTile = map(near);
		if (extent < 1) return { scale: 1, move: nearTile - start };
		const scale = (map(near + extent) - nearTile) / extent;
		return { scale, move: nearTile - scale * start };
	}

	interface CalcPlacedEntry {
		entry: CalcDrawingEntry;
		/// The placing in its object form, from the anchor cell.
		placing: CalcObjectPlacing;
		/// The placing in its sheet form, from print twips of the sheet.
		sheetPlacing: CalcObjectPlacing;
		object: SlideObject;
	}

	/// The drawing objects of one sheet in tile twips.
	export class CalcObjectView {
		readonly data: VectorPrimitivesData;

		private _placed: Map<number, CalcPlacedEntry> = new Map();

		constructor(part: number) {
			this.data = {
				partId: String(part),
				mode: 0,
				slideWidth: 0,
				slideHeight: 0,
				objects: new Map(),
				order: [],
			};
		}

		/// An unchanged object keeps its old view object and the edits made to it.
		update(sheet: CalcSheetData, geometry: SheetGeometry): void {
			const placed: Map<number, CalcPlacedEntry> = new Map();
			const order: number[] = [];
			const objects: Map<number, SlideObject> = new Map();
			const columns = geometry.getColumnsGeometry();
			const rows = geometry.getRowsGeometry();
			const mapX = calcTwipsMap(columns);
			const mapY = calcTwipsMap(rows);
			for (const id of sheet.drawOrder) {
				const entry = sheet.objects.get(id);
				if (!entry) continue;
				const content = entry.content;
				const columnPrint = columns.getElementDataAny(
					content.anchorColumn,
					'printtwips',
				);
				const rowPrint = rows.getElementDataAny(
					content.anchorRow,
					'printtwips',
				);
				// An anchor cell the geometry does not reach places nothing.
				if (!columnPrint || !rowPrint) continue;
				const anchorX = columnPrint.startpos;
				const anchorY = rowPrint.startpos;
				const horizontal = fittedAxis(
					mapX,
					anchorX,
					content.x ?? 0,
					content.width ?? 0,
				);
				const vertical = fittedAxis(
					mapY,
					anchorY,
					content.y ?? 0,
					content.height ?? 0,
				);
				const placing: CalcObjectPlacing = {
					scaleX: horizontal.scale,
					scaleY: vertical.scale,
					moveX: horizontal.move,
					moveY: vertical.move,
				};

				const held = this._placed.get(id);
				const one: CalcPlacedEntry =
					held && held.entry === entry && samePlacing(held.placing, placing)
						? held
						: {
								entry: entry,
								placing: placing,
								sheetPlacing: {
									scaleX: placing.scaleX,
									scaleY: placing.scaleY,
									moveX: placing.moveX - placing.scaleX * anchorX,
									moveY: placing.moveY - placing.scaleY * anchorY,
								},
								object: calcPlacedObject(content, placing),
							};
				placed.set(id, one);
				objects.set(id, one.object);
				order.push(id);
			}
			this._placed = placed;

			const size = geometry.getSize('tiletwips');
			this.data.version = sheet.version;
			this.data.slideWidth = size.x;
			this.data.slideHeight = size.y;
			this.data.objects = objects;
			this.data.order = order;
		}

		/// The placing of the object in its sheet form, undefined for unknown objects.
		placingOf(id: number): CalcObjectPlacing | undefined {
			return this._placed.get(id)?.sheetPlacing;
		}
	}

	/// How the objects of a sheet are taken from print twips into tile twips.
	export interface CalcPlacement {
		/// The placing of the object in its sheet form, nothing for an unknown object.
		placingOf(id: number): CalcObjectPlacing | undefined;
		/// The outermost group holding the object, or the object itself.
		outermostOf(id: number): number;
		/// The leaf members of a group, or the object itself when it is no group.
		membersOf(outermost: number): number[];
		/// The placing of the object with its corner at the point in tile twips: its scale as it
		/// is, moved so the point goes to its place in the sheet.
		placingAt(id: number, point: Point): CalcObjectPlacing;
	}

	/// Twips keys of each message. An empty list means the positions are nested.
	const calcPositionKeys: { [command: string]: string[] } = {
		setobjecthandle: ['x', 'y'],
		setobjectcontrolpoint: ['x', 'y'],
		cropobject: ['x1', 'y1', 'x2', 'y2'],
		setobjectpoints: [],
		setobjecttransform: [],
	};

	/// True when the message is an object command with positions in tile twips.
	export function calcMessageHasPositions(message: string): boolean {
		return calcPositionKeys[message.split(' ', 1)[0]] !== undefined;
	}

	/// The top-left corner of the box that a mapping of the unit square covers.
	function cornerOfMapping(values: number[]): Point {
		const [a, b, c, d, e, f] = values;
		const xs = [e, a + e, c + e, a + c + e];
		const ys = [f, b + f, d + f, b + d + f];
		return new Point(Math.min(...xs), Math.min(...ys));
	}

	/// One mapping of a transform message, with its original text.
	interface NamedMapping {
		text: string;
		id: number;
		matrix: number[];
	}

	function parseMapping(text: string): NamedMapping | null {
		const values = text.split(',').map(Number);
		if (values.length !== 7 || values.some((value) => isNaN(value)))
			return null;
		return { text, id: values[0], matrix: values.slice(1) };
	}

	/// The top-left corner of the box that the mappings together cover.
	function cornerOfMappings(mappings: NamedMapping[]): Point {
		const corners = mappings.map((mapping) => cornerOfMapping(mapping.matrix));
		return new Point(
			Math.min(...corners.map((corner) => corner.x)),
			Math.min(...corners.map((corner) => corner.y)),
		);
	}

	/// Takes transform mappings back to print twips. When the message carries all members of an
	/// outermost group, their mappings give the new corner of the group and so its placing. A
	/// member moved alone keeps its placing.
	function transformInPrintTwips(
		mappings: string,
		placement: CalcPlacement,
	): string {
		const parsed = mappings
			.split(';')
			.map((text) => parseMapping(text) ?? { text, id: NaN, matrix: [] });
		const byOutermost = new Map<number, NamedMapping[]>();
		for (const mapping of parsed) {
			if (isNaN(mapping.id)) continue;
			const outermost = placement.outermostOf(mapping.id);
			const list = byOutermost.get(outermost) ?? [];
			list.push(mapping);
			byOutermost.set(outermost, list);
		}
		const corners = new Map<number, Point>();
		for (const [outermost, list] of byOutermost) {
			const carried = new Set(list.map((mapping) => mapping.id));
			const whole = placement
				.membersOf(outermost)
				.every((member) => carried.has(member));
			if (whole) corners.set(outermost, cornerOfMappings(list));
		}
		return parsed
			.map((mapping) => {
				if (isNaN(mapping.id)) return mapping.text;
				const corner = corners.get(placement.outermostOf(mapping.id));
				const placing = corner
					? placement.placingAt(mapping.id, corner)
					: (placement.placingOf(mapping.id) ??
						placement.placingAt(mapping.id, cornerOfMapping(mapping.matrix)));
				const matrix = calcMatrixInPrintTwips(placing, mapping.matrix);
				return [String(mapping.id)]
					.concat(matrix.map((value) => value.toFixed(4)))
					.join(',');
			})
			.join(';');
	}

	/// Each change holds polygon, point, part, x, y and continuity.
	function pathChangesInPrintTwips(
		changes: string,
		placing: CalcObjectPlacing,
	): string {
		return changes
			.split(';')
			.map((change) => {
				const fields = change.split(',');
				if (fields.length < 5) return change;
				const point = calcPointInPrintTwips(
					placing,
					Number(fields[3]),
					Number(fields[4]),
				);
				fields[3] = String(Math.round(point.x));
				fields[4] = String(Math.round(point.y));
				return fields.join(',');
			})
			.join(';');
	}

	/// Moves the positions in object messages back into print twips.
	export function calcMessageInPrintTwips(
		message: string,
		placement: CalcPlacement,
	): string {
		const tokens = message.split(' ');
		const keys = calcPositionKeys[tokens[0]];
		if (!keys) return message;

		if (tokens[0] === 'setobjecttransform') {
			return tokens
				.map((token) =>
					token.startsWith('to=')
						? 'to=' + transformInPrintTwips(token.substring(3), placement)
						: token,
				)
				.join(' ');
		}

		const idToken = tokens.find((token) => token.startsWith('id='));
		const placing = idToken
			? placement.placingOf(Number(idToken.substring(3)))
			: undefined;
		if (!placing) return message;

		return tokens
			.map((token) => {
				const equals = token.indexOf('=');
				if (equals < 0) return token;
				const key = token.substring(0, equals);
				const value = token.substring(equals + 1);
				if (tokens[0] === 'setobjectpoints' && key === 'changes')
					return 'changes=' + pathChangesInPrintTwips(value, placing);
				const index = keys.indexOf(key);
				if (index < 0) return token;
				// The keys come in x, y pairs.
				const along =
					index % 2 === 0
						? (Number(value) - placing.moveX) / placing.scaleX
						: (Number(value) - placing.moveY) / placing.scaleY;
				return key + '=' + String(Math.round(along));
			})
			.join(' ');
	}
}
