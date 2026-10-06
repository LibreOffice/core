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

describe('VectorColorModifiers', function () {
	const assert = require('assert');

	// The modified color, each channel from 0 to 1.
	function modify(
		modifier: cool.ColorModifierAttribute,
		color: number[],
	): number[] {
		const change = cool.VectorColorModifiers.colorChange(modifier);
		assert.ok(change, 'the modifier changes colors');
		const result = color.slice();
		change!(result);
		return result;
	}

	function assertColorNear(
		actual: number[],
		expected: number[],
		what: string,
	): void {
		for (let channel = 0; channel < 3; channel++) {
			assert.ok(
				Math.abs(actual[channel] - expected[channel]) < 1e-9,
				what +
					': expected ' +
					JSON.stringify(expected) +
					' but got ' +
					JSON.stringify(actual),
			);
		}
	}

	it('grays with the luminance weights of the engine', function () {
		assertColorNear(
			modify({ modifier: 'gray' }, [1, 0, 0]),
			[77 / 256, 77 / 256, 77 / 256],
			'red',
		);
	});

	it('inverts each channel', function () {
		assertColorNear(
			modify({ modifier: 'invert' }, [1, 0.25, 0]),
			[0, 0.75, 1],
			'inverted',
		);
	});

	it('turns the luminance into a gray of one minus it', function () {
		const level = 1 - 0.7154;
		assertColorNear(
			modify({ modifier: 'luminance_to_alpha' }, [0, 1, 0]),
			[level, level, level],
			'green',
		);
	});

	it('replaces every color with the one given', function () {
		assertColorNear(
			modify({ modifier: 'replace', color: '#00ff00' }, [1, 0, 0]),
			[0, 1, 0],
			'replaced',
		);
	});

	it('keeps the given part of the original when it interpolates', function () {
		// 0.25 keeps a quarter of the red and takes three quarters blue.
		assertColorNear(
			modify(
				{ modifier: 'interpolate', color: '#0000ff', value: 0.25 },
				[1, 0, 0],
			),
			[0.25, 0, 0.75],
			'interpolated',
		);
	});

	it('keeps the interpolation between the two colors for a value out of range', function () {
		// As in the engine, above 1 is all original and below 0 all color.
		assertColorNear(
			modify(
				{ modifier: 'interpolate', color: '#0000ff', value: 1.5 },
				[1, 0, 0],
			),
			[1, 0, 0],
			'above one',
		);
		assertColorNear(
			modify(
				{ modifier: 'interpolate', color: '#0000ff', value: -0.5 },
				[1, 0, 0],
			),
			[0, 0, 1],
			'below zero',
		);
	});

	it('lets a saturation go past the channel range', function () {
		// As in the engine, doubled saturation goes past one unclamped.
		const value = 2;
		const matrix = [
			0.213 + 0.787 * value,
			0.715 - 0.715 * value,
			0.072 - 0.072 * value,
			0.213 - 0.213 * value,
			0.715 + 0.285 * value,
			0.072 - 0.072 * value,
			0.213 - 0.213 * value,
			0.715 - 0.715 * value,
			0.072 + 0.928 * value,
		];
		const result = modify({ modifier: 'saturate', matrix: matrix }, [1, 0, 0]);
		assert.ok(result[0] > 1, 'red goes past one');
		assert.ok(result[1] < 0, 'green goes below zero');
	});

	it('keeps a hue rotation in the channel range', function () {
		const matrix = [2, 0, 0, -1, 0, 0, 0, 0, 1];
		assertColorNear(
			modify({ modifier: 'hueRotate', matrix: matrix }, [1, 0, 0.5]),
			[1, 0, 0.5],
			'clamped',
		);
	});

	it('weighs the channels and a constant through a color matrix', function () {
		// Red and green swap. Alpha and constant each add a quarter to blue.
		const matrix = [
			0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0.25, 0.25, 0, 0, 0, 1, 0,
		];
		assertColorNear(
			modify({ modifier: 'matrix', matrix: matrix }, [1, 0.5, 0]),
			[0.5, 1, 0.5],
			'matrix',
		);
	});

	it('splits black from white at the luminance threshold', function () {
		const modifier = { modifier: 'black_and_white', value: 0.5 };
		assertColorNear(modify(modifier, [1, 0, 0]), [0, 0, 0], 'red is dark');
		assertColorNear(modify(modifier, [0, 1, 0]), [1, 1, 1], 'green is light');
	});

	it('raises each channel to one over the gamma', function () {
		assertColorNear(
			modify({ modifier: 'gamma', value: 2 }, [0.25, 1, 0]),
			[0.5, 1, 0],
			'gamma 2',
		);
	});

	it('leaves the colors alone for a gamma of one', function () {
		assert.strictEqual(
			cool.VectorColorModifiers.colorChange({ modifier: 'gamma', value: 1 }),
			null,
		);
	});

	it('lifts black to a light gray for the watermark adjustment', function () {
		// The watermark, luminance 0.5 and contrast -0.7, as slope and offset.
		const watermark = {
			modifier: 'RGBLuminanceContrast',
			luminance: 0.5,
			contrast: -0.7,
		};
		const slope = (128 - 0.7 * 127) / 128;
		const offset = 0.5 + (128 - slope * 128) / 255;
		assertColorNear(
			modify(watermark, [0, 0, 0]),
			[offset, offset, offset],
			'black',
		);
		assertColorNear(modify(watermark, [1, 1, 1]), [1, 1, 1], 'white');
	});

	it('spreads the channels apart for a raised contrast', function () {
		const contrast = { modifier: 'RGBLuminanceContrast', contrast: 0.5 };
		const slope = 128 / (128 - 0.5 * 127);
		const offset = (128 - slope * 128) / 255;
		const result = modify(contrast, [0.25, 0.75, 0.5]);
		assertColorNear(
			result,
			[0.25 * slope + offset, 0.75 * slope + offset, 0.5 * slope + offset],
			'contrast',
		);
	});

	it('moves each channel by its own amount', function () {
		assertColorNear(
			modify(
				{ modifier: 'RGBLuminanceContrast', red: 0.25, green: -0.25 },
				[0.5, 0.5, 0.5],
			),
			[0.75, 0.25, 0.5],
			'channels',
		);
	});

	it('leaves the colors alone for an adjustment of nothing', function () {
		assert.strictEqual(
			cool.VectorColorModifiers.colorChange({
				modifier: 'RGBLuminanceContrast',
			}),
			null,
		);
	});

	it('leaves the colors alone for a random mix', function () {
		assert.strictEqual(
			cool.VectorColorModifiers.changesColors({ modifier: 'randomize' }),
			false,
		);
	});

	it('applies modifiers to pixels first to last and keeps their alpha', function () {
		// Replace with red, then invert, gives cyan. The reverse gives red.
		const pixels = new Uint8ClampedArray([0, 0, 255, 128, 10, 20, 30, 255]);
		cool.VectorColorModifiers.applyToPixels(pixels, [
			{ modifier: 'replace', color: '#ff0000' },
			{ modifier: 'invert' },
		]);
		assert.deepStrictEqual(
			Array.from(pixels),
			[0, 255, 255, 128, 0, 255, 255, 255],
		);
	});

	it('leaves a fully transparent pixel alone', function () {
		const pixels = new Uint8ClampedArray([10, 20, 30, 0]);
		cool.VectorColorModifiers.applyToPixels(pixels, [{ modifier: 'invert' }]);
		assert.deepStrictEqual(Array.from(pixels), [10, 20, 30, 0]);
	});
});
