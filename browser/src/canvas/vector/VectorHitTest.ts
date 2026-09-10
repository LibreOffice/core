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

namespace cool {
	/// What kind of geometry answered a hit test: the body of a shape, its outline, its text, an
	/// image, or one of the points of a point array.
	export type HitKind = 'fill' | 'line' | 'text' | 'bitmap' | 'point';

	/// What a hit test found.
	export interface HitResult {
		kind: HitKind;
		/*
			The geometry that answered is carried to be hit rather than to be seen. An unfilled text
			frame holds such a fill over its whole rectangle, which is what makes the inside of the
			frame pickable although nothing is painted there.
		*/
		hidden: boolean;
		/// The geometry that answered is what only an editing view shows: the frame around an empty
		/// presentation object, or the text that names what belongs in it.
		editView: boolean;
		/// A fill answered within the tolerance of its boundary rather than inside it.
		edge: boolean;
		/*
			The geometry that answered, in the coordinates the primitives were asked in. It is the
			outline of a fill, the line of a stroke, the box of a text portion or the area of an
			image.
		*/
		path?: Path2D;
	}

	/// What the walk carries down: what it was asked to look for, and what the geometry that
	/// answers sits inside.
	interface HitWalk {
		/// Only the text of what is walked can answer. Everything else is passed through, so a
		/// text inside a group or behind a clip is still reached.
		textOnly: boolean;
		hidden: boolean;
		editView: boolean;
		/*
			Maps what is walked now onto the coordinates the walk was asked in. The point travels
			the other way, through the inverse, so this is what puts the geometry that answers back
			where the caller can use it.
		*/
		matrix: number[];
	}

	/// Where a hit test asks, in the coordinates the primitives are written in, and how far from
	/// the geometry still counts as a hit. The tolerance is per axis, so a view that scales the
	/// two axes differently still asks about the same distance on screen.
	export interface HitPosition {
		x: number;
		y: number;
		toleranceX: number;
		toleranceY: number;
	}

	/// The area an object covers, in the coordinates the primitives are written in. Some
	/// primitives paint over the whole of it rather than carrying geometry of their own.
	export interface HitRange {
		minX: number;
		minY: number;
		maxX: number;
		maxY: number;
	}

	/** Tells whether a point hits what a JSON primitive tree paints.
	 *
	 * Each type of primitive is answered the way the document engine answers it: an outline is hit
	 * along its line, a filled polygon anywhere inside it or within the tolerance of its edge, a
	 * stroke within half its width, a text and a bitmap inside the area they cover, and a group
	 * wherever one of its children is hit. A clip is a gate: what it holds can only be hit inside
	 * it. A shadow is never hit.
	 */
	export class VectorHitTest {
		/*
			The tests ask the browser whether a point lies in a path, which needs a drawing context
			but no drawing surface, so one context is kept for all of them. It also measures text,
			whose extent decides the area a text portion covers.
		*/
		private static _context: CanvasRenderingContext2D | null | undefined =
			undefined;

		private static context(): CanvasRenderingContext2D | null {
			if (VectorHitTest._context === undefined)
				VectorHitTest._context = document
					.createElement('canvas')
					.getContext('2d');

			return VectorHitTest._context;
		}

		/** What the point hits among the primitives, or nothing when it hits none of them.
		 *
		 * The primitives are in the order they are painted, and the answer is the last of them
		 * that is hit, so what lies on top of everything else is what answers. Primitives that
		 * paint over the whole object are tested against the range, the area the object covers.
		 */
		public static hit(
			primitives: Primitive[] | undefined,
			position: HitPosition,
			range?: HitRange,
			walk: HitWalk = VectorHitTest.wholeOf(),
		): HitResult | undefined {
			if (!primitives) return undefined;

			let result: HitResult | undefined = undefined;

			primitives.forEach((primitive: Primitive) => {
				const hit = VectorHitTest.hitPrimitive(
					primitive,
					position,
					range,
					walk,
				);
				if (hit) result = hit;
			});

			return result;
		}

