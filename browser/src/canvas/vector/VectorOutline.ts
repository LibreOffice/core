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

namespace cool {
	/// The fields of a primitive the outline reads.
	interface OutlineFields extends Primitive {
		matrix?: number[];
		path?: string;
		clip?: string;
		bounds?: number[];
		outputRange?: number[];
	}

	/*
		The outline of what primitives draw as geometry, as the hit test walks them: the boundary
		of every fill and the course of every line, through the transforms they hang under. Text is
		left out, so the outline of a shape is not that of the letters in it, and so are shadows and
		the background that covers the whole object. A fill inside a mask, a gradient or a picture
		filling a shape, is outlined by the mask, which is what gives it its shape; an image by the
		area it covers.

		It answers what the outline is, as polygons or as a path to draw, and whether it lies wholly
		inside a given polygon, which is how a lasso takes what it encloses: by what the user sees,
		not by the rectangle the object is laid out in.
	*/
	export class VectorOutline {
		/// How many straight pieces a curve is split into where the outline is tested.
		private static readonly piecesOfACurve: number = 8;

		/// The outline as polygons in the coordinates the primitives are written in, curves kept
		/// with their weights. Nothing where the primitives draw no geometry.
		public static polygonsOf(
			primitives: Primitive[] | undefined,
		): ObjectPathPolygon[] {
			const found: ObjectPathPolygon[] = [];

			const add = (polygons: ObjectPathPolygon[], matrix: Matrix2D): void => {
				for (const polygon of polygons)
					found.push(VectorLineGeometryExtractor.mapped(polygon, matrix));
			};
			const corners = (points: Point[]): ObjectPathPolygon[] => [
				{
					closed: true,
					points: points.map((point: Point) => ({ x: point.x, y: point.y })),
				},
			];
			const ofRange = (range: Range2D | null): ObjectPathPolygon[] =>
				range
					? corners([
							new Point(range.minX, range.minY),
							new Point(range.maxX, range.minY),
							new Point(range.maxX, range.maxY),
							new Point(range.minX, range.maxY),
						])
					: [];
			const ofUnitSquare = (matrix: Matrix2D | null): ObjectPathPolygon[] =>
				matrix
					? corners([
							matrix.apply(0, 0),
							matrix.apply(1, 0),
							matrix.apply(1, 1),
							matrix.apply(0, 1),
						])
					: [];

			const walk = (list: Primitive[] | undefined, matrix: Matrix2D): void => {
				for (const primitive of list ?? []) {
					if (!primitive) continue;
					const fields = primitive as OutlineFields;
					switch (primitive.type) {
						case TextSimplePortionPrimitive.type:
						case TextDecoratedPortionPrimitive.type:
						case BackgroundColorPrimitive.type:
						case PointArrayPrimitive.type:
						case 'shadow':
							break;

						case TransformPrimitive.type:
						case ReflectionPrimitive.type: {
							const own = Matrix2D.fromArray(fields.matrix);
							walk(primitive.children, own ? own.then(matrix) : matrix);
							break;
						}

						case MaskPrimitive.type:
							if (fields.clip) add(SvgPathReader.read(fields.clip), matrix);
							else walk(primitive.children, matrix);
							break;

						case PolygonHairlinePrimitive.type:
						case PolygonStrokePrimitive.type:
						case PolyPolygonStrokePrimitive.type:
						case PolyPolygonColorPrimitive.type:
						case PolyPolygonRGBAPrimitive.type:
						case 'polyPolygonGradient':
						case 'polyPolygonAlphaGradient':
						case 'polyPolygonHatch':
						case 'polyPolygonGraphic':
						case 'patternFill':
							if (fields.path) add(SvgPathReader.read(fields.path), matrix);
							break;

						case SingleLinePrimitive.type: {
							const line = primitive as SingleLinePrimitive;
							add(
								[
									{
										closed: false,
										points: [
											{ x: line.startX ?? 0, y: line.startY ?? 0 },
											{ x: line.endX ?? 0, y: line.endY ?? 0 },
										],
									},
								],
								matrix,
							);
							break;
						}

						case LineRectanglePrimitive.type:
						case FilledRectanglePrimitive.type:
							add(ofRange(Range2D.fromArray(fields.bounds)), matrix);
							break;

						case 'fillGradient':
						case 'fillHatch':
						case 'fillGraphic': {
							const covered = Range2D.fromArray(fields.outputRange);
							add(
								covered
									? ofRange(covered)
									: ofUnitSquare(Matrix2D.fromArray(fields.matrix)),
								matrix,
							);
							break;
						}

						case BitmapPrimitive.type:
						case BitmapAlphaPrimitive.type:
						case GraphicPrimitive.type:
							if (fields.matrix)
								add(ofUnitSquare(Matrix2D.fromArray(fields.matrix)), matrix);
							else walk(primitive.children, matrix);
							break;

						default:
							walk(primitive.children, matrix);
					}
				}
			};

			walk(primitives, Matrix2D.IDENTITY);
			return found;
		}

