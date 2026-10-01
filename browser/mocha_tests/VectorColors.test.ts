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

describe('VectorColors', function () {
	const luminanceOf = (color: string): number => {
		const rgb = cool.VectorColors.parse(color);
		nodeassert.ok(rgb, 'not a readable color: ' + color);
		return cool.VectorColors.wcagLuminance(rgb);
	};

	describe('reading and writing colors', function () {
		it('reads the channels of a hex color', function () {
			const color = cool.VectorColors.parse('#336699');
			nodeassert.ok(Math.abs(color.r - 0x33 / 255) < 1e-9);
			nodeassert.ok(Math.abs(color.g - 0x66 / 255) < 1e-9);
			nodeassert.ok(Math.abs(color.b - 0x99 / 255) < 1e-9);
		});

		it('has no channels to read for the automatic marker', function () {
			nodeassert.strictEqual(
				cool.VectorColors.parse(cool.VectorColors.AUTOMATIC),
				undefined,
			);
		});

		it('has no channels to read for a color with a bad digit', function () {
			nodeassert.strictEqual(cool.VectorColors.parse('#12345G'), undefined);
			nodeassert.strictEqual(cool.VectorColors.parse('#1234567'), undefined);
		});

		it('writes a color back as it was read', function () {
			nodeassert.strictEqual(
				cool.VectorColors.format(cool.VectorColors.parse('#0A1B2C')),
				'#0A1B2C',
			);
		});
	});

	describe('telling dark from light', function () {
		it('calls the dark document background dark', function () {
			nodeassert.ok(cool.VectorColors.isDark('#1C1C1C'));
		});

		it('calls the light document background light', function () {
			nodeassert.ok(!cool.VectorColors.isDark('#FFFFFF'));
		});

		it('puts the threshold at a luminance byte of 87', function () {
			// The engine compares a byte of WCAG luminance against 87,
			// and for grey that byte crosses 87 between 9E and 9F.
			const belowThreshold = '#9E9E9E';
			const aboveThreshold = '#9F9F9F';
			nodeassert.ok(Math.trunc(luminanceOf(belowThreshold) * 255) <= 87);
			nodeassert.ok(Math.trunc(luminanceOf(aboveThreshold) * 255) > 87);
			nodeassert.ok(cool.VectorColors.isDark(belowThreshold));
			nodeassert.ok(!cool.VectorColors.isDark(aboveThreshold));
		});

		it('has no answer for a color it cannot read', function () {
			nodeassert.ok(!cool.VectorColors.isDark(cool.VectorColors.AUTOMATIC));
		});
	});

	describe('mixing a fill with what lies behind it', function () {
		it('shows the fill alone when it lets nothing through', function () {
			nodeassert.strictEqual(
				cool.VectorColors.blend('#FF0000', '#0000FF', 0),
				'#FF0000',
			);
		});

		it('shows what lies behind in the share the fill lets through', function () {
			// A tenth of white over the dark background stays dark.
			const mixed = cool.VectorColors.blend('#FFFFFF', '#1C1C1C', 0.9);
			nodeassert.strictEqual(mixed, '#333333');
			nodeassert.ok(cool.VectorColors.isDark(mixed));
		});

		it('leaves a fill alone when a color cannot be read', function () {
			nodeassert.strictEqual(
				cool.VectorColors.blend('#FF0000', cool.VectorColors.AUTOMATIC, 0.5),
				'#FF0000',
			);
		});
	});

	describe('the light variant', function () {
		// The same expectations the engine's BColorTest makes: whatever
		// goes in, the result is a color that carries on a dark
		// background.
		const readable = (color: string): boolean => {
			const light = cool.VectorColors.getLightVariant(color);
			const luminance = luminanceOf(light);
			return luminance >= 0.5 - 1e-6 && luminance <= 1 + 1e-6;
		};

		it('lifts dark colors above half luminance', function () {
			for (const color of [
				'#000000',
				'#1A1A1A',
				'#331A1A',
				'#000080',
				'#4D1A33',
			])
				nodeassert.ok(readable(color), color);
		});

		it('keeps light colors above half luminance', function () {
			for (const color of ['#E6E6E6', '#FFFFFF', '#CCCCB3', '#B3CCE6'])
				nodeassert.ok(readable(color), color);
		});

		it('keeps the hue of a saturated color', function () {
			// Dark blue comes back a brighter blue, not white: blue
			// stays the largest channel.
			const light = cool.VectorColors.parse(
				cool.VectorColors.getLightVariant('#000080'),
			);
			nodeassert.ok(light.b > light.r);
			nodeassert.ok(light.b > light.g);
		});

		it('leaves a color it cannot read alone', function () {
			nodeassert.strictEqual(
				cool.VectorColors.getLightVariant(cool.VectorColors.AUTOMATIC),
				cool.VectorColors.AUTOMATIC,
			);
		});
	});
});