		/// Whether the point hits any of the primitives.
		public static isHit(
			primitives: Primitive[] | undefined,
			position: HitPosition,
			range?: HitRange,
		): boolean {
			return VectorHitTest.hit(primitives, position, range) !== undefined;
		}

		/** Whether the point lies in the unit square put through the matrix.
		 *
		 * This is the shape of an object as its own transformation describes it, so it carries the
		 * rotation and the shear that an upright rectangle around the object cannot. What an object
		 * paints beyond that square, a wide outline or a shadow, lies outside it.
		 */
		public static isInUnitSquare(
			matrix: number[] | undefined,
			position: HitPosition,
		): boolean {
			if (!matrix) return false;

			return (
				VectorHitTest.hitFill(
					VectorHitTest.pathOfUnitSquare(matrix),
					position,
				) !== undefined
			);
		}

		/** Whether the point hits the text of the primitives, and nothing else of them.
		 *
		 * The tolerance is what decides how far from the glyphs still counts, per axis, so a
		 * caller that asks about a line of text gives a wide reach along the line and none across
		 * it. The document engine asks the same question, with the same per-axis tolerance, to
		 * decide whether a click would go into the text of a shape.
		 */
		public static isHitText(
			primitives: Primitive[] | undefined,
			position: HitPosition,
		): boolean {
			return (
				VectorHitTest.hit(primitives, position, undefined, {
					...VectorHitTest.wholeOf(),
					textOnly: true,
				}) !== undefined
			);
		}

