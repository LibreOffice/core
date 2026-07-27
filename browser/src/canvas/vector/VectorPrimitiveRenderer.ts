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
	/// A function that resolves a bitmap checksum to the decoded image,
	/// or undefined when the image is not in the cache yet.
	export type BitmapLookup = (checksum: number) => HTMLImageElement | undefined;

	/// Renders JSON primitives with Canvas 2D drawing operations.
	export class VectorPrimitiveRenderer {
		private _bitmapLookup: BitmapLookup | undefined;
		// Slide size in twips. The background primitive fills this rectangle.
		private _slideWidth = 0;
		private _slideHeight = 0;

		constructor(bitmapLookup?: BitmapLookup) {
			this._bitmapLookup = bitmapLookup;
		}

		/// Set the slide size in twips before rendering a primitive tree, so the
		/// background fills the slide and leaves the area around it untouched.
		setSlideBounds(width: number, height: number): void {
			this._slideWidth = width;
			this._slideHeight = height;
		}

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
				case PolyPolygonRGBAPrimitive.type:
					this._renderPolyPolygonColor(
						context,
						primitive as PolyPolygonRGBAPrimitive,
					);
					break;
				case PolygonStrokePrimitive.type:
				case PolyPolygonStrokePrimitive.type:
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
				case LineRectanglePrimitive.type:
					this._renderLineRectangle(
						context,
						primitive as LineRectanglePrimitive,
					);
					break;
				case SingleLinePrimitive.type:
					this._renderSingleLine(context, primitive as SingleLinePrimitive);
					break;
				case PointArrayPrimitive.type:
					this._renderPointArray(context, primitive as PointArrayPrimitive);
					break;
				case BitmapPrimitive.type:
					this._renderBitmap(context, primitive as BitmapPrimitive);
					break;
				case GraphicPrimitive.type:
					this._renderGraphic(context, primitive as GraphicPrimitive);
					break;
				case TextSimplePortionPrimitive.type:
					this._renderTextSimplePortion(
						context,
						primitive as TextSimplePortionPrimitive,
					);
					break;
				case TextDecoratedPortionPrimitive.type:
					this._renderTextDecoratedPortion(
						context,
						primitive as TextDecoratedPortionPrimitive,
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
				case MaskPrimitive.type:
					this._renderMask(context, primitive as MaskPrimitive);
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

			// Fill only the slide rectangle, in the current twip transform, so
			// the workspace color shows around the slide.
			context.save();
			context.globalAlpha = 1 - (primitive.transparency ?? 0);
			context.fillStyle = primitive.color;
			context.fillRect(0, 0, this._slideWidth, this._slideHeight);
			context.restore();
		}

		private _renderPolyPolygonColor(
			context: CanvasRenderingContext2D,
			primitive: PolyPolygonColorPrimitive | PolyPolygonRGBAPrimitive,
		): void {
			if (!primitive.path || !primitive.color) return;

			const path = new Path2D(primitive.path);
			const transparency =
				(primitive as PolyPolygonRGBAPrimitive).transparency ?? 0;
			const needsAlphaBracket = transparency > 0;

			if (needsAlphaBracket) {
				context.save();
				context.globalAlpha = 1 - transparency;
			}
			context.fillStyle = primitive.color;
			// The engine fills polypolygons with the even-odd rule, so
			// a subpath inside another subpath reads as a hole.
			context.fill(path, 'evenodd');
			if (needsAlphaBracket) context.restore();
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

		private _renderLineRectangle(
			context: CanvasRenderingContext2D,
			primitive: LineRectanglePrimitive,
		): void {
			if (!primitive.bounds || primitive.bounds.length < 4) return;

			const [minX, minY, maxX, maxY] = primitive.bounds;
			context.save();
			context.strokeStyle = primitive.color ?? '#000000';
			// LineRectangle is a one-canvas-pixel outline by definition.
			context.lineWidth = 1;
			context.strokeRect(minX, minY, maxX - minX, maxY - minY);
			context.restore();
		}

		private _renderSingleLine(
			context: CanvasRenderingContext2D,
			primitive: SingleLinePrimitive,
		): void {
			const startX = primitive.startX ?? 0;
			const startY = primitive.startY ?? 0;
			const endX = primitive.endX ?? 0;
			const endY = primitive.endY ?? 0;

			context.save();
			context.strokeStyle = primitive.color ?? '#000000';
			// SingleLine renders at one canvas pixel by definition.
			context.lineWidth = 1;
			context.beginPath();
			context.moveTo(startX, startY);
			context.lineTo(endX, endY);
			context.stroke();
			context.restore();
		}

		// Maps the wire font-weight value 0..10 to a CSS font-weight
		// number. The value 0 means unknown and renders as normal.
		private static readonly _FONT_WEIGHT_CSS: Record<number, number> = {
			0: 400,
			1: 100,
			2: 200,
			3: 300,
			4: 350,
			5: 400,
			6: 500,
			7: 600,
			8: 700,
			9: 800,
			10: 900,
		};

		// Sets up the canvas font and (when needed) the rotation
		// transform for a text primitive, and returns the drawing
		// anchor and the substring to paint. Callers must save and
		// restore the context around the call. Returns null if there
		// is nothing to draw.
		private _setupTextFrame(
			context: CanvasRenderingContext2D,
			primitive: TextSimplePortionPrimitive | TextDecoratedPortionPrimitive,
		): { x: number; y: number; fontSize: number; text: string } | null {
			if (!primitive.text) return null;
			const start = primitive.textPosition ?? 0;
			const length = primitive.textLength ?? primitive.text.length - start;
			const text = primitive.text.substring(start, start + length);
			if (!text) return null;

			const fontSize = primitive.fontSize ?? 12;
			if (!(fontSize > 0)) return null;
			// A missing matrix defaults to the plain upright scale.
			const [a = fontSize, b = 0, c = 0, d = fontSize, e = 0, f = 0] =
				primitive.matrix ?? [];
			const style = primitive.italic ? 'italic' : 'normal';
			const weight =
				VectorPrimitiveRenderer._FONT_WEIGHT_CSS[primitive.weight ?? 5] ?? 400;
			const family = primitive.familyname ?? 'sans-serif';

			context.font = `${style} ${weight} ${fontSize}px "${family}"`;
			// A matrix that only scales by the font size moves the
			// anchor and nothing else, so it skips the transform.
			const transformed =
				b !== 0 || c !== 0 || a !== fontSize || d !== fontSize;
			if (transformed) {
				// The font size is the matrix's scale, so dividing it
				// out leaves a unit transform that rotates, shears,
				// flips or stretches the glyphs drawn at fontSize px.
				context.transform(
					a / fontSize,
					b / fontSize,
					c / fontSize,
					d / fontSize,
					e,
					f,
				);
			}
			return {
				x: transformed ? 0 : e,
				y: transformed ? 0 : f,
				fontSize,
				text,
			};
		}

		private _renderTextSimplePortion(
			context: CanvasRenderingContext2D,
			primitive: TextSimplePortionPrimitive | TextDecoratedPortionPrimitive,
		): void {
			context.save();
			const frame = this._setupTextFrame(context, primitive);
			if (frame) {
				context.fillStyle = primitive.fontcolor ?? '#000000';
				context.fillText(frame.text, frame.x, frame.y);

				if (primitive.outline) {
					context.strokeStyle = primitive.fontcolor ?? '#000000';
					// Outline thickness scales with the font size. A one-
					// pixel stroke becomes invisible on large text.
					context.lineWidth = Math.max(1, frame.fontSize / 20);
					context.strokeText(frame.text, frame.x, frame.y);
				}
			}
			context.restore();
		}

		private _renderTextDecoratedPortion(
			context: CanvasRenderingContext2D,
			primitive: TextDecoratedPortionPrimitive,
		): void {
			// Paint the text body first. The decorations draw on
			// top below.
			this._renderTextSimplePortion(context, primitive);

			if (!primitive.underline && !primitive.overline && !primitive.strikeout)
				return;

			context.save();
			const frame = this._setupTextFrame(context, primitive);
			if (frame) {
				const width = context.measureText(frame.text).width;
				const lineWidth = Math.max(1, frame.fontSize / 20);
				const textColor = primitive.fontcolor ?? '#000000';

				const drawLine = (lineY: number, color: string): void => {
					context.strokeStyle = color;
					context.lineWidth = lineWidth;
					context.beginPath();
					context.moveTo(frame.x, lineY);
					context.lineTo(frame.x + width, lineY);
					context.stroke();
				};

				if (primitive.underline) {
					const lineY = primitive.underlineAbove
						? frame.y - frame.fontSize * 0.85
						: frame.y + frame.fontSize * 0.15;
					drawLine(lineY, primitive.underlineColor ?? textColor);
				}
				if (primitive.overline)
					drawLine(
						frame.y - frame.fontSize * 0.85,
						primitive.overlineColor ?? textColor,
					);
				if (primitive.strikeout)
					drawLine(frame.y - frame.fontSize * 0.3, textColor);
			}
			context.restore();
		}

		private _renderBitmap(
			context: CanvasRenderingContext2D,
			primitive: BitmapPrimitive,
		): void {
			this._drawRaster(context, primitive.matrix, primitive.checksum);
		}

		private _renderGraphic(
			context: CanvasRenderingContext2D,
			primitive: GraphicPrimitive,
		): void {
			// Vector graphics arrive pre-decomposed. The children are
			// already in slide coordinates, so let the post-switch
			// recursion in renderPrimitive draw them.
			if (primitive.vector) return;
			this._drawRaster(
				context,
				primitive.matrix,
				primitive.checksum,
				primitive.crop,
				primitive.rotation,
				primitive.alpha,
				primitive.mirror,
				primitive.drawMode,
			);
		}

		// Resolve the image through the checksum lookup and draw it
		// into the unit square mapped by the wire matrix. Skips when
		// the matrix is missing, no lookup is registered, the lookup
		// has no entry yet, or the image is still decoding.
		//
		// When crop is set, the source rectangle is the corresponding
		// inset region of the image in pixels, and the destination is
		// the same inset region of the unit square. The matrix was
		// sized for the original image, so insetting both rectangles
		// keeps the visible content aligned with the slide bounds.
		//
		// rotation is in tenths of a degree and rotates around the
		// centre of the unit square, which the matrix maps to the
		// centre of the image in slide space.
		//
		// alpha is a byte where 255 is opaque. Values below 255
		// translate to a globalAlpha that scopes to the save and
		// restore around the draw.
		//
		// mirror is a bitfield: bit 0 flips horizontally, bit 1
		// flips vertically. The flip is scoped to the unit square,
		// so the cropped or rotated content lands in the same place
		// but with the chosen axes reflected.
		//
		// drawMode picks a CSS filter that recolours the image:
		// greys desaturates, mono is one-bit black-and-white,
		// watermark is a faded grey overlay.
		private _drawRaster(
			context: CanvasRenderingContext2D,
			matrix: number[] | undefined,
			checksum: number,
			crop?: GraphicPrimitive['crop'],
			rotation?: number,
			alpha?: number,
			mirror?: number,
			drawMode?: GraphicPrimitive['drawMode'],
		): void {
			if (!matrix || matrix.length < 6) return;
			if (!this._bitmapLookup) return;

			const image = this._bitmapLookup(checksum);
			if (!image) return;
			if (!image.complete) return;

			context.save();
			if (typeof alpha === 'number' && alpha < 255)
				context.globalAlpha = alpha / 255;
			// A drawMode recolour is the innermost colour modifier
			// around its graphic, so its filter leads the list.
			if (drawMode === 'greys')
				context.filter = this._composeFilter(context, 'grayscale(1)');
			else if (drawMode === 'mono')
				context.filter = this._composeFilter(
					context,
					'grayscale(1) contrast(1000%)',
				);
			else if (drawMode === 'watermark')
				context.filter = this._composeFilter(
					context,
					'grayscale(1) brightness(1.5) opacity(0.3)',
				);
			// The matrix maps the unit square to the image's bounds,
			// so we draw the image into the unit square and let the
			// transform place it on the slide.
			context.transform(
				matrix[0],
				matrix[1],
				matrix[2],
				matrix[3],
				matrix[4],
				matrix[5],
			);

			if (mirror) {
				// Translate to the far edge of each flipped axis so
				// the scaled image still fills the unit square.
				const mx = mirror & 1 ? -1 : 1;
				const my = mirror & 2 ? -1 : 1;
				context.translate(mx < 0 ? 1 : 0, my < 0 ? 1 : 0);
				context.scale(mx, my);
			}

			if (rotation) {
				const radians = ((rotation / 10) * Math.PI) / 180;
				context.translate(0.5, 0.5);
				context.rotate(radians);
				context.translate(-0.5, -0.5);
			}

			if (crop && (crop.left || crop.top || crop.right || crop.bottom)) {
				// The crop distances and the matrix's column-vector
				// frame size share slide units, so dividing gives
				// fractions of the frame.
				const frameW = Math.hypot(matrix[0], matrix[1]);
				const frameH = Math.hypot(matrix[2], matrix[3]);
				const left = (crop.left ?? 0) / frameW;
				const top = (crop.top ?? 0) / frameH;
				const right = (crop.right ?? 0) / frameW;
				const bottom = (crop.bottom ?? 0) / frameH;
				// The uncropped image spans -left to 1 + right and -top
				// to 1 + bottom in the frame's unit square, which shows
				// the part inside it. Draw the overlap, mapped back to
				// source pixels, since drawImage needs a source
				// rectangle inside the image.
				const x0 = Math.max(0, -left);
				const y0 = Math.max(0, -top);
				const x1 = Math.min(1, 1 + right);
				const y1 = Math.min(1, 1 + bottom);
				if (x0 < x1 && y0 < y1) {
					const unitW = 1 + left + right;
					const unitH = 1 + top + bottom;
					const sx = Math.round(((x0 + left) / unitW) * image.naturalWidth);
					const sy = Math.round(((y0 + top) / unitH) * image.naturalHeight);
					const sw = Math.round(((x1 - x0) / unitW) * image.naturalWidth);
					const sh = Math.round(((y1 - y0) / unitH) * image.naturalHeight);
					if (sw > 0 && sh > 0) {
						context.drawImage(image, sx, sy, sw, sh, x0, y0, x1 - x0, y1 - y0);
					}
				}
			} else {
				context.drawImage(image, 0, 0, 1, 1);
			}
			context.restore();
		}

		private _renderPointArray(
			context: CanvasRenderingContext2D,
			primitive: PointArrayPrimitive,
		): void {
			if (!primitive.points?.length) return;

			context.save();
			context.fillStyle = primitive.color ?? '#000000';
			for (const point of primitive.points)
				context.fillRect(point.x, point.y, 1, 1);
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
				context.filter = this._composeFilter(context, filter);
				this._renderPrimitives(context, primitive.children);
				context.restore();
			} else {
				this._renderPrimitives(context, primitive.children);
			}
		}

		/// Prepend a filter to the context's active filter list. The
		/// engine applies the innermost colour modifier first and
		/// canvas filters run left to right, so the new filter leads.
		private _composeFilter(
			context: CanvasRenderingContext2D,
			filter: string,
		): string {
			const active = context.filter;
			return active && active !== 'none' ? filter + ' ' + active : filter;
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

		private _renderMask(
			context: CanvasRenderingContext2D,
			primitive: MaskPrimitive,
		): void {
			context.save();
			if (primitive.clip) context.clip(new Path2D(primitive.clip), 'evenodd');
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
