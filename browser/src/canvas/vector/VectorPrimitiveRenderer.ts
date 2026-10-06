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

	/// A function that tells whether the font with the given id has been
	/// loaded and registered, so it can be used for drawing.
	export type FontLoadedLookup = (fontId: string) => boolean;

	/// Renders JSON primitives with Canvas 2D drawing operations.
	export class VectorPrimitiveRenderer {
		private _bitmapLookup: BitmapLookup | undefined;
		private _fontLoaded: FontLoadedLookup | undefined;
		// Slide size in twips. The background primitive fills this rectangle.
		private _slideWidth = 0;
		private _slideHeight = 0;
		private _scratch: VectorScratchCanvases;
		private _gradients: VectorGradientPrimitiveRenderer;
		private _fillGraphics: VectorFillGraphicPrimitiveRenderer;
		private _colorFilters = new VectorColorFilters();
		private _modifiedImages = new VectorModifiedImages();
		// Whether the content an editing view alone shows is drawn. Every
		// entry point sets it from what the render is for, so it never
		// carries over from an earlier render.
		private _editViewContentVisible = false;

		constructor(bitmapLookup?: BitmapLookup, fontLoaded?: FontLoadedLookup) {
			this._bitmapLookup = bitmapLookup;
			this._fontLoaded = fontLoaded;
			this._scratch = new VectorScratchCanvases();
			this._gradients = new VectorGradientPrimitiveRenderer(this._scratch);
			this._fillGraphics = new VectorFillGraphicPrimitiveRenderer(bitmapLookup);
		}

		/// Set the slide size in twips before rendering a primitive tree, so the
		/// background fills the slide and leaves the area around it untouched.
		setSlideBounds(width: number, height: number): void {
			this._slideWidth = width;
			this._slideHeight = height;
		}

		/// Show or hide the content only an editing view shows: the prompt
		/// of an empty placeholder and the stand-in graphic of an empty
		/// picture. A thumbnail and a slideshow leave it out.
		setEditViewContentVisible(visible: boolean): void {
			this._editViewContentVisible = visible;
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
				case FillGradientPrimitive.type:
					this._gradients.renderFillGradient(
						context,
						primitive as FillGradientPrimitive,
					);
					break;
				case PolyPolygonGradientPrimitive.type:
					this._gradients.renderPolyPolygonGradient(
						context,
						primitive as PolyPolygonGradientPrimitive,
					);
					break;
				case PolyPolygonAlphaGradientPrimitive.type:
					this._gradients.renderPolyPolygonAlphaGradient(
						context,
						primitive as PolyPolygonAlphaGradientPrimitive,
					);
					break;
				case FillHatchPrimitive.type:
				case PolyPolygonHatchPrimitive.type:
					this._renderHatch(context, primitive as FillHatchPrimitive);
					break;
				case FillGraphicPrimitive.type:
					this._fillGraphics.renderFillGraphic(
						context,
						primitive as FillGraphicPrimitive,
					);
					break;
				case PolyPolygonGraphicPrimitive.type:
					this._fillGraphics.renderPolyPolygonGraphic(
						context,
						primitive as PolyPolygonGraphicPrimitive,
					);
					break;
				case BitmapPrimitive.type:
					this._renderBitmap(context, primitive as BitmapPrimitive);
					break;
				case BitmapAlphaPrimitive.type:
					this._renderBitmapAlpha(context, primitive as BitmapAlphaPrimitive);
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
				case PolygonStrokeArrowPrimitive.type:
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
				case PatternFillPrimitive.type:
					// The children are one tile, and the pattern fill draws
					// them once per tile itself.
					this._renderPatternFill(context, primitive as PatternFillPrimitive);
					return;
				case TransparencePrimitive.type:
					this._renderTransparence(context, primitive as TransparencePrimitive);
					return;
				case ShadowPrimitive.type:
					this._renderShadow(context, primitive as ShadowPrimitive);
					return;
				case GlowPrimitive.type:
					this._renderGlow(context, primitive as GlowPrimitive);
					return;
				case SoftEdgePrimitive.type:
					this._renderSoftEdge(context, primitive as SoftEdgePrimitive);
					return;
				case MaskPrimitive.type:
					this._renderMask(context, primitive as MaskPrimitive);
					return;
				case HiddenGeometryPrimitive.type:
					// Non-painting subtree. Return so the recursion
					// at the end of renderPrimitive does not descend
					// into the children.
					return;
				case ExclusiveEditViewPrimitive.type:
					// Wraps what only an editing view shows, so the
					// recursion below descends into it only there.
					if (!this._editViewContentVisible) return;
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

		/// Width that draws as one device pixel under the current transform.
		/// The context is in twips, so the scale has to be divided out.
		private _hairlineWidth(context: CanvasRenderingContext2D): number {
			if (typeof context.getTransform !== 'function') return 1;
			const matrix = context.getTransform();
			const scale = Math.hypot(matrix.a, matrix.b);
			return scale > 0 ? 1 / scale : 1;
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
			// Width 0 means a hairline. Any other width is drawn as given,
			// even when that is thinner than a device pixel.
			const width = line.width ?? 0;
			context.lineWidth = width === 0 ? this._hairlineWidth(context) : width;
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
			context.lineWidth = this._hairlineWidth(context);
			context.stroke(path);
			context.restore();
		}

		private _renderFilledRectangle(
			context: CanvasRenderingContext2D,
			primitive: FilledRectanglePrimitive,
		): void {
			const bounds = Range2D.fromArray(primitive.bounds);
			if (!bounds) return;

			context.save();
			context.fillStyle = primitive.color ?? '#000000';
			context.fillRect(bounds.minX, bounds.minY, bounds.width, bounds.height);
			context.restore();
		}

		private _renderLineRectangle(
			context: CanvasRenderingContext2D,
			primitive: LineRectanglePrimitive,
		): void {
			const bounds = Range2D.fromArray(primitive.bounds);
			if (!bounds) return;

			context.save();
			context.strokeStyle = primitive.color ?? '#000000';
			context.lineWidth = this._hairlineWidth(context);
			context.strokeRect(bounds.minX, bounds.minY, bounds.width, bounds.height);
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
			context.lineWidth = this._hairlineWidth(context);
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
			const matrix =
				Matrix2D.fromArray(primitive.matrix) ??
				new Matrix2D(fontSize, 0, 0, fontSize, 0, 0);
			let style = primitive.italic ? 'italic' : 'normal';
			let weight =
				VectorPrimitiveRenderer._FONT_WEIGHT_CSS[primitive.weight ?? 5] ?? 400;
			// Prefer the exact face the engine sent, once it has loaded.
			// Fall back to the family name otherwise.
			let family = primitive.familyname ?? 'sans-serif';
			if (
				primitive.fontId !== undefined &&
				this._fontLoaded &&
				this._fontLoaded(primitive.fontId)
			) {
				family = 'vecfont-' + primitive.fontId;
				// The face is already the right cut, so asking again would
				// apply it twice. The engine flags a family that has no cut.
				if (!primitive.syntheticItalic) style = 'normal';
				if (!primitive.syntheticBold) weight = 400;
			}

			context.font = `${style} ${weight} ${fontSize}px "${family}"`;
			// A matrix that only scales by the font size moves the
			// anchor and nothing else, so it skips the transform.
			const transformed =
				matrix.b !== 0 ||
				matrix.c !== 0 ||
				matrix.a !== fontSize ||
				matrix.d !== fontSize;
			if (transformed) {
				// The font size is the matrix's scale, so dividing it
				// out leaves a unit transform that rotates, shears,
				// flips or stretches the glyphs drawn at fontSize px.
				// The scale goes first, so the translation stays as is.
				Matrix2D.IDENTITY.scale(1 / fontSize, 1 / fontSize)
					.then(matrix)
					.applyTo(context);
			}
			return {
				x: transformed ? 0 : matrix.e,
				y: transformed ? 0 : matrix.f,
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

		private _renderBitmapAlpha(
			context: CanvasRenderingContext2D,
			primitive: BitmapAlphaPrimitive,
		): void {
			this._drawRaster(context, primitive.matrix, primitive.checksum, {
				// The wire counts transparency up from 0 for opaque.
				alpha:
					typeof primitive.transparency === 'number'
						? 1 - primitive.transparency
						: undefined,
			});
		}

		private _renderGraphic(
			context: CanvasRenderingContext2D,
			primitive: GraphicPrimitive,
		): void {
			// Vector graphics arrive pre-decomposed. The children are
			// already in slide coordinates, so let the post-switch
			// recursion in renderPrimitive draw them.
			if (primitive.vector) return;
			this._drawRaster(context, primitive.matrix, primitive.checksum, {
				imageRect: primitive.imageRect,
				rotation: primitive.rotation,
				// The wire gives a byte, with 255 for opaque.
				alpha:
					typeof primitive.alpha === 'number'
						? primitive.alpha / 255
						: undefined,
				mirror: primitive.mirror,
				colorModifiers: primitive.colorModifiers,
			});
		}

		// Resolve the image through the checksum lookup and draw it
		// into the unit square mapped by the wire matrix. Skips when
		// the matrix is missing, no lookup is registered, the lookup
		// has no entry yet, or the image is still decoding.
		private _drawRaster(
			context: CanvasRenderingContext2D,
			wireMatrix: number[] | undefined,
			checksum: number,
			options: DrawRasterOptions = {},
		): void {
			const matrix = Matrix2D.fromArray(wireMatrix);
			if (!matrix) return;
			if (!this._bitmapLookup) return;

			const image = this._bitmapLookup(checksum);
			if (!image) return;
			if (!image.complete) return;

			const { imageRect, rotation, alpha, mirror, colorModifiers } = options;

			context.save();
			if (typeof alpha === 'number' && alpha < 1) context.globalAlpha = alpha;
			// The matrix maps the unit square to the image's bounds,
			// so we draw the image into the unit square and let the
			// transform place it on the slide.
			matrix.applyTo(context);

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

			const source = colorModifiers?.length
				? this._colorChangedImage(context, image, colorModifiers, imageRect)
				: image;

			if (imageRect) {
				// The whole image maps onto this rectangle of the
				// unit square; the clip keeps the frame's part.
				context.beginPath();
				context.rect(0, 0, 1, 1);
				context.clip();
				context.drawImage(
					source,
					imageRect.x,
					imageRect.y,
					imageRect.width,
					imageRect.height,
				);
			} else {
				context.drawImage(source, 0, 0, 1, 1);
			}
			context.restore();
		}

		// The image with its colors changed: through an SVG filter where the
		// canvas has filters, else a copy at the drawn size. The engine changes
		// pixels before scaling, so mono or gamma can differ along sharp edges.
		private _colorChangedImage(
			context: CanvasRenderingContext2D,
			image: HTMLImageElement,
			modifiers: ColorModifierAttribute[],
			imageRect: DrawRasterOptions['imageRect'],
		): CanvasImageSource {
			if (VectorColorFilters.canFilter(context)) {
				const filter = this._colorFilters.filterFor(modifiers);
				if (filter) context.filter = this._composeFilter(context, filter);
				return image;
			}

			// The whole image spans the image rectangle of the unit square.
			const matrix = context.getTransform();
			return this._modifiedImages.get(
				image,
				modifiers,
				Math.hypot(matrix.a, matrix.b) * (imageRect?.width ?? 1),
				Math.hypot(matrix.c, matrix.d) * (imageRect?.height ?? 1),
			);
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
			const matrix = Matrix2D.fromArray(primitive.matrix);

			if (matrix) {
				context.save();
				matrix.applyTo(context);
			}

			if (primitive.children)
				this._renderPrimitives(context, primitive.children);

			if (matrix) context.restore();
		}

		private _renderModifiedColor(
			context: CanvasRenderingContext2D,
			primitive: ModifiedColorPrimitive,
		): void {
			const children = primitive.children;
			if (!children) return;

			if (primitive.modifier === 'gray' || primitive.modifier === 'invert') {
				this._renderGrayOrInvert(
					context,
					children,
					primitive.bounds,
					primitive.modifier === 'invert',
				);
				return;
			}

			// No scratch canvas, or no color change: the children draw as is.
			const plain = () => this._renderPrimitives(context, children);
			if (!VectorColorModifiers.changesColors(primitive)) {
				plain();
				return;
			}
			this._scratch.withEffectSlots(plain, (slot) => {
				const region = VectorScratchCanvases.effectRegion(
					context,
					primitive.bounds,
					0,
				);
				if (!region) return;
				const content = this._renderToScratch(context, slot, children, region);
				if (!content) {
					plain();
					return;
				}
				if (
					!VectorPrimitiveRenderer._modifiesByCompositing(primitive) &&
					VectorColorFilters.canFilter(context)
				) {
					// Drawn through the filter, with no pixel read back.
					context.save();
					const filter = this._colorFilters.filterFor([primitive]);
					if (filter) context.filter = this._composeFilter(context, filter);
					this._scratch.drawOnto(context, content, region);
					context.restore();
					return;
				}
				VectorPrimitiveRenderer._modifyScratch(content, primitive);
				this._scratch.drawOnto(context, content, region);
			});
		}

		// Replace and interpolate are composited fills, which all browsers do.
		private static _modifiesByCompositing(
			modifier: ColorModifierAttribute,
		): boolean {
			return (
				modifier.modifier === 'replace' || modifier.modifier === 'interpolate'
			);
		}

		// Change the colors on a scratch canvas, keeping its alpha. Replace and
		// interpolate are composited fills, the rest read the pixels back.
		private static _modifyScratch(
			content: CanvasRenderingContext2D,
			modifier: ColorModifierAttribute,
		): void {
			const width = content.canvas.width;
			const height = content.canvas.height;
			// The engine holds the value of an interpolate between 0 and 1.
			const value = Math.min(Math.max(modifier.value ?? 0, 0), 1);

			content.save();
			content.setTransform(1, 0, 0, 1, 0, 0);
			if (modifier.modifier === 'replace') {
				// Source in keeps the content's alpha and takes the color.
				content.globalCompositeOperation = 'source-in';
				content.fillStyle = modifier.color ?? '#000000';
				content.fillRect(0, 0, width, height);
			} else if (modifier.modifier === 'interpolate') {
				// Source atop keeps the alpha and lays the color at one minus
				// the value over the original.
				content.globalCompositeOperation = 'source-atop';
				content.globalAlpha = 1 - value;
				content.fillStyle = modifier.color ?? '#000000';
				content.fillRect(0, 0, width, height);
			} else {
				const pixels = content.getImageData(0, 0, width, height);
				VectorColorModifiers.applyToPixels(pixels.data, [modifier]);
				content.putImageData(pixels, 0, 0);
			}
			content.restore();
		}

		private _renderGrayOrInvert(
			context: CanvasRenderingContext2D,
			children: Primitive[],
			bounds: number[] | undefined,
			invert: boolean,
		): void {
			// Without a scratch canvas the canvas filter does it, in the
			// browsers that have one.
			const filtered = (): void => {
				context.save();
				context.filter = this._composeFilter(
					context,
					invert ? 'invert(1)' : 'grayscale(1)',
				);
				this._renderPrimitives(context, children);
				context.restore();
			};
			this._scratch.withEffectSlots(filtered, (slot) => {
				if (
					!this._renderModifiedOnScratch(
						context,
						slot,
						children,
						bounds,
						invert,
					)
				)
					filtered();
			});
		}

		// Gray or invert a subtree with blend modes, which every browser
		// has. A blend works on colors that are not premultiplied and
		// mixes in the backdrop where the content is partly transparent.
		// Over opaque black each pixel is its premultiplied color
		// instead, both modifiers map that exactly, and dividing by the
		// alpha afterwards gives the plain color back. False when there
		// is no canvas for it.
		private _renderModifiedOnScratch(
			context: CanvasRenderingContext2D,
			slot: number,
			children: Primitive[],
			bounds: number[] | undefined,
			invert: boolean,
		): boolean {
			const region = VectorScratchCanvases.effectRegion(context, bounds, 0);
			if (!region) return true;
			const content = this._renderToScratch(context, slot, children, region);
			if (!content) return false;
			const width = region.width;
			const height = region.height;
			const color = this._scratch.slotContext(slot + 1, width, height);
			const alpha = color
				? this._scratch.slotContext(slot + 2, width, height)
				: null;
			if (!color || !alpha) return false;

			color.save();
			color.fillStyle = '#000000';
			color.fillRect(0, 0, width, height);
			color.drawImage(content.canvas, 0, 0);

			// The alpha as an opaque gray level, white where the content
			// is solid.
			alpha.save();
			alpha.drawImage(content.canvas, 0, 0);
			alpha.globalCompositeOperation = 'source-in';
			alpha.fillStyle = '#ffffff';
			alpha.fillRect(0, 0, width, height);
			alpha.globalCompositeOperation = 'destination-over';
			alpha.fillStyle = '#000000';
			alpha.fillRect(0, 0, width, height);

			if (invert) {
				// The alpha less the premultiplied color is the
				// premultiplied inverse.
				color.globalCompositeOperation = 'difference';
				color.drawImage(alpha.canvas, 0, 0);
			} else {
				// A gray has no saturation, so the blend keeps only the
				// luminosity, with the weights the engine uses.
				color.globalCompositeOperation = 'saturation';
				color.fillStyle = '#808080';
				color.fillRect(0, 0, width, height);
			}

			// Color dodge divides by one less the source, so one less
			// the alpha divides the premultiplied color by the alpha.
			alpha.globalCompositeOperation = 'difference';
			alpha.fillStyle = '#ffffff';
			alpha.fillRect(0, 0, width, height);
			color.globalCompositeOperation = 'color-dodge';
			color.drawImage(alpha.canvas, 0, 0);

			color.globalCompositeOperation = 'destination-in';
			color.drawImage(content.canvas, 0, 0);
			alpha.restore();
			color.restore();

			this._scratch.drawOnto(context, color, region);
			return true;
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

		/// Drop the effects' scratch canvases. The next effect makes new ones.
		releaseScratchCanvases(): void {
			this._scratch.release();
		}

		/// Drop the color-changed images and filters. Draws rebuild them.
		releaseColorChangedImages(): void {
			this._modifiedImages.release();
			this._colorFilters.release();
		}

		// Gaussian deviation for a blur radius. The engine takes the radius as
		// three deviations.
		private static _blurDeviation(radius: number): number {
			return radius / 3;
		}

		// Smallest blur or grow, in pixels, that shows.
		private static readonly _VISIBLE_SPREAD = 0.5;

		// How far a Gaussian blur reaches: three deviations.
		private static _blurReach(deviation: number): number {
			return 3 * deviation;
		}

		private _renderHatch(
			context: CanvasRenderingContext2D,
			primitive: FillHatchPrimitive | PolyPolygonHatchPrimitive,
		): void {
			const hatch = primitive.hatch;
			if (!hatch) return;

			const path = (primitive as PolyPolygonHatchPrimitive).path;
			const output = (primitive as FillHatchPrimitive).outputRange;
			const area = Range2D.fromArray(
				(primitive as PolyPolygonHatchPrimitive).bounds ?? output,
			);
			if (!area) return;

			context.save();
			if (path) context.clip(new Path2D(path), 'evenodd');
			else VectorScratchCanvases.clipToRange(context, area);

			if (hatch.fillBackground && primitive.backgroundColor) {
				context.fillStyle = primitive.backgroundColor;
				context.fillRect(area.minX, area.minY, area.width, area.height);
			}

			this._drawHatchLines(
				context,
				hatch,
				Range2D.fromArray(primitive.definitionRange) ?? area,
				area,
			);
			context.restore();
		}

		// Single is one set along the hatch angle, double adds one square
		// to it, and triple a third at forty-five degrees.
		private _drawHatchLines(
			context: CanvasRenderingContext2D,
			hatch: HatchAttribute,
			layout: Range2D,
			area: Range2D,
		): void {
			const pixels = VectorScratchCanvases.pixelsPerUnit(context);
			const distance = hatch.distance ?? 0;
			if (!(distance > 0)) return;

			// Lines under a pixel apart read as a solid area and cost a
			// stroke each, so they stop at a pixel.
			const spacing = Math.max(distance, 1 / pixels);
			const angle = hatch.angle ?? 0;
			const angles = [angle];
			if (hatch.style === 'double' || hatch.style === 'triple')
				angles.push(angle - Math.PI / 2);
			if (hatch.style === 'triple') angles.push(angle - Math.PI / 4);

			const centerX = layout.centerX;
			const centerY = layout.centerY;
			// Far enough to cross the whole area, even though the lines
			// are laid out around the middle of the definition range.
			const reach =
				Math.hypot(area.width, area.height) / 2 +
				Math.hypot(centerX - area.centerX, centerY - area.centerY);
			const steps = Math.ceil(reach / spacing);

			context.strokeStyle = hatch.color ?? '#000000';
			context.lineWidth = 1 / pixels;
			context.beginPath();
			for (const lineAngle of angles) {
				// The hatch angle turns anticlockwise, y grows down.
				const alongX = Math.cos(lineAngle);
				const alongY = -Math.sin(lineAngle);
				const acrossX = Math.sin(lineAngle);
				const acrossY = Math.cos(lineAngle);
				for (let step = -steps; step <= steps; step++) {
					const offset = step * spacing;
					const originX = centerX + acrossX * offset;
					const originY = centerY + acrossY * offset;
					context.moveTo(originX - alongX * reach, originY - alongY * reach);
					context.lineTo(originX + alongX * reach, originY + alongY * reach);
				}
			}
			context.stroke();
		}

		private _renderPatternFill(
			context: CanvasRenderingContext2D,
			primitive: PatternFillPrimitive,
		): void {
			const path = primitive.path;
			const area = primitive.bounds;
			const reference = primitive.referenceRange;
			if (!path || !area || !reference) return;
			if (area.length < 4 || reference.length < 4) return;
			const children = primitive.children;
			if (!children) return;

			const areaWidth = area[2] - area[0];
			const areaHeight = area[3] - area[1];
			const unitWidth = reference[2] - reference[0];
			const unitHeight = reference[3] - reference[1];
			const tileWidth = unitWidth * areaWidth;
			const tileHeight = unitHeight * areaHeight;
			if (!(tileWidth > 0 && tileHeight > 0)) return;

			const pixels = VectorScratchCanvases.pixelsPerUnit(context);
			// Skip tiles smaller than a pixel. They draw nothing visible,
			// and there would be one per pixel.
			if (tileWidth * pixels < 1 || tileHeight * pixels < 1) return;

			context.save();
			context.clip(new Path2D(path), 'evenodd');
			// The tiles are laid out in the unit square of the bounds.
			context.translate(area[0], area[1]);
			context.scale(areaWidth, areaHeight);

			// Repeat one drawn tile, or draw each tile when there is no canvas
			// for one.
			const eachTile = () =>
				this._drawEachPatternTile(context, reference, children);
			this._scratch.withEffectSlots(eachTile, (slot) => {
				if (!this._fillWithPatternTile(context, slot, reference, children)) {
					eachTile();
				}
			});

			context.restore();
		}

		// Map the children onto each tile, clipped to it as in the engine.
		private _drawEachPatternTile(
			context: CanvasRenderingContext2D,
			reference: number[],
			children: Primitive[],
		): void {
			const unitWidth = reference[2] - reference[0];
			const unitHeight = reference[3] - reference[1];
			VectorScratchCanvases.iterateTiles(reference, 0, 0, (x, y) => {
				context.save();
				context.translate(x, y);
				context.scale(unitWidth, unitHeight);
				context.beginPath();
				context.rect(0, 0, 1, 1);
				context.clip();
				this._renderPrimitives(context, children);
				context.restore();
			});
		}

		// Fill the bounds with one tile drawn at the target's resolution. False
		// without a canvas, or when the tile is larger than the target.
		private _fillWithPatternTile(
			context: CanvasRenderingContext2D,
			slot: number,
			reference: number[],
			children: Primitive[],
		): boolean {
			const unitWidth = reference[2] - reference[0];
			const unitHeight = reference[3] - reference[1];
			const matrix = context.getTransform();
			const width = Math.ceil(Math.hypot(matrix.a, matrix.b) * unitWidth);
			const height = Math.ceil(Math.hypot(matrix.c, matrix.d) * unitHeight);
			if (width > context.canvas.width || height > context.canvas.height)
				return false;

			const tile = this._scratch.slotContext(slot, width, height);
			if (!tile) return false;
			tile.setTransform(width, 0, 0, height, 0, 0);
			this._renderPrimitives(tile, children);

			const pattern = context.createPattern(tile.canvas, 'repeat');
			if (!pattern) return false;
			// Scale the whole-pixel canvas back to the exact tile size.
			pattern.setTransform({
				a: unitWidth / width,
				b: 0,
				c: 0,
				d: unitHeight / height,
				e: reference[0],
				f: reference[1],
			});
			context.fillStyle = pattern;
			context.fillRect(0, 0, 1, 1);
			return true;
		}

		// Draw a subtree onto a scratch canvas aligned with the region. A
		// matrix applies on top of the target's transform.
		private _renderToScratch(
			context: CanvasRenderingContext2D,
			slot: number,
			children: Primitive[] | undefined,
			region: ScratchRegion,
			matrix?: number[],
		): CanvasRenderingContext2D | null {
			const scratch = this._scratch.slotContext(
				slot,
				region.width,
				region.height,
			);
			if (!scratch) return null;

			VectorScratchCanvases.alignWith(scratch, context, region);
			if (matrix && matrix.length >= 6) {
				scratch.transform(
					matrix[0],
					matrix[1],
					matrix[2],
					matrix[3],
					matrix[4],
					matrix[5],
				);
			}
			if (children) this._renderPrimitives(scratch, children);
			return scratch;
		}

		// Draw the source's coverage onto a same-size target in one color,
		// blurred by a Gaussian of the deviation in pixels. The canvas shadow
		// does the blur, with shadowBlur twice the deviation, and every browser
		// supports it. The source goes one width off to the left, so only its
		// shadow lands.
		private static _drawBlurredSilhouette(
			target: CanvasRenderingContext2D,
			source: HTMLCanvasElement,
			color: string,
			deviation: number,
		): void {
			target.save();
			target.setTransform(1, 0, 0, 1, 0, 0);
			target.shadowColor = color;
			target.shadowBlur = 2 * deviation;
			target.shadowOffsetX = source.width;
			target.shadowOffsetY = 0;
			target.drawImage(source, -source.width, 0);
			target.restore();
		}

		// Draws the children unchanged.
		private _childrenPainter(
			context: CanvasRenderingContext2D,
			primitive: Primitive,
		): () => void {
			return () => {
				if (primitive.children)
					this._renderPrimitives(context, primitive.children);
			};
		}

		// Grow the scratch content by the given pixels, drawing the canvas onto
		// itself at offsets around a full turn.
		private _dilateScratch(
			scratch: CanvasRenderingContext2D,
			radius: number,
		): void {
			// Copies a couple of pixels apart, so thin shapes stay covered,
			// capped per effect.
			const steps = Math.min(
				32,
				Math.max(8, Math.ceil((Math.PI * radius) / 2)),
			);
			// Over a full turn the steps sum to pi offsets in any direction.
			const offset = (Math.PI * radius) / steps;

			scratch.save();
			scratch.setTransform(1, 0, 0, 1, 0, 0);
			scratch.globalCompositeOperation = 'source-over';
			for (let step = 0; step < steps; step++) {
				const angle = (step / steps) * 2 * Math.PI;
				scratch.drawImage(
					scratch.canvas,
					Math.cos(angle) * offset,
					Math.sin(angle) * offset,
				);
			}
			scratch.restore();
		}

		// Shrink the scratch content by growing its outside and cutting that
		// away.
		private _erodeScratch(
			scratch: CanvasRenderingContext2D,
			radius: number,
			outsideSlot: number,
		): void {
			const width = scratch.canvas.width;
			const height = scratch.canvas.height;
			const outside = this._scratch.slotContext(outsideSlot, width, height);
			if (!outside) return;

			// The outside, solid.
			outside.save();
			outside.setTransform(1, 0, 0, 1, 0, 0);
			outside.fillStyle = '#000000';
			outside.fillRect(0, 0, width, height);
			outside.globalCompositeOperation = 'destination-out';
			outside.drawImage(scratch.canvas, 0, 0);
			outside.restore();

			this._dilateScratch(outside, radius);

			scratch.save();
			scratch.setTransform(1, 0, 0, 1, 0, 0);
			scratch.globalCompositeOperation = 'destination-out';
			scratch.drawImage(outside.canvas, 0, 0);
			scratch.restore();
		}

		// A subtree in one color, moved by the matrix, grown by the dilate
		// radius and blurred by a Gaussian of the deviation in pixels. The
		// bounds cover the result. The subtree goes on the slot's canvas, the
		// result on the next.
		private _renderColoredBlur(
			context: CanvasRenderingContext2D,
			children: Primitive[] | undefined,
			bounds: number[] | undefined,
			slot: number,
			color: string,
			deviation: number,
			dilateRadius: number,
			opacity: number,
			matrix?: number[],
		): void {
			const region = VectorScratchCanvases.effectRegion(
				context,
				bounds,
				dilateRadius + VectorPrimitiveRenderer._blurReach(deviation),
			);
			if (!region) return;
			const scratch = this._renderToScratch(
				context,
				slot,
				children,
				region,
				matrix,
			);
			if (!scratch) return;

			if (dilateRadius >= VectorPrimitiveRenderer._VISIBLE_SPREAD)
				this._dilateScratch(scratch, dilateRadius);
			const blurred = this._scratch.slotContext(
				slot + 1,
				region.width,
				region.height,
			);
			if (!blurred) return;
			VectorPrimitiveRenderer._drawBlurredSilhouette(
				blurred,
				scratch.canvas,
				color,
				deviation,
			);
			// A color modifier around the effect still applies, through the
			// target's filter.
			this._scratch.drawOnto(context, blurred, region, opacity);
		}

		private _renderShadow(
			context: CanvasRenderingContext2D,
			primitive: ShadowPrimitive,
		): void {
			// The object is its own primitive, so without a canvas the shadow
			// is left out.
			this._scratch.withEffectSlots(undefined, (slot) => {
				// Blurred by the whole radius, not grown.
				this._renderColoredBlur(
					context,
					primitive.children,
					primitive.bounds,
					slot,
					primitive.color ?? '#000000',
					VectorPrimitiveRenderer._blurDeviation(
						(primitive.blur ?? 0) *
							VectorScratchCanvases.pixelsPerUnit(context),
					),
					0,
					1,
					primitive.matrix,
				);
			});
		}

		private _renderGlow(
			context: CanvasRenderingContext2D,
			primitive: GlowPrimitive,
		): void {
			// The object is its own primitive, so without a canvas the halo is
			// left out.
			this._scratch.withEffectSlots(undefined, (slot) => {
				// Grow by half the radius and blur over the other half, as the
				// engine does, so the halo is solid at the object's edge.
				const radius =
					(primitive.radius ?? 0) *
					VectorScratchCanvases.pixelsPerUnit(context);
				this._renderColoredBlur(
					context,
					primitive.children,
					primitive.bounds,
					slot,
					primitive.color ?? '#000000',
					VectorPrimitiveRenderer._blurDeviation(radius / 2),
					radius / 2,
					1 - (primitive.transparency ?? 0),
				);
			});
		}

		private _renderSoftEdge(
			context: CanvasRenderingContext2D,
			primitive: SoftEdgePrimitive,
		): void {
			// Without a scratch canvas the children are drawn plainly.
			const plain = this._childrenPainter(context, primitive);
			this._scratch.withEffectSlots(plain, (slot) => {
				const radius =
					(primitive.radius ?? 0) *
					VectorScratchCanvases.pixelsPerUnit(context);
				const deviation = VectorPrimitiveRenderer._blurDeviation(radius);
				// Margin for the erode and the blur, so the shape past the
				// target's edge still counts as the shape.
				const region = VectorScratchCanvases.effectRegion(
					context,
					primitive.bounds,
					radius + VectorPrimitiveRenderer._blurReach(deviation),
				);
				if (!region) return;
				const content = this._renderToScratch(
					context,
					slot,
					primitive.children,
					region,
				);
				if (!content) {
					plain();
					return;
				}

				const shrunk =
					radius >= VectorPrimitiveRenderer._VISIBLE_SPREAD
						? this._scratch.slotContext(slot + 1, region.width, region.height)
						: null;
				if (!shrunk) {
					this._scratch.drawOnto(context, content, region);
					return;
				}

				// Erode by the radius and blur back out, so the alpha rises
				// from zero at the edge to solid a radius inside.
				shrunk.save();
				shrunk.setTransform(1, 0, 0, 1, 0, 0);
				shrunk.globalCompositeOperation = 'copy';
				shrunk.drawImage(content.canvas, 0, 0);
				shrunk.restore();
				this._erodeScratch(shrunk, radius, slot + 2);

				// The erode's canvas takes the blurred mask.
				const fade = this._scratch.slotContext(
					slot + 2,
					region.width,
					region.height,
				);
				if (!fade) {
					this._scratch.drawOnto(context, content, region);
					return;
				}
				VectorPrimitiveRenderer._drawBlurredSilhouette(
					fade,
					shrunk.canvas,
					'#000000',
					deviation,
				);

				// The fade multiplies the children's own alpha.
				content.save();
				content.setTransform(1, 0, 0, 1, 0, 0);
				content.globalCompositeOperation = 'destination-in';
				content.drawImage(fade.canvas, 0, 0);
				content.restore();

				this._scratch.drawOnto(context, content, region);
			});
		}

		private _renderTransparence(
			context: CanvasRenderingContext2D,
			primitive: TransparencePrimitive,
		): void {
			// An empty mask hides everything.
			const maskPrimitives = primitive.transparence;
			if (!maskPrimitives?.length) return;

			// Without a scratch canvas the children are drawn plainly.
			const plain = this._childrenPainter(context, primitive);
			this._scratch.withEffectSlots(plain, (slot) => {
				const region = VectorScratchCanvases.effectRegion(
					context,
					primitive.bounds,
					0,
				);
				if (!region) return;
				const content = this._renderToScratch(
					context,
					slot,
					primitive.children,
					region,
				);
				const mask = content
					? this._scratch.slotContext(slot + 1, region.width, region.height)
					: null;
				if (!content || !mask) {
					plain();
					return;
				}

				// The mask goes over white, which is fully clear, as in the
				// engine.
				const width = region.width;
				const height = region.height;
				mask.setTransform(1, 0, 0, 1, 0, 0);
				mask.fillStyle = '#ffffff';
				mask.fillRect(0, 0, width, height);
				VectorScratchCanvases.alignWith(mask, context, region);
				this._renderPrimitives(mask, maskPrimitives);

				const contentImage = content.getImageData(0, 0, width, height);
				const maskImage = mask.getImageData(0, 0, width, height);
				VectorPrimitiveRenderer._applyLuminanceMask(
					contentImage.data,
					maskImage.data,
				);
				content.putImageData(contentImage, 0, 0);

				this._scratch.drawOnto(context, content, region);
			});
		}

		// Scale the content's alpha by one minus the mask's luminance, with the
		// engine's weights.
		private static _applyLuminanceMask(
			content: Uint8ClampedArray,
			mask: Uint8ClampedArray,
		): void {
			for (let index = 0; index < content.length; index += 4) {
				const luminance =
					mask[index] * 0.2125 +
					mask[index + 1] * 0.7154 +
					mask[index + 2] * 0.0721;
				content[index + 3] = Math.round(
					content[index + 3] * (1 - luminance / 255),
				);
			}
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
