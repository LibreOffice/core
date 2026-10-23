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

namespace cool {
	/*
		The tail of a caption, laid out from its end and the box of the caption as the office lays
		it out (SdrCaptionObj::ImpCalcTail). The end stays where it is when the caption is moved or
		scaled, the tail follows the box, so a drag of the caption shows the tail laid out anew.
	*/
	export class CaptionTail {
		/// where the tail leaves the box, and whether that is a side left or right of it
		private static leaving(
			caption: ObjectCaption,
			end: Point,
			box: Range2D,
		): { at: Point; acrossTheSide: boolean; side: number } {
			const relative = caption.escapeRelative;
			const alongX =
				box.minX +
				(relative !== undefined
					? (box.width * relative) / 10000
					: (caption.escapeAbsolute ?? 0));
			const alongY =
				box.minY +
				(relative !== undefined
					? (box.height * relative) / 10000
					: (caption.escapeAbsolute ?? 0));

			// the first kind takes the sides the other way round
			const bestFit = caption.escapeDirection === 2;
			const horizontal = caption.escapeDirection === 0;
			const firstKind = caption.type === 1;
			const tryLeftRight = bestFit || horizontal !== firstKind;
			const tryTopBottom = bestFit || horizontal === firstKind;

			let best = {
				at: new Point(box.minX, alongY),
				acrossTheSide: true,
				side: -1,
			};
			if (tryLeftRight) {
				const left = new Point(box.minX - caption.gap, alongY);
				const right = new Point(box.maxX + caption.gap, alongY);
				best =
					end.x - left.x < right.x - end.x
						? { at: left, acrossTheSide: true, side: -1 }
						: { at: right, acrossTheSide: true, side: 1 };
			}

			if (tryTopBottom) {
				const top = new Point(alongX, box.minY - caption.gap);
				const bottom = new Point(alongX, box.maxY + caption.gap);
				const other =
					end.y - top.y < bottom.y - end.y
						? { at: top, acrossTheSide: false, side: -1 }
						: { at: bottom, acrossTheSide: false, side: 1 };

				let takeIt = !bestFit;
				if (!takeIt) {
					const toBest = best.at.subtract(end).length();
					const toOther = other.at.subtract(end).length();
					takeIt = firstKind ? toOther >= toBest : toOther < toBest;
				}
				if (takeIt) best = other;
			}

			return best;
		}

		/// the points of the tail, from its end to the box, in twips
		public static laidOut(
			caption: ObjectCaption,
			end: Point,
			box: Range2D,
		): Point[] {
			const leaves = CaptionTail.leaving(caption, end, box);

			// a line bent once: the end is brought onto the line the tail leaves along
			if (caption.type === 1)
				return [
					leaves.acrossTheSide
						? new Point(leaves.at.x, end.y)
						: new Point(end.x, leaves.at.y),
					leaves.at,
				];

			if (caption.type === 2) return [end, leaves.at];

			// a piece of line comes out of the box first, half the way long where fitted
			const fitted = caption.fitLineLength === true;
			const length = caption.lineLength ?? 0;
			const bend = leaves.acrossTheSide
				? new Point(
						fitted
							? (end.x + leaves.at.x) / 2
							: leaves.at.x + leaves.side * length,
						leaves.at.y,
					)
				: new Point(
						leaves.at.x,
						fitted
							? (end.y + leaves.at.y) / 2
							: leaves.at.y + leaves.side * length,
					);

			return [end, bend, leaves.at];
		}

		/// where the end of the tail of that caption stands, which is its one handle of the kind 9
		public static endOf(object: SlideObject): Point | undefined {
			const handle = object.handles?.find(
				(one: ObjectHandle) => one.kind === 9,
			);
			return handle ? new Point(handle.x, handle.y) : undefined;
		}

