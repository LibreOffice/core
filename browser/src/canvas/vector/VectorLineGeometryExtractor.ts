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
	The line geometry of a primitive tree: every hairline and every stroke in it, as polygons in
	the coordinates the tree was asked in. It is the engine's LineGeometryExtractor2D for the
	primitives as they arrive here. The walk descends into whatever has children and carries the
	transformations down with it, so it knows nothing of how any object's tree is built; it only
	knows which primitives draw a line and how a transformation maps what lies under it.
*/
namespace cool {
	export class VectorLineGeometryExtractor {
		/// The polygons of every line drawn by these primitives, in the order the tree draws them.
		public static extract(
			primitives: Primitive[] | undefined,
		): ObjectPathPolygon[] {
			const found: ObjectPathPolygon[] = [];
			VectorLineGeometryExtractor.walk(primitives, Matrix2D.IDENTITY, found);
			return found;
		}

		private static walk(
			primitives: Primitive[] | undefined,
			matrix: Matrix2D,
			found: ObjectPathPolygon[],
		): void {
			if (!primitives) return;

			for (const primitive of primitives) {
				if (!primitive) continue;
				const fields = primitive as Primitive & {
					matrix?: number[];
					path?: string;
				};

				switch (primitive.type) {
					case TransformPrimitive.type: {
						const child = Matrix2D.fromArray(fields.matrix);
						VectorLineGeometryExtractor.walk(
							primitive.children,
							child ? child.then(matrix) : matrix,
							found,
						);
						continue;
					}

					case PolygonHairlinePrimitive.type:
					case PolygonStrokePrimitive.type:
					case PolyPolygonStrokePrimitive.type:
						if (fields.path)
							for (const polygon of SvgPathReader.read(fields.path))
								found.push(VectorLineGeometryExtractor.mapped(polygon, matrix));
						continue;

					default:
						VectorLineGeometryExtractor.walk(primitive.children, matrix, found);
				}
			}
		}

		private static mapped(
			polygon: ObjectPathPolygon,
			matrix: Matrix2D,
		): ObjectPathPolygon {
			return {
				closed: polygon.closed,
				points: polygon.points.map((point: ObjectPathPoint) => {
					const at = matrix.apply(point.x, point.y);
					const one: ObjectPathPoint = { x: at.x, y: at.y };
					if (point.behindX !== undefined && point.behindY !== undefined) {
						const behind = matrix.apply(point.behindX, point.behindY);
						one.behindX = behind.x;
						one.behindY = behind.y;
					}
					if (point.aheadX !== undefined && point.aheadY !== undefined) {
						const ahead = matrix.apply(point.aheadX, point.aheadY);
						one.aheadX = ahead.x;
						one.aheadY = ahead.y;
					}
					return one;
				}),
			};
		}
	}

