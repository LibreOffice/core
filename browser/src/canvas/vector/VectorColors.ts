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
	/// A color with its channels in [0, 1].
	export interface VectorRgb {
		r: number;
		g: number;
		b: number;
	}

	/// Color arithmetic for the vector renderer. The results follow the
	/// engine's basegfx and tools helpers, so the same document reads
	/// the same whether the engine painted it or the client did.
	export namespace VectorColors {
		/// The color of text that carries no color of its own. It is
		/// chosen for contrast against the surface under the text.
		export const AUTOMATIC = 'auto';

		/// The document background before the engine has named one.
		export const DEFAULT_DOCUMENT_BACKGROUND = '#FFFFFF';

		/// The document background of the view this client paints in, as
		/// the engine last reported it for the view.
		export function documentBackground(): string {
			const color = app.sectionContainer
				? app.sectionContainer.getDocumentBackgroundColor().trim()
				: '';
			return parse(color) ? color : DEFAULT_DOCUMENT_BACKGROUND;
		}

		const RGB_HEX = /^#[0-9a-fA-F]{6}$/;

		/// Read a "#RRGGBB" color. Anything else, the automatic marker
		/// included, has no channels to read and gives undefined.
		export function parse(color: string | undefined): VectorRgb | undefined {
			if (!color || !RGB_HEX.test(color)) return undefined;
			const value = Number.parseInt(color.substring(1), 16);
			return {
				r: ((value >> 16) & 0xff) / 255,
				g: ((value >> 8) & 0xff) / 255,
				b: (value & 0xff) / 255,
			};
		}

		export function format(color: VectorRgb): string {
			const channel = (value: number): string => {
				const byte = Math.round(clamp(value) * 255);
				const digits = byte.toString(16).toUpperCase();
				return byte < 16 ? '0' + digits : digits;
			};
			return '#' + channel(color.r) + channel(color.g) + channel(color.b);
		}

		/// The color a fill shows over what lies behind it, when the fill
		/// lets a share of that show through: transparency 0 is the fill
		/// alone and 1 is what lies behind alone. A color that cannot be
		/// read leaves the fill as it was given.
		export function blend(
			fill: string,
			behind: string,
			transparency: number,
		): string {
			const front = parse(fill);
			const back = parse(behind);
			if (!front || !back) return fill;
			const opacity = 1 - transparency;
			return format({
				r: front.r * opacity + back.r * transparency,
				g: front.g * opacity + back.g * transparency,
				b: front.b * opacity + back.b * transparency,
			});
		}

		function clamp(value: number): number {
			return Math.min(1, Math.max(0, value));
		}

		function srgbToLinear(value: number): number {
			return value < 0.04045
				? value / 12.92
				: Math.pow((value + 0.055) / 1.055, 2.4);
		}

		function linearToSrgb(value: number): number {
			return value <= 0.0031308
				? value * 12.92
				: 1.055 * Math.pow(value, 1 / 2.4) - 0.055;
		}

		/// Relative luminance following WCAG 2.1, in [0, 1].
		export function wcagLuminance(color: VectorRgb): number {
			return (
				srgbToLinear(clamp(color.r)) * 0.2126 +
				srgbToLinear(clamp(color.g)) * 0.7152 +
				srgbToLinear(clamp(color.b)) * 0.0722
			);
		}

		/// True for a color dark enough to carry light text. The
		/// thresholds are the engine's: a byte of WCAG luminance at 87
		/// or below, and for the default shape fill the weighted
		/// luminance at 62 or below.
		export function isDark(color: string): boolean {
			const rgb = parse(color);
			if (!rgb) return false;

			const red = Math.round(rgb.r * 255);
			const green = Math.round(rgb.g * 255);
			const blue = Math.round(rgb.b * 255);

			if (red === 0x72 && green === 0x9f && blue === 0xcf) {
				const weighted = (blue * 29 + green * 151 + red * 76) >> 8;
				return weighted <= 62;
			}

			return Math.trunc(wcagLuminance(rgb) * 255) <= 87;
		}

		/// Map a color to a lighter variant so it stays readable on a
		/// dark background. A dark input comes back bright and a bright
		/// input comes back a little darker, both above half luminance.
		/// Saturated colors keep their hue instead of washing out.
		export function getLightVariant(color: string): string {
			const rgb = parse(color);
			if (!rgb) return color;

			const red = clamp(rgb.r);
			const green = clamp(rgb.g);
			const blue = clamp(rgb.b);

			// Saturation in sRGB space, which sets the luminance cap.
			const maxChannel = Math.max(red, green, blue);
			const minChannel = Math.min(red, green, blue);
			const saturation =
				maxChannel < 0.001 ? 0 : (maxChannel - minChannel) / maxChannel;

			const linear = {
				r: srgbToLinear(red),
				g: srgbToLinear(green),
				b: srgbToLinear(blue),
			};
			const luminance = wcagLuminance({ r: red, g: green, b: blue });

			const maximumOutput = 1.0;
			const minimumOutput = 0.5;
			const cap = maximumOutput - saturation * (maximumOutput - minimumOutput);

			// Flip the input luminance into the range between the cap
			// and the floor.
			const target = cap - luminance * (cap - minimumOutput);

			let r: number;
			let g: number;
			let b: number;
			if (luminance < 0.001) {
				r = g = b = target;
			} else {
				const ratio = target / luminance;
				r = linear.r * ratio;
				g = linear.g * ratio;
				b = linear.b * ratio;
			}

			// A channel above 1.0 comes back by desaturating just
			// enough, which keeps the hue.
			const brightest = Math.max(r, g, b);
			if (brightest > 1.0) {
				const desaturate = (1.0 - target) / (brightest - target);
				r = target + (r - target) * desaturate;
				g = target + (g - target) * desaturate;
				b = target + (b - target) * desaturate;
			}

			return format({
				r: linearToSrgb(r),
				g: linearToSrgb(g),
				b: linearToSrgb(b),
			});
		}
	}
}