		public static hitPrimitive(
			primitive: Primitive,
			position: HitPosition,
			range?: HitRange,
			walk: HitWalk = VectorHitTest.wholeOf(),
		): HitResult | undefined {
			if (!primitive || !primitive.type) return undefined;

			const fields = primitive as HitFields;

			switch (primitive.type) {
				case TextSimplePortionPrimitive.type:
				case TextDecoratedPortionPrimitive.type:
					return VectorHitTest.result(
						'text',
						walk,
						VectorHitTest.isInText(
							primitive as TextSimplePortionPrimitive,
							position,
						),
					);

				case TransformPrimitive.type:
					return VectorHitTest.hit(
						primitive.children,
						VectorHitTest.positionThrough(position, fields.matrix),
						range,
						{
							...walk,
							matrix: VectorHitTest.matrixThrough(walk.matrix, fields.matrix),
						},
					);

				case MaskPrimitive.type:
					// The clip decides whether anything inside it can be reached at all.
					if (
						fields.clip &&
						VectorHitTest.hitFill(new Path2D(fields.clip), position) ===
							undefined
					)
						return undefined;

					return VectorHitTest.hit(primitive.children, position, range, walk);

				case HiddenGeometryPrimitive.type:
					// What this holds is there to be hit, so it answers, and says so.
					return VectorHitTest.hit(primitive.children, position, range, {
						...walk,
						hidden: true,
					});

				case ExclusiveEditViewPrimitive.type:
					return VectorHitTest.hit(primitive.children, position, range, {
						...walk,
						editView: true,
					});

				case PolygonHairlinePrimitive.type: {
					if (walk.textOnly) return undefined;
					const line = VectorHitTest.pathOf(fields.path);
					return VectorHitTest.result(
						'line',
						walk,
						VectorHitTest.isAlongStrokedPath(line, position, 0),
						line,
					);
				}

				case PolygonStrokePrimitive.type:
				case PolyPolygonStrokePrimitive.type:
				case 'polygonStrokeArrow': {
					if (walk.textOnly) return undefined;
					// A line is hit within half its width, which a hairline does not have.
					const stroke = VectorHitTest.pathOf(fields.path);
					return VectorHitTest.result(
						'line',
						walk,
						VectorHitTest.isAlongStrokedPath(
							stroke,
							position,
							fields.line?.width ?? 0,
							fields.line,
						),
						stroke,
					);
				}

				case SingleLinePrimitive.type: {
					if (walk.textOnly) return undefined;
					const line = fields as SingleLinePrimitive;
					const path = new Path2D();
					path.moveTo(line.startX ?? 0, line.startY ?? 0);
					path.lineTo(line.endX ?? 0, line.endY ?? 0);
					return VectorHitTest.result(
						'line',
						walk,
						VectorHitTest.isAlongStrokedPath(path, position, 0),
						path,
					);
				}

				case LineRectanglePrimitive.type: {
					if (walk.textOnly) return undefined;
					// An outlined rectangle is hit along its outline, not inside it.
					const outline = VectorHitTest.pathOfBounds(fields.bounds);
					return VectorHitTest.result(
						'line',
						walk,
						VectorHitTest.isAlongStrokedPath(outline, position, 0),
						outline,
					);
				}

				case PolyPolygonColorPrimitive.type:
				case PolyPolygonRGBAPrimitive.type:
				case 'polyPolygonGradient':
				case 'polyPolygonAlphaGradient':
				case 'polyPolygonHatch':
				case 'polyPolygonGraphic':
				case 'patternFill': {
					if (walk.textOnly) return undefined;
					const fill = VectorHitTest.pathOf(fields.path);
					return VectorHitTest.fillResult(
						walk,
						VectorHitTest.hitFill(fill, position),
						fill,
					);
				}

				case FilledRectanglePrimitive.type: {
					if (walk.textOnly) return undefined;
					const area = VectorHitTest.pathOfBounds(fields.bounds);
					return VectorHitTest.fillResult(
						walk,
						VectorHitTest.hitFill(area, position),
						area,
					);
				}

				case 'fillGradient':
				case 'fillHatch':
				case 'fillGraphic': {
					if (walk.textOnly) return undefined;
					const covered =
						VectorHitTest.pathOfBounds(fields.outputRange) ??
						VectorHitTest.pathOfUnitSquare(fields.matrix ?? []);
					return VectorHitTest.fillResult(
						walk,
						VectorHitTest.hitFill(covered, position),
						covered,
					);
				}

				case BackgroundColorPrimitive.type: {
					if (walk.textOnly) return undefined;
					// The background covers the whole object it belongs to.
					const whole = VectorHitTest.pathOfRange(range);
					return VectorHitTest.fillResult(
						walk,
						VectorHitTest.hitFill(whole, position),
						whole,
					);
				}

				case BitmapPrimitive.type:
				case BitmapAlphaPrimitive.type:
				case GraphicPrimitive.type: {
					/*
						An image is hit anywhere inside the area it covers, see-through pixels
						included, as the engine does while it serves a client.
					*/
					if (fields.matrix) {
						if (walk.textOnly) return undefined;

						const image = VectorHitTest.pathOfUnitSquare(fields.matrix);
						return VectorHitTest.result(
							'bitmap',
							walk,
							VectorHitTest.hitFill(image, position) !== undefined,
							image,
						);
					}

					// A vector graphic paints through its children instead.
					return VectorHitTest.hit(primitive.children, position, range, walk);
				}

				case PointArrayPrimitive.type:
					if (walk.textOnly) return undefined;
					return VectorHitTest.result(
						'point',
						walk,
						VectorHitTest.isAtPoint(primitive as PointArrayPrimitive, position),
					);

				case 'shadow':
					// A shadow cannot be hit.
					return undefined;

				default:
					/*
						Everything else is painted by what it holds: the containers, the text
						hierarchy, and the groups that only change how their children look.
					*/
					return VectorHitTest.hit(primitive.children, position, range, walk);
			}
		}

		private static result(
			kind: HitKind,
			walk: HitWalk,
			hit: boolean,
			path?: Path2D,
		): HitResult | undefined {
			if (!hit) return undefined;

			return {
				kind: kind,
				hidden: walk.hidden,
				editView: walk.editView,
				edge: false,
				path: VectorHitTest.pathThrough(path, walk.matrix),
			};
		}

		private static fillResult(
			walk: HitWalk,
			where: 'inside' | 'edge' | undefined,
			path?: Path2D,
		): HitResult | undefined {
			if (!where) return undefined;

			return {
				kind: 'fill',
				hidden: walk.hidden,
				editView: walk.editView,
				edge: where === 'edge',
				path: VectorHitTest.pathThrough(path, walk.matrix),
			};
		}

