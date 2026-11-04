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
	/// A Gaussian blur of RGBA pixels, as three box blurs across and three
	/// down. Pixels outside the image count as transparent.
	export class VectorImageBlur {
		private static readonly _BOX_COUNT = 3;

		/// Blur the pixels in place. The deviation is in pixels.
		static blur(
			data: Uint8ClampedArray,
			width: number,
			height: number,
			deviation: number,
		): void {
			if (!(deviation > 0) || !(width > 0 && height > 0)) return;

			// The colors blur weighted by alpha, so a transparent pixel adds no
			// color to its neighbors.
			const count = width * height;
			const channels = [
				new Float32Array(count),
				new Float32Array(count),
				new Float32Array(count),
				new Float32Array(count),
			];
			for (let index = 0; index < count; index++) {
				const alpha = data[4 * index + 3];
				channels[0][index] = (data[4 * index] * alpha) / 255;
				channels[1][index] = (data[4 * index + 1] * alpha) / 255;
				channels[2][index] = (data[4 * index + 2] * alpha) / 255;
				channels[3][index] = alpha;
			}

			const pass = new Float32Array(count);
			for (const size of VectorImageBlur.boxSizes(deviation)) {
				const radius = (size - 1) / 2;
				for (const channel of channels) {
					// Along the rows, then down the columns.
					VectorImageBlur._boxBlur(
						channel,
						pass,
						height,
						width,
						width,
						1,
						radius,
					);
					VectorImageBlur._boxBlur(
						pass,
						channel,
						width,
						1,
						height,
						width,
						radius,
					);
				}
			}

			for (let index = 0; index < count; index++) {
				const alpha = channels[3][index];
				const scale = alpha > 0 ? 255 / alpha : 0;
				data[4 * index] = channels[0][index] * scale;
				data[4 * index + 1] = channels[1][index] * scale;
				data[4 * index + 2] = channels[2][index] * scale;
				data[4 * index + 3] = alpha;
			}
		}

		/// The odd widths of the three boxes closest to a Gaussian of the
		/// deviation.
		static boxSizes(deviation: number): number[] {
			const boxes = VectorImageBlur._BOX_COUNT;
			const variance = deviation * deviation;
			let lower = Math.floor(Math.sqrt((12 * variance) / boxes + 1));
			if (lower % 2 === 0) lower--;
			const upper = lower + 2;
			// How many boxes take the lower width, for the closest variance.
			const lowerCount = Math.round(
				(12 * variance -
					boxes * lower * lower -
					4 * boxes * lower -
					3 * boxes) /
					(-4 * lower - 4),
			);
			const sizes: number[] = [];
			for (let box = 0; box < boxes; box++)
				sizes.push(box < lowerCount ? lower : upper);
			return sizes;
		}

		// Average each value with radius values on each side. Lines start
		// lineStep apart, and the values of a line are step apart.
		private static _boxBlur(
			source: Float32Array,
			target: Float32Array,
			lines: number,
			lineStep: number,
			length: number,
			step: number,
			radius: number,
		): void {
			const size = 2 * radius + 1;
			for (let line = 0; line < lines; line++) {
				const start = line * lineStep;
				let sum = 0;
				for (let offset = 0; offset <= Math.min(radius, length - 1); offset++)
					sum += source[start + offset * step];
				for (let position = 0; position < length; position++) {
					target[start + position * step] = sum / size;
					const entering = position + radius + 1;
					if (entering < length) sum += source[start + entering * step];
					const leaving = position - radius;
					if (leaving >= 0) sum -= source[start + leaving * step];
				}
			}
		}
	}
}
