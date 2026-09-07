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
	/// A color split into its parts, so mixing needs no string parsing.
	interface RampColor {
		red: number;
		green: number;
		blue: number;
		alpha: number;
	}

	/// A ramp stop ready to paint with.
	interface RampStop extends RampColor {
		offset: number;
	}

	/// Renders the gradient primitives with Canvas 2D drawing operations.
	export class VectorGradientPrimitiveRenderer {
		private _scratch: VectorScratchCanvases;

		constructor(scratch: VectorScratchCanvases) {
			this._scratch = scratch;
		}

		renderFillGradient(
			context: CanvasRenderingContext2D,
			primitive: FillGradientPrimitive,
		): void {
			const gradient = primitive.gradient;
			const output = Range2D.fromArray(primitive.outputRange);
			if (!gradient || !output) return;

			this._paintGradient(
				context,
				gradient,
				primitive.alphaGradient,
				Range2D.fromArray(primitive.definitionRange) ?? output,
				primitive.transparency,
				() => VectorScratchCanvases.clipToRange(context, output),
			);
		}

		renderPolyPolygonGradient(
			context: CanvasRenderingContext2D,
			primitive: PolyPolygonGradientPrimitive,
		): void {
			const gradient = primitive.gradient;
			const path = primitive.path;
			if (!gradient || !path) return;

			const range =
				Range2D.fromArray(primitive.definitionRange) ??
				Range2D.fromArray(primitive.bounds);
			this._paintGradient(
				context,
				gradient,
				primitive.alphaGradient,
				range,
				primitive.transparency,
				() => context.clip(new Path2D(path), 'evenodd'),
			);
		}

		renderPolyPolygonAlphaGradient(
			context: CanvasRenderingContext2D,
			primitive: PolyPolygonAlphaGradientPrimitive,
		): void {
			const gradient = primitive.alphaGradient;
			const path = primitive.path;
			if (!gradient || !path) return;

			const fill = VectorGradientPrimitiveRenderer._parseColor(primitive.color);
			const stops = VectorGradientPrimitiveRenderer._rampStops(gradient, fill);

			context.save();
			context.clip(new Path2D(path), 'evenodd');
			this._fillRamp(
				context,
				gradient,
				Range2D.fromArray(primitive.bounds),
				stops,
			);
			context.restore();
		}

		// Fill the clip with a color ramp, its transparency from an alpha
		// ramp when there is one.
		private _paintGradient(
			context: CanvasRenderingContext2D,
			gradient: GradientAttribute,
			alphaGradient: GradientAttribute | undefined,
			range: Range2D | null,
			transparency: number | undefined,
			clip: () => void,
		): void {
			const stops = VectorGradientPrimitiveRenderer._rampStops(gradient);
			if (!stops.length) return;

			context.save();
			if (transparency) context.globalAlpha = 1 - transparency;
			clip();

			if (!alphaGradient) {
				this._fillRamp(context, gradient, range, stops);
				context.restore();
				return;
			}

			const width = context.canvas.width;
			const height = context.canvas.height;
			const scratch =
				width > 0 && height > 0 ? this._scratch.context(width, height) : null;
			if (!scratch) {
				// Without a scratch canvas the ramp is painted with
				// nothing taken back out.
				this._fillRamp(context, gradient, range, stops);
				context.restore();
				return;
			}

			// The alpha ramp takes away what it says is clear.
			scratch.setTransform(context.getTransform());
			this._fillRamp(scratch, gradient, range, stops);
			scratch.globalCompositeOperation = 'destination-in';
			this._fillRamp(
				scratch,
				alphaGradient,
				range,
				VectorGradientPrimitiveRenderer._rampStops(alphaGradient, {
					red: 0,
					green: 0,
					blue: 0,
					alpha: 1,
				}),
			);
			scratch.globalCompositeOperation = 'source-over';
			this._scratch.drawOnto(context, scratch);
			context.restore();
		}

		// The canvas gradient for a ramp, in the ramp's own space.
		private _rampGradient(
			context: CanvasRenderingContext2D,
			style: string | undefined,
			stops: RampStop[],
		): CanvasGradient {
			if (style === 'axial') {
				// An axial ramp has its first color in the center. The
				// mirrored gradient puts the last stop in the middle, so
				// the stops go in reversed.
				return VectorGradientPrimitiveRenderer._mirroredGradient(
					context,
					stops.map((stop) => ({ ...stop, offset: 1 - stop.offset })),
					0,
					-1,
					0,
					1,
				);
			}

			if (style === 'radial' || style === 'elliptical') {
				// A round ramp has its last color in the center and its
				// first at the rim, which the frame puts at one. The
				// engine flattens an elliptical ramp's inner bands
				// towards its long axis; these rings keep one shape.
				const gradient = context.createRadialGradient(0, 0, 0, 0, 0, 1);
				for (const stop of stops) {
					gradient.addColorStop(
						1 - stop.offset,
						VectorGradientPrimitiveRenderer._cssColor(stop),
					);
				}
				return gradient;
			}

			const gradient = context.createLinearGradient(0, 0, 0, 1);
			for (const stop of stops) {
				gradient.addColorStop(
					stop.offset,
					VectorGradientPrimitiveRenderer._cssColor(stop),
				);
			}
			return gradient;
		}

		// Paint a ramp over the clip. The frame places it, so the fill
		// works in ramp space.
		private _fillRamp(
			context: CanvasRenderingContext2D,
			gradient: GradientAttribute,
			range: Range2D | null,
			stops: RampStop[],
		): void {
			if (!stops.length) return;

			const frame = VectorGradientFrame.create(gradient, range);
			if (!frame) return;

			const reach = VectorGradientPrimitiveRenderer._RAMP_REACH;
			context.save();
			frame.matrix.applyTo(context);

			if (stops.length === 1) {
				context.fillStyle = VectorGradientPrimitiveRenderer._cssColor(stops[0]);
				context.fillRect(-reach, -reach, reach * 2, reach * 2);
			} else if (gradient.style === 'square' || gradient.style === 'rect') {
				this._fillBoxedRamp(context, stops, frame.aspectRatio);
			} else {
				context.fillStyle = this._rampGradient(context, gradient.style, stops);
				context.fillRect(-reach, -reach, reach * 2, reach * 2);
			}

			context.restore();
		}

		// A linear gradient read inwards from both ends, so the last
		// stop sits in the middle. The middle stretch, inner in units of
		// the line's half-length, holds that color throughout.
		private static _mirroredGradient(
			context: CanvasRenderingContext2D,
			stops: RampStop[],
			x0: number,
			y0: number,
			x1: number,
			y1: number,
			inner = 0,
		): CanvasGradient {
			const gradient = context.createLinearGradient(x0, y0, x1, y1);
			const span = (1 - inner) / 2;
			for (const stop of stops) {
				const color = VectorGradientPrimitiveRenderer._cssColor(stop);
				gradient.addColorStop(stop.offset * span, color);
				gradient.addColorStop(1 - stop.offset * span, color);
			}
			return gradient;
		}

		// The square and rect ramps fall off from each edge of a box to
		// the middle. That is the ramp mirrored top to bottom, with the
		// ramp mirrored side to side over the left and right wedges, and
		// both give the same color along a wedge edge. The engine
		// stretches the wider axis, so the last stop lands on a segment
		// rather than a point.
		private _fillBoxedRamp(
			context: CanvasRenderingContext2D,
			stops: RampStop[],
			aspectRatio: number,
		): void {
			const reach = VectorGradientPrimitiveRenderer._RAMP_REACH;
			// Outside the box the ramp holds its first color.
			context.fillStyle = VectorGradientPrimitiveRenderer._cssColor(stops[0]);
			context.fillRect(-reach, -reach, reach * 2, reach * 2);

			// Half-length of the segment the last stop lands on, x for a
			// wide box and y for a tall one.
			const innerX = aspectRatio > 1 ? 1 - 1 / aspectRatio : 0;
			const innerY = aspectRatio > 0 && aspectRatio < 1 ? 1 - aspectRatio : 0;

			context.fillStyle = VectorGradientPrimitiveRenderer._mirroredGradient(
				context,
				stops,
				0,
				-1,
				0,
				1,
				innerY,
			);
			context.fillRect(-1, -1, 2, 2);

			context.beginPath();
			context.moveTo(-1, -1);
			context.lineTo(-innerX, -innerY);
			context.lineTo(-innerX, innerY);
			context.lineTo(-1, 1);
			context.closePath();
			context.moveTo(1, -1);
			context.lineTo(innerX, -innerY);
			context.lineTo(innerX, innerY);
			context.lineTo(1, 1);
			context.closePath();
			context.fillStyle = VectorGradientPrimitiveRenderer._mirroredGradient(
				context,
				stops,
				-1,
				0,
				1,
				0,
				innerX,
			);
			context.fill();
		}

		// How far to fill in ramp space. The ramp reaches one unit, and
		// the color past the last stop carries on to the edge of the clip.
		private static readonly _RAMP_REACH = 4;

		private static _parseColor(color: string | undefined): RampColor {
			if (!color || color.charAt(0) !== '#' || color.length < 7)
				return { red: 0, green: 0, blue: 0, alpha: 1 };
			return {
				red: parseInt(color.substring(1, 3), 16),
				green: parseInt(color.substring(3, 5), 16),
				blue: parseInt(color.substring(5, 7), 16),
				alpha: 1,
			};
		}

		private static _cssColor(color: RampColor): string {
			return (
				'rgba(' +
				color.red +
				',' +
				color.green +
				',' +
				color.blue +
				',' +
				color.alpha +
				')'
			);
		}

		private static _mixColor(
			from: RampColor,
			to: RampColor,
			position: number,
		): RampColor {
			const mix = (start: number, end: number): number =>
				start + (end - start) * position;
			return {
				red: Math.round(mix(from.red, to.red)),
				green: Math.round(mix(from.green, to.green)),
				blue: Math.round(mix(from.blue, to.blue)),
				alpha: mix(from.alpha, to.alpha),
			};
		}

		// The stops a gradient describes. An alpha ramp keeps the given
		// color and reads each stop's luminance as its transparency.
		private static _rampStops(
			gradient: GradientAttribute,
			fill?: RampColor,
		): RampStop[] {
			const source = gradient.colorStops;
			if (!source || !source.length) return [];

			let stops: RampStop[] = source.map((stop) => {
				const color = VectorGradientPrimitiveRenderer._parseColor(stop.color);
				if (typeof stop.opacity === 'number') color.alpha = stop.opacity;
				if (fill) {
					const luminance =
						(color.red * 0.299 + color.green * 0.587 + color.blue * 0.114) /
						255;
					return {
						offset: stop.offset,
						red: fill.red,
						green: fill.green,
						blue: fill.blue,
						alpha: 1 - luminance,
					};
				}
				return { offset: stop.offset, ...color };
			});

			const steps = gradient.steps ?? 0;
			if (steps > 0 && stops.length > 1)
				stops = VectorGradientPrimitiveRenderer._bandStops(stops, steps);

			return stops;
		}

		// The most bands worth drawing between two colors. The engine
		// takes it from their distance in the widest channel.
		private static _bandLimit(from: RampColor, to: RampColor): number {
			const distance = Math.max(
				Math.abs(from.red - to.red),
				Math.abs(from.green - to.green),
				Math.abs(from.blue - to.blue),
			);
			return Math.round((distance / 255) * 127.5);
		}

		// Flat bands the way the engine counts them: the step count
		// applies per pair of stops, and the end bands carry the pair's
		// colors exactly.
		private static _bandStops(stops: RampStop[], steps: number): RampStop[] {
			const banded: RampStop[] = [];

			for (let index = 0; index < stops.length - 1; index++) {
				const from = stops[index];
				const to = stops[index + 1];
				const span = to.offset - from.offset;
				if (!(span > 0)) continue;

				const bands = Math.max(
					1,
					Math.min(steps, VectorGradientPrimitiveRenderer._bandLimit(from, to)),
				);
				const width = span / bands;
				for (let band = 0; band < bands; band++) {
					const color =
						bands === 1
							? from
							: VectorGradientPrimitiveRenderer._mixColor(
									from,
									to,
									band / (bands - 1),
								);
					banded.push({ offset: from.offset + band * width, ...color });
					banded.push({ offset: from.offset + (band + 1) * width, ...color });
				}
			}

			return banded.length ? banded : stops;
		}
	}
}