		/// A walk that asks about everything, in the coordinates the primitives are written in.
		private static wholeOf(): HitWalk {
			return {
				textOnly: false,
				hidden: false,
				editView: false,
				matrix: [1, 0, 0, 1, 0, 0],
			};
		}

		/// The two matrices as one: what maps a child onto what the walk was asked in.
		private static matrixThrough(
			parent: number[],
			child: number[] | undefined,
		): number[] {
			if (!child || child.length < 6) return parent;

			const [a, b, c, d, e, f] = parent;
			const [g, h, i, j, k, l] = child;

			return [
				a * g + c * h,
				b * g + d * h,
				a * i + c * j,
				b * i + d * j,
				a * k + c * l + e,
				b * k + d * l + f,
			];
		}

		/// The path in the coordinates the walk was asked in.
		private static pathThrough(
			path: Path2D | undefined,
			matrix: number[],
		): Path2D | undefined {
			if (!path) return undefined;

			const [a, b, c, d, e, f] = matrix;
			const mapped = new Path2D();
			mapped.addPath(path, { a: a, b: b, c: c, d: d, e: e, f: f });
			return mapped;
		}

		/// The position and the tolerance in the coordinates a child of the matrix is written in.
		private static positionThrough(
			position: HitPosition,
			matrix: number[] | undefined,
		): HitPosition {
			if (!matrix || matrix.length < 6) return position;

			const [a, b, c, d, e, f] = matrix;
			const determinant = a * d - b * c;
			if (determinant === 0) return position;

			// The point goes the other way through the matrix, which is cheaper than putting the
			// geometry of every child through it.
			const x = position.x - e;
			const y = position.y - f;

			return {
				x: (d * x - c * y) / determinant,
				y: (a * y - b * x) / determinant,
				toleranceX: Math.abs(
					(d * position.toleranceX - c * position.toleranceY) / determinant,
				),
				toleranceY: Math.abs(
					(a * position.toleranceY - b * position.toleranceX) / determinant,
				),
			};
		}

		private static tolerance(position: HitPosition): number {
			return Math.max(position.toleranceX, position.toleranceY);
		}

		private static pathOfBounds(
			bounds: number[] | undefined,
		): Path2D | undefined {
			if (!bounds || bounds.length < 4) return undefined;

			const path = new Path2D();
			path.rect(
				bounds[0],
				bounds[1],
				bounds[2] - bounds[0],
				bounds[3] - bounds[1],
			);
			return path;
		}

		private static pathOfRange(
			range: HitRange | undefined,
		): Path2D | undefined {
			if (!range) return undefined;

			return VectorHitTest.pathOfBounds([
				range.minX,
				range.minY,
				range.maxX,
				range.maxY,
			]);
		}

		/** The four corners of the unit square put through the matrix.
		 *
		 * It is the area an image covers, and the shape of an object as its own transformation
		 * describes it.
		 */
		public static pathOfUnitSquare(matrix: number[]): Path2D | undefined {
			if (matrix.length < 6) return undefined;

			const [a, b, c, d, e, f] = matrix;
			const path = new Path2D();
			path.moveTo(e, f);
			path.lineTo(a + e, b + f);
			path.lineTo(a + c + e, b + d + f);
			path.lineTo(c + e, d + f);
			path.closePath();
			return path;
		}

		/// Where the point lies in a filled path: inside it, within the tolerance of its
		/// boundary, or neither. The engine fills with the even-odd rule, so a subpath inside
		/// another subpath is a hole there as well.
		private static hitFill(
			path: Path2D | undefined,
			position: HitPosition,
		): 'inside' | 'edge' | undefined {
			const context = VectorHitTest.context();
			if (!path || !context) return undefined;

			if (context.isPointInPath(path, position.x, position.y, 'evenodd'))
				return 'inside';

			return VectorHitTest.isAlongStrokedPath(path, position, 0)
				? 'edge'
				: undefined;
		}