	/*
		A reader of the path data the primitives carry, the "d" of an SVG path: moves, lines,
		cubic and quadratic curves and closes, each in absolute or relative form, as the engine
		writes them and as any SVG may. A quadratic curve becomes the cubic that draws the same.
	*/
	export class SvgPathReader {
		public static read(data: string): ObjectPathPolygon[] {
			const polygons: ObjectPathPolygon[] = [];
			const tokens = data.match(
				/[a-zA-Z]|[-+]?(?:\d+\.?\d*|\.\d+)(?:[eE][-+]?\d+)?/g,
			);
			if (!tokens) return polygons;

			let polygon: ObjectPathPolygon | undefined;
			let x = 0;
			let y = 0;
			let startX = 0;
			let startY = 0;
			// The second control point of the last curve, mirrored for a smooth continuation.
			let lastControlX: number | undefined;
			let lastControlY: number | undefined;
			let command = '';
			let at = 0;

			const number = (): number => {
				const value = parseFloat(tokens[at++]);
				return isNaN(value) ? 0 : value;
			};
			const hasNumber = (): boolean =>
				at < tokens.length && !/[a-zA-Z]/.test(tokens[at]);
			const begin = (): ObjectPathPolygon => {
				polygon = { points: [] };
				polygons.push(polygon);
				return polygon;
			};
			const last = (): ObjectPathPoint | undefined =>
				polygon && polygon.points.length
					? polygon.points[polygon.points.length - 1]
					: undefined;
			const point = (px: number, py: number): void => {
				const current = polygon ?? begin();
				current.points.push({ x: px, y: py });
			};
			const curve = (
				c1x: number,
				c1y: number,
				c2x: number,
				c2y: number,
				px: number,
				py: number,
			): void => {
				const current = polygon ?? begin();
				const previous = last();
				if (previous) {
					previous.aheadX = c1x;
					previous.aheadY = c1y;
				}
				current.points.push({ x: px, y: py, behindX: c2x, behindY: c2y });
				lastControlX = c2x;
				lastControlY = c2y;
			};

			while (at < tokens.length) {
				const token = tokens[at];
				if (/[a-zA-Z]/.test(token)) {
					command = token;
					at++;
				} else if (command === 'M') command = 'L';
				else if (command === 'm') command = 'l';
				else if (!command) break;

				const relative = command === command.toLowerCase();
				const dx = relative ? x : 0;
				const dy = relative ? y : 0;

				switch (command.toUpperCase()) {
					case 'M': {
						x = number() + dx;
						y = number() + dy;
						startX = x;
						startY = y;
						begin();
						point(x, y);
						lastControlX = lastControlY = undefined;
						break;
					}
					case 'L': {
						x = number() + dx;
						y = number() + dy;
						point(x, y);
						lastControlX = lastControlY = undefined;
						break;
					}
					case 'H': {
						x = number() + dx;
						point(x, y);
						lastControlX = lastControlY = undefined;
						break;
					}
					case 'V': {
						y = number() + dy;
						point(x, y);
						lastControlX = lastControlY = undefined;
						break;
					}
					case 'C': {
						const c1x = number() + dx;
						const c1y = number() + dy;
						const c2x = number() + dx;
						const c2y = number() + dy;
						x = number() + dx;
						y = number() + dy;
						curve(c1x, c1y, c2x, c2y, x, y);
						break;
					}
					case 'S': {
						const c1x = lastControlX !== undefined ? 2 * x - lastControlX : x;
						const c1y = lastControlY !== undefined ? 2 * y - lastControlY : y;
						const c2x = number() + dx;
						const c2y = number() + dy;
						x = number() + dx;
						y = number() + dy;
						curve(c1x, c1y, c2x, c2y, x, y);
						break;
					}
					case 'Q': {
						const qx = number() + dx;
						const qy = number() + dy;
						const fromX = x;
						const fromY = y;
						x = number() + dx;
						y = number() + dy;
						curve(
							fromX + (2 / 3) * (qx - fromX),
							fromY + (2 / 3) * (qy - fromY),
							x + (2 / 3) * (qx - x),
							y + (2 / 3) * (qy - y),
							x,
							y,
						);
						// A smooth quadratic continues from the quadratic control point.
						lastControlX = qx;
						lastControlY = qy;
						break;
					}
					case 'T': {
						const qx = lastControlX !== undefined ? 2 * x - lastControlX : x;
						const qy = lastControlY !== undefined ? 2 * y - lastControlY : y;
						const fromX = x;
						const fromY = y;
						x = number() + dx;
						y = number() + dy;
						curve(
							fromX + (2 / 3) * (qx - fromX),
							fromY + (2 / 3) * (qy - fromY),
							x + (2 / 3) * (qx - x),
							y + (2 / 3) * (qy - y),
							x,
							y,
						);
						lastControlX = qx;
						lastControlY = qy;
						break;
					}
					case 'Z': {
						if (polygon) polygon.closed = true;
						x = startX;
						y = startY;
						polygon = undefined;
						lastControlX = lastControlY = undefined;
						break;
					}
					default:
						// An arc or anything unknown: its numbers are skipped.
						while (hasNumber()) at++;
				}
			}

			return polygons;
		}
	}
}
