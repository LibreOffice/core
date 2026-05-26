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
	/// Renders JSON primitives with Canvas 2D drawing operations.
	export class VectorPrimitiveRenderer {
		renderPrimitive(
			context: CanvasRenderingContext2D,
			primitive: Primitive,
		): void {
			if (!primitive || !primitive.type) return;

			switch (primitive.type) {
				case BackgroundColorPrimitive.type:
					this._renderBackgroundColor(
						context,
						primitive as BackgroundColorPrimitive,
					);
					break;
				case PolyPolygonColorPrimitive.type:
					this._renderPolyPolygonColor(
						context,
						primitive as PolyPolygonColorPrimitive,
					);
					break;
				case PolygonStrokePrimitive.type:
					this._renderPolygonStroke(
						context,
						primitive as PolygonStrokePrimitive,
					);
					break;
				case PolygonHairlinePrimitive.type:
					this._renderPolygonHairline(
						context,
						primitive as PolygonHairlinePrimitive,
					);
					break;
				case FilledRectanglePrimitive.type:
					this._renderFilledRectangle(
						context,
						primitive as FilledRectanglePrimitive,
					);
					break;
				case GroupPrimitive.type:
				case ObjectInfoPrimitive.type:
					// Pure container - recursion into children happens
					// below for every primitive type.
					break;
				case TransformPrimitive.type:
					// Children must be drawn inside the transformed
					// coordinates. The helper recurses on its own, so
					// return here to skip the recursion at the end of
					// renderPrimitive.
					this._renderTransform(context, primitive as TransformPrimitive);
					return;
				case UnifiedTransparencePrimitive.type:
					// The helper sets globalAlpha and recurses on its
					// own, so return here to skip the recursion at the
					// end of renderPrimitive.
					this._renderUnifiedTransparence(
						context,
						primitive as UnifiedTransparencePrimitive,
					);
					return;
				case ModifiedColorPrimitive.type:
					this._renderModifiedColor(
						context,
						primitive as ModifiedColorPrimitive,
					);
					return;
				case HiddenGeometryPrimitive.type:
				case ExclusiveEditViewPrimitive.type:
					// Non-painting subtree. Return so the recursion
					// at the end of renderPrimitive does not descend
					// into the children.
					return;
			}

			if (primitive.children)
				this._renderPrimitives(context, primitive.children);
		}

		private _renderBackgroundColor(
			context: CanvasRenderingContext2D,
			primitive: BackgroundColorPrimitive,
		): void {
			if (!primitive.color) return;

			// Fill the entire canvas = the whole drawing area.
			context.save();
			context.resetTransform();
			context.globalAlpha = 1 - (primitive.transparency ?? 0);
			context.fillStyle = primitive.color;
			context.fillRect(0, 0, context.canvas.width, context.canvas.height);
			context.restore();
		}

		private _renderPolyPolygonColor(
			context: CanvasRenderingContext2D,
			primitive: PolyPolygonColorPrimitive,
		): void {
			if (!primitive.path || !primitive.color) return;

			const path = new Path2D(primitive.path);
			context.fillStyle = primitive.color;
			context.fill(path);
		}

		private _renderPolygonStroke(
			context: CanvasRenderingContext2D,
			primitive: PolygonStrokePrimitive,
		): void {
			if (!primitive.path) return;

			const line = primitive.line ?? {};
			const path = new Path2D(primitive.path);

			context.save();
			context.strokeStyle = line.color ?? '#000000';
			// A zero-width stroke is invisible on canvas, so clamp to one canvas pixel.
			context.lineWidth = Math.max(line.width ?? 0, 1);
			// The wire format can carry "none" and "unknown" for linejoin
			// and linecap. Canvas has no matching option, so fall back
			// to "miter" and "butt".
			context.lineJoin =
				line.linejoin === 'round' || line.linejoin === 'bevel'
					? line.linejoin
					: 'miter';
			context.lineCap =
				line.linecap === 'round' || line.linecap === 'square'
					? line.linecap
					: 'butt';
			if (primitive.stroke?.dotDashArray?.length)
				context.setLineDash(primitive.stroke.dotDashArray);
			context.stroke(path);
			context.restore();
		}

		private _renderPolygonHairline(
			context: CanvasRenderingContext2D,
			primitive: PolygonHairlinePrimitive,
		): void {
			if (!primitive.path) return;

			const path = new Path2D(primitive.path);
			context.save();
			context.strokeStyle = primitive.color ?? '#000000';
			// Hairlines render at one canvas pixel by definition.
			context.lineWidth = 1;
			context.stroke(path);
			context.restore();
		}

		private _renderFilledRectangle(
			context: CanvasRenderingContext2D,
			primitive: FilledRectanglePrimitive,
		): void {
			if (!primitive.bounds || primitive.bounds.length < 4) return;

			const [minX, minY, maxX, maxY] = primitive.bounds;
			context.save();
			context.fillStyle = primitive.color ?? '#000000';
			context.fillRect(minX, minY, maxX - minX, maxY - minY);
			context.restore();
		}

		private _renderTransform(
			context: CanvasRenderingContext2D,
			primitive: TransformPrimitive,
		): void {
			const matrix = primitive.matrix;
			const hasMatrix = matrix !== undefined && matrix.length >= 6;

			if (hasMatrix) {
				context.save();
				context.transform(
					matrix[0],
					matrix[1],
					matrix[2],
					matrix[3],
					matrix[4],
					matrix[5],
				);
			}

			if (primitive.children)
				this._renderPrimitives(context, primitive.children);

			if (hasMatrix) context.restore();
		}

		private _renderModifiedColor(
			context: CanvasRenderingContext2D,
			primitive: ModifiedColorPrimitive,
		): void {
			if (!primitive.children) return;

			let filter: string | null = null;
			switch (primitive.modifier) {
				case 'gray':
				case 'luminance_to_alpha':
					filter = 'grayscale(1)';
					break;
				case 'invert':
					filter = 'invert(1)';
					break;
				// The "replace" modifier needs every primitive that
				// draws a colour to consult a shared colour override.
				// That is left for a follow-up, so for now children
				// render with their original colours.
			}

			if (filter !== null) {
				context.save();
				context.filter = filter;
				this._renderPrimitives(context, primitive.children);
				context.restore();
			} else {
				this._renderPrimitives(context, primitive.children);
			}
		}

		private _renderUnifiedTransparence(
			context: CanvasRenderingContext2D,
			primitive: UnifiedTransparencePrimitive,
		): void {
			context.save();
			// Canvas globalAlpha uses the opposite convention to the
			// wire transparence: 1 is opaque on the canvas, 0 on the
			// wire. Invert the value to translate.
			context.globalAlpha = 1 - (primitive.transparence ?? 0);
			if (primitive.children)
				this._renderPrimitives(context, primitive.children);
			context.restore();
		}

		private _renderPrimitives(
			context: CanvasRenderingContext2D,
			primitives: Primitive[],
		): void {
			for (const primitive of primitives) {
				this.renderPrimitive(context, primitive);
			}
		}
	}
}