		private static pathOf(path: string | undefined): Path2D | undefined {
			return path ? new Path2D(path) : undefined;
		}

		/// Within half the line width of the path, plus the tolerance. The joins and caps are the
		/// ones the line is drawn with, so a mitered corner reaches as far as it is painted.
		private static isAlongStrokedPath(
			path: Path2D | undefined,
			position: HitPosition,
			width: number,
			line?: { linejoin?: string; linecap?: string },
		): boolean {
			const context = VectorHitTest.context();
			if (!path || !context) return false;

			const reach = width + 2 * VectorHitTest.tolerance(position);
			if (!(reach > 0)) return false;

			context.save();
			context.lineWidth = reach;
			context.lineJoin =
				line?.linejoin === 'round' || line?.linejoin === 'bevel'
					? line.linejoin
					: 'miter';
			context.lineCap =
				line?.linecap === 'round' || line?.linecap === 'square'
					? line.linecap
					: 'butt';
			const hit = context.isPointInStroke(path, position.x, position.y);
			context.restore();

			return hit;
		}

		/// A text is hit inside the area its glyphs occupy, measured for the font the portion
		/// names, the way the engine uses the area a text portion covers.
		private static isInText(
			primitive: TextSimplePortionPrimitive,
			position: HitPosition,
		): boolean {
			const context = VectorHitTest.context();
			if (!context || !primitive.text) return false;

			const start = primitive.textPosition ?? 0;
			const length = primitive.textLength ?? primitive.text.length - start;
			const text = primitive.text.substring(start, start + length);
			if (!text) return false;

			const fontSize = primitive.fontSize ?? 0;
			if (!(fontSize > 0)) return false;

			const [a = fontSize, b = 0, c = 0, d = fontSize, e = 0, f = 0] =
				primitive.matrix ?? [];

			/*
				The matrix carries the font size in its scale, so dividing it out leaves what
				rotates, shears and flips the glyphs. The point goes the other way through that,
				which puts it where the glyphs sit on their baseline.
			*/
			const local = VectorHitTest.positionThrough(position, [
				a / fontSize,
				b / fontSize,
				c / fontSize,
				d / fontSize,
				e,
				f,
			]);

			context.font = `${primitive.italic ? 'italic' : 'normal'} ${fontSize}px "${primitive.familyname ?? 'sans-serif'}"`;
			const metrics = context.measureText(text);
			const width = metrics.width;
			// A font that does not report where it reaches above and below the baseline is taken to
			// fill its own size.
			const ascent = metrics.fontBoundingBoxAscent ?? fontSize * 0.8;
			const descent = metrics.fontBoundingBoxDescent ?? fontSize * 0.2;

			/*
				The box the glyphs occupy, reached from as far as the tolerance says on each axis.
				A reader that asks about a whole line of text gives a wide reach along the line and
				none across it, so a point past the end of a short line still counts as that line
				and a point above it does not.
			*/
			const box = VectorHitTest.pathOfBounds([
				-local.toleranceX,
				-ascent - local.toleranceY,
				width + local.toleranceX,
				descent + local.toleranceY,
			]);

			return (
				VectorHitTest.hitFill(box, {
					x: local.x,
					y: local.y,
					toleranceX: 0,
					toleranceY: 0,
				}) !== undefined
			);
		}

		private static isAtPoint(
			primitive: PointArrayPrimitive,
			position: HitPosition,
		): boolean {
			if (!primitive.points) return false;

			const reach = VectorHitTest.tolerance(position);

			return primitive.points.some(
				(point: { x: number; y: number }) =>
					Math.hypot(point.x - position.x, point.y - position.y) <= reach,
			);
		}
	}

	/// The fields the tests read, which the types of primitive carry in different combinations.
	interface HitFields extends Primitive {
		matrix?: number[];
		path?: string;
		clip?: string;
		bounds?: number[];
		outputRange?: number[];
		line?: {
			width?: number;
			linejoin?: string;
			linecap?: string;
		};
	}
}
