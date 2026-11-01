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
	/// Renders the wave under a misspelled word with Canvas 2D drawing operations.
	export class VectorWrongSpellPrimitiveRenderer {
		// A hairline wave along the line, as the drawing layer draws it: one wave every 8
		// device pixels, and nothing under a word smaller than 5 pixels. pixel is the size of
		// a device pixel in the units of the context.
		static render(
			context: CanvasRenderingContext2D,
			primitive: WrongSpellPrimitive,
			pixel: number,
		): void {
			if ((primitive.fontHeight ?? 0) / pixel < 5) return;

			const startX = primitive.startX ?? 0;
			const startY = primitive.startY ?? 0;
			const length = Math.hypot(
				(primitive.endX ?? 0) - startX,
				(primitive.endY ?? 0) - startY,
			);
			const waveLength = pixel * 8;
			if (length === 0 || length / waveLength > 1024) return;

			const directionX = ((primitive.endX ?? 0) - startX) / length;
			const directionY = ((primitive.endY ?? 0) - startY) / length;
			const stepX = directionX * waveLength;
			const stepY = directionY * waveLength;
			// The control points sit half a wave length to the side, half way along.
			const controlX = -directionY * waveLength * 0.5 + stepX * 0.5;
			const controlY = directionX * waveLength * 0.5 + stepY * 0.5;

			context.save();
			context.strokeStyle = primitive.color ?? '#000000';
			context.lineWidth = pixel;
			context.beginPath();
			let x = startX;
			let y = startY;
			context.moveTo(x, y);
			let rest = length;
			while (rest > waveLength) {
				context.bezierCurveTo(
					x + controlX,
					y + controlY,
					x + stepX - controlX,
					y + stepY - controlY,
					x + stepX,
					y + stepY,
				);
				x += stepX;
				y += stepY;
				rest -= waveLength;
			}
			// A rest of more than a fifth of a wave is the start of one more wave.
			if (rest > waveLength * 0.2) {
				const [firstControl, secondControl, tip] =
					VectorWrongSpellPrimitiveRenderer._cubicPart(
						{ x, y },
						{ x: x + controlX, y: y + controlY },
						{ x: x + stepX - controlX, y: y + stepY - controlY },
						{ x: x + stepX, y: y + stepY },
						rest / waveLength,
					);
				context.bezierCurveTo(
					firstControl.x,
					firstControl.y,
					secondControl.x,
					secondControl.y,
					tip.x,
					tip.y,
				);
			}
			context.stroke();
			context.restore();
		}

		// The control points and the end of a cubic curve cut at the fraction, by de
		// Casteljau's construction.
		private static _cubicPart(
			start: PointLike,
			controlA: PointLike,
			controlB: PointLike,
			end: PointLike,
			fraction: number,
		): [PointLike, PointLike, PointLike] {
			const between = (from: PointLike, to: PointLike): PointLike => ({
				x: from.x + (to.x - from.x) * fraction,
				y: from.y + (to.y - from.y) * fraction,
			});
			const startToA = between(start, controlA);
			const aToB = between(controlA, controlB);
			const bToEnd = between(controlB, end);
			const secondControl = between(startToA, aToB);
			const tip = between(secondControl, between(aToB, bToEnd));
			return [startToA, secondControl, tip];
		}
	}
}
