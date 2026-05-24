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
			// A zero-width stroke is invisible on canvas, so clamp to one device pixel.
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
