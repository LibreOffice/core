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
	/// Changes red, green and blue, each from 0 to 1, in place.
	export type ColorChange = (color: number[]) => void;

	/// One step of a color modifier on red, green and blue, from 0 to 1.
	/// - "matrix": a row of four weights per channel, over red, green, blue
	///   and one. clamped holds each result between 0 and 1.
	/// - "linear": channel times slope plus intercept, held between 0 and 1.
	/// - "power": channel to the exponent, held between 0 and 1.
	export type ColorStep =
		| { kind: 'matrix'; rows: number[]; clamped: boolean }
		| { kind: 'linear'; slopes: number[]; intercepts: number[] }
		| { kind: 'power'; exponent: number };

	/// Color modifiers computed as the engine does, to match its rendering.
	export namespace VectorColorModifiers {
		function clampUnit(value: number): number {
			return Math.min(Math.max(value, 0), 1);
		}

		// "#rrggbb" as red, green and blue from 0 to 1. Black if unparsable.
		function parseColor(color: string | undefined): number[] {
			const value = color ? parseInt(color.replace('#', ''), 16) : NaN;
			if (isNaN(value)) return [0, 0, 0];
			return [
				((value >> 16) & 0xff) / 255,
				((value >> 8) & 0xff) / 255,
				(value & 0xff) / 255,
			];
		}

		// The engine's luminance weights, 77, 151 and 28 over 256.
		const LUMINANCE = [77 / 256, 151 / 256, 28 / 256];

		// The same row for each of red, green and blue, which makes a gray.
		function grayRows(row: number[]): number[] {
			return row.concat(row, row);
		}

		// Steep enough that the smallest 8 bit luminance step, 1 / 65280,
		// crosses the whole range, so the split matches the engine.
		const THRESHOLD_SLOPE = 1e6;

		// The engine's precomputed contrast slope and channel offsets.
		function luminanceContrastSteps(
			modifier: ColorModifierAttribute,
		): ColorStep[] {
			const red = Math.min(Math.max(modifier.red ?? 0, -1), 1);
			const green = Math.min(Math.max(modifier.green ?? 0, -1), 1);
			const blue = Math.min(Math.max(modifier.blue ?? 0, -1), 1);
			const brightness = Math.min(Math.max(modifier.luminance ?? 0, -1), 1);
			const contrast = Math.min(Math.max(modifier.contrast ?? 0, -1), 1);
			if (!red && !green && !blue && !brightness && !contrast) return [];

			const slope =
				contrast >= 0
					? 128 / (128 - contrast * 127)
					: (128 + contrast * 127) / 128;
			const shared = brightness + (128 - slope * 128) / 255;
			return [
				{
					kind: 'linear',
					slopes: [slope, slope, slope],
					intercepts: [red + shared, green + shared, blue + shared],
				},
			];
		}

		/// The modifier's steps, first to last. Empty for no change or unknown.
		export function steps(modifier: ColorModifierAttribute): ColorStep[] {
			switch (modifier.modifier) {
				case 'gray':
					return [
						{
							kind: 'matrix',
							rows: grayRows([...LUMINANCE, 0]),
							clamped: true,
						},
					];
				case 'invert':
					return [
						{ kind: 'linear', slopes: [-1, -1, -1], intercepts: [1, 1, 1] },
					];
				case 'luminance_to_alpha':
					return [
						{
							kind: 'matrix',
							rows: grayRows([-0.2125, -0.7154, -0.0721, 1]),
							clamped: true,
						},
					];
				case 'replace': {
					const [red, green, blue] = parseColor(modifier.color);
					return [
						{
							kind: 'matrix',
							rows: [0, 0, 0, red, 0, 0, 0, green, 0, 0, 0, blue],
							clamped: true,
						},
					];
				}
				case 'interpolate': {
					const [red, green, blue] = parseColor(modifier.color);
					// The engine holds the value between 0 and 1.
					const value = clampUnit(modifier.value ?? 0);
					const rest = 1 - value;
					return [
						{
							kind: 'matrix',
							rows: [
								value,
								0,
								0,
								red * rest,
								0,
								value,
								0,
								green * rest,
								0,
								0,
								value,
								blue * rest,
							],
							clamped: false,
						},
					];
				}
				case 'saturate':
				case 'hueRotate': {
					const matrix = modifier.matrix;
					if (!matrix || matrix.length !== 9) return [];
					return [
						{
							kind: 'matrix',
							rows: [
								...matrix.slice(0, 3),
								0,
								...matrix.slice(3, 6),
								0,
								...matrix.slice(6, 9),
								0,
							],
							// The engine clamps a hue rotation and not a saturation.
							clamped: modifier.modifier === 'hueRotate',
						},
					];
				}
				case 'matrix': {
					const matrix = modifier.matrix;
					if (!matrix || matrix.length !== 20) return [];
					// The engine takes alpha as one, so its column adds to the
					// constant. The alpha row has no effect.
					return [
						{
							kind: 'matrix',
							rows: [
								...matrix.slice(0, 3),
								matrix[3] + matrix[4],
								...matrix.slice(5, 8),
								matrix[8] + matrix[9],
								...matrix.slice(10, 13),
								matrix[13] + matrix[14],
							],
							clamped: true,
						},
					];
				}
				case 'black_and_white': {
					// Below the threshold is black, the rest is white.
					const threshold = modifier.value ?? 0.5;
					const slope = THRESHOLD_SLOPE;
					return [
						{
							kind: 'matrix',
							rows: grayRows([...LUMINANCE, 0]),
							clamped: true,
						},
						{
							kind: 'linear',
							slopes: [slope, slope, slope],
							intercepts: [
								1 - slope * threshold,
								1 - slope * threshold,
								1 - slope * threshold,
							],
						},
					];
				}
				case 'gamma': {
					const value = modifier.value ?? 1;
					// The engine ignores a gamma of one and one out of range.
					if (value === 1 || !(value > 0 && value <= 10)) return [];
					// The exact power. The engine's approximation is off by
					// up to 15 of 255 at gamma 0.5 and 7 at gamma 2.
					return [{ kind: 'power', exponent: 1 / value }];
				}
				case 'RGBLuminanceContrast':
					return luminanceContrastSteps(modifier);
				default:
					// randomize picks new colors on every draw. No steps.
					return [];
			}
		}

		function applyStep(color: number[], step: ColorStep): void {
			const red = color[0];
			const green = color[1];
			const blue = color[2];
			for (let channel = 0; channel < 3; channel++) {
				switch (step.kind) {
					case 'matrix': {
						const row = channel * 4;
						const value =
							step.rows[row] * red +
							step.rows[row + 1] * green +
							step.rows[row + 2] * blue +
							step.rows[row + 3];
						color[channel] = step.clamped ? clampUnit(value) : value;
						break;
					}
					case 'linear':
						color[channel] = clampUnit(
							color[channel] * step.slopes[channel] + step.intercepts[channel],
						);
						break;
					case 'power':
						color[channel] = clampUnit(Math.pow(color[channel], step.exponent));
						break;
				}
			}
		}

		/// The modifier's change to one color. Null for no change or unknown.
		export function colorChange(
			modifier: ColorModifierAttribute,
		): ColorChange | null {
			const modifierSteps = steps(modifier);
			if (!modifierSteps.length) return null;
			return (color) => {
				for (const step of modifierSteps) applyStep(color, step);
			};
		}

		/// Whether the modifier changes any color.
		export function changesColors(modifier: ColorModifierAttribute): boolean {
			return steps(modifier).length > 0;
		}

		/// Apply the modifiers, first to last, to unpremultiplied RGBA pixels.
		/// Alpha and fully transparent pixels stay as they are.
		export function applyToPixels(
			data: Uint8ClampedArray,
			modifiers: ColorModifierAttribute[],
		): void {
			const allSteps: ColorStep[] = [];
			for (const modifier of modifiers) allSteps.push(...steps(modifier));
			if (!allSteps.length) return;

			// Neighboring pixels often share a color, so reuse the last result.
			const color = [0, 0, 0];
			let lastKey = -1;
			let lastRed = 0;
			let lastGreen = 0;
			let lastBlue = 0;
			for (let index = 0; index < data.length; index += 4) {
				if (data[index + 3] === 0) continue;
				const key =
					(data[index] << 16) | (data[index + 1] << 8) | data[index + 2];
				if (key !== lastKey) {
					color[0] = data[index] / 255;
					color[1] = data[index + 1] / 255;
					color[2] = data[index + 2] / 255;
					for (const step of allSteps) applyStep(color, step);
					lastKey = key;
					// The array clamps to a byte and rounds.
					lastRed = color[0] * 255;
					lastGreen = color[1] * 255;
					lastBlue = color[2] * 255;
				}
				data[index] = lastRed;
				data[index + 1] = lastGreen;
				data[index + 2] = lastBlue;
			}
		}
	}
}