		/*
			The primitives of a caption as a change of that mapping leaves it: the end of the tail
			stays, the box goes along, and the tail is laid out anew between them. The primitives
			are the ones drawn through the change. Nothing for an object that is no caption.
		*/
		public static changedBy(
			object: SlideObject,
			matrix: Matrix2D,
		): Primitive[] | undefined {
			const mapping = Matrix2D.fromArray(object.transform);
			const end = CaptionTail.endOf(object);
			if (!object.caption || !object.primitives || !mapping || !end)
				return undefined;

			const tail = CaptionTail.laidOut(
				object.caption,
				end,
				CaptionTail.boxOf(mapping.then(matrix)),
			);
			return CaptionTail.relaid(object.primitives, tail, matrix.invert());
		}

		/*
			Lays the tail of a caption out anew where its primitives hang under one change laid on
			here: the end stays, the box is the one the caption's mapping now gives, and the tail
			is taken back through that change to be drawn under it.
		*/
		public static layAnew(object: SlideObject): void {
			const drawn = object.primitives ?? [];
			const wrap =
				drawn.length === 1 && drawn[0].type === TransformPrimitive.type
					? (drawn[0] as TransformPrimitive)
					: undefined;
			const under = Matrix2D.fromArray(wrap?.matrix);
			const mapping = Matrix2D.fromArray(object.transform);
			const end = CaptionTail.endOf(object);
			if (!object.caption || !wrap || !under || !mapping || !end) return;

			wrap.children = CaptionTail.relaid(
				wrap.children ?? [],
				CaptionTail.laidOut(object.caption, end, CaptionTail.boxOf(mapping)),
				under.invert(),
			);
		}

		/// the box of the caption drawn by that mapping, as an upright range
		public static boxOf(mapping: Matrix2D): Range2D {
			return Range2D.fromPoints([
				mapping.apply(0, 0),
				mapping.apply(1, 0),
				mapping.apply(1, 1),
				mapping.apply(0, 1),
			]);
		}

		/// the first stroke in the primitives, whose line the tail is drawn with
		private static strokeIn(
			primitives: Primitive[],
		): PolygonStrokePrimitive | undefined {
			for (const primitive of primitives) {
				if (primitive.type === PolygonStrokePrimitive.type)
					return primitive as PolygonStrokePrimitive;
				const inside = CaptionTail.strokeIn(primitive.children ?? []);
				if (inside) return inside;
			}
			return undefined;
		}

		/*
			The primitives with each tail of a caption drawn along those points instead, given in
			twips of the page; inside takes them to the place the primitives are drawn in. A tail
			with a line keeps its line without the line ends, a hidden one stays hidden. Only the
			nodes on the way to a tail are copied.
		*/
		public static relaid(
			primitives: Primitive[],
			tail: Point[],
			inside: Matrix2D | null,
		): Primitive[] {
			const path = tail
				.map((point: Point, index: number) => {
					const at = inside ? inside.apply(point.x, point.y) : point;
					return (index ? 'L ' : 'M ') + String(at.x) + ' ' + String(at.y);
				})
				.join(' ');

			const walk = (list: Primitive[]): Primitive[] =>
				list.map((primitive: Primitive) => {
					if (primitive.type === CaptionTailPrimitive.type) {
						const children = primitive.children ?? [];
						const hidden = children[0]?.type === HiddenGeometryPrimitive.type;
						const stroke = CaptionTail.strokeIn(children);
						const line: Primitive =
							stroke && !hidden
								? ({ ...stroke, path: path } as PolygonStrokePrimitive)
								: ({
										type: PolygonHairlinePrimitive.type,
										path: path,
									} as PolygonHairlinePrimitive);

						return {
							...primitive,
							children: hidden
								? [{ type: HiddenGeometryPrimitive.type, children: [line] }]
								: [line],
						};
					}

					if (!primitive.children) return primitive;
					return { ...primitive, children: walk(primitive.children) };
				});

			return walk(primitives);
		}
	}
}