		/// Those polygons as one path to draw or to hit, curves as curves.
		public static pathOf(polygons: ObjectPathPolygon[]): Path2D {
			const path = new Path2D();
			for (const polygon of polygons) {
				const points = polygon.points;
				if (!points.length) continue;
				path.moveTo(points[0].x, points[0].y);
				const edges = polygon.closed ? points.length : points.length - 1;
				for (let edge = 0; edge < edges; ++edge) {
					const from = points[edge];
					const to = points[(edge + 1) % points.length];
					if (from.aheadX !== undefined || to.behindX !== undefined)
						path.bezierCurveTo(
							from.aheadX ?? from.x,
							from.aheadY ?? from.y,
							to.behindX ?? to.x,
							to.behindY ?? to.y,
							to.x,
							to.y,
						);
					else if (edge < points.length - 1) path.lineTo(to.x, to.y);
				}
				if (polygon.closed) path.closePath();
			}
			return path;
		}

		/*
			Whether what the primitives draw lies wholly inside that area, a polygon closed back to
			its start, both in the coordinates the primitives are written in. Undefined where they
			draw no geometry, so the one who asks can judge such an object another way.
		*/
		public static liesInside(
			primitives: Primitive[] | undefined,
			area: Point[],
		): boolean | undefined {
			const polygons = VectorOutline.polygonsOf(primitives);
			if (!polygons.length) return undefined;
			return VectorOutline.holds(
				area,
				polygons.map((polygon: ObjectPathPolygon) =>
					VectorOutline.flattened(polygon),
				),
			);
		}

		/*
			Whether the area, a polygon closed back to its start, holds those lines wholly: every
			point of them lies inside it, by the even-odd rule, and no edge of the area crosses an
			edge of them, which an area that bends inwards would do with every point still inside.
			Each line is closed or open as it says.
		*/
		public static holds(
			area: Point[],
			lines: { closed: boolean; points: Point[] }[],
		): boolean {
			if (area.length < 3 || !lines.length) return false;

			const inside = (at: Point): boolean => {
				let crossings = 0;
				for (let edge = 0; edge < area.length; ++edge) {
					const from = area[edge];
					const to = area[(edge + 1) % area.length];
					if (
						from.y > at.y !== to.y > at.y &&
						at.x <
							from.x + ((at.y - from.y) * (to.x - from.x)) / (to.y - from.y)
					)
						++crossings;
				}
				return crossings % 2 === 1;
			};
			const side = (a: Point, b: Point, c: Point): number =>
				Math.sign(b.subtract(a).cross(c.subtract(a)));
			const cross = (a: Point, b: Point, c: Point, d: Point): boolean =>
				side(a, b, c) * side(a, b, d) < 0 && side(c, d, a) * side(c, d, b) < 0;

			for (const line of lines) {
				if (!line.points.every(inside)) return false;
				const edges = line.closed ? line.points.length : line.points.length - 1;
				for (let edge = 0; edge < edges; ++edge) {
					const a = line.points[edge];
					const b = line.points[(edge + 1) % line.points.length];
					for (let other = 0; other < area.length; ++other)
						if (cross(area[other], area[(other + 1) % area.length], a, b))
							return false;
				}
			}
			return true;
		}

		/// The polygon as straight pieces, each curve split into some, in points.
		private static flattened(polygon: ObjectPathPolygon): {
			closed: boolean;
			points: Point[];
		} {
			const points = polygon.points;
			const flat: Point[] = [];
			const edges = polygon.closed ? points.length : points.length - 1;
			if (points.length) flat.push(new Point(points[0].x, points[0].y));
			for (let edge = 0; edge < edges; ++edge) {
				const from = points[edge];
				const to = points[(edge + 1) % points.length];
				const curved = from.aheadX !== undefined || to.behindX !== undefined;
				const pieces = curved ? VectorOutline.piecesOfACurve : 1;
				for (let piece = 1; piece <= pieces; ++piece) {
					const t = piece / pieces;
					const s = 1 - t;
					const c1x = from.aheadX ?? from.x;
					const c1y = from.aheadY ?? from.y;
					const c2x = to.behindX ?? to.x;
					const c2y = to.behindY ?? to.y;
					flat.push(
						new Point(
							s * s * s * from.x +
								3 * s * s * t * c1x +
								3 * s * t * t * c2x +
								t * t * t * to.x,
							s * s * s * from.y +
								3 * s * s * t * c1y +
								3 * s * t * t * c2y +
								t * t * t * to.y,
						),
					);
				}
			}
			// a closed polygon came back to its first point, which it holds once
			if (polygon.closed && flat.length > 1) flat.pop();
			return { closed: !!polygon.closed, points: flat };
		}
	}
}
