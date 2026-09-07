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

describe('VectorPrimitiveRenderer gradients', function () {
	const assert = require('assert');

	let originalPath2D: any;

	before(function () {
		originalPath2D = (globalThis as any).Path2D;
		(globalThis as any).Path2D = Path2DRecorder;
	});

	after(function () {
		(globalThis as any).Path2D = originalPath2D;
	});

	function render(primitive: any, recorder?: CanvasRecorder): CanvasRecorder {
		const context = recorder ?? new CanvasRecorder(200, 200);
		new cool.VectorPrimitiveRenderer().renderPrimitive(
			context as any,
			primitive,
		);
		return context;
	}

	const blackToWhite = [
		{ offset: 0, color: '#000000' },
		{ offset: 1, color: '#ffffff' },
	];

	it('runs a linear ramp from its first stop to its last', function () {
		const context = render({
			type: 'fillGradient',
			outputRange: [0, 0, 100, 100],
			gradient: { style: 'linear', colorStops: blackToWhite },
		});

		assert.strictEqual(context.gradients.length, 1);
		const stops = context.gradients[0].sortedStops();
		assert.strictEqual(stops[0].color, 'rgba(0,0,0,1)');
		assert.strictEqual(stops[stops.length - 1].color, 'rgba(255,255,255,1)');
	});

	it('puts the first color of an axial ramp in the middle and the last at both edges', function () {
		// The engine runs an axial ramp outwards from the center.
		const context = render({
			type: 'fillGradient',
			outputRange: [0, 0, 100, 100],
			gradient: { style: 'axial', colorStops: blackToWhite },
		});

		const gradient = context.gradients[0];
		assert.strictEqual(gradient.kind, 'linear');
		const stops = gradient.sortedStops();
		assert.strictEqual(stops[0].color, 'rgba(255,255,255,1)');
		assert.strictEqual(stops[stops.length - 1].color, 'rgba(255,255,255,1)');
		const middle = stops.find(
			(stop: any) => Math.abs(stop.offset - 0.5) < 1e-9,
		);
		assert.ok(middle, 'a stop sits at the center of the ramp');
		assert.strictEqual(middle.color, 'rgba(0,0,0,1)');
	});

	it('puts the last color of a radial ramp at the center', function () {
		const context = render({
			type: 'fillGradient',
			outputRange: [0, 0, 100, 100],
			gradient: { style: 'radial', colorStops: blackToWhite },
		});

		const gradient = context.gradients[0];
		assert.strictEqual(gradient.kind, 'radial');
		// The radius runs out from the center, so offset zero is the
		// rim.
		const stops = gradient.sortedStops();
		assert.strictEqual(stops[0].color, 'rgba(255,255,255,1)');
		assert.strictEqual(stops[stops.length - 1].color, 'rgba(0,0,0,1)');
	});

	it('keeps the rim of an elliptical ramp on the edge of its grown box', function () {
		const context = render({
			type: 'fillGradient',
			outputRange: [0, 0, 200, 100],
			gradient: { style: 'elliptical', colorStops: blackToWhite },
		});

		// The ring is the unit circle under the transforms the fill
		// applied, so composing them says where its rim lands. The
		// engine grows the box by root two about its center and fits
		// the outer ring to that, as wide as it is tall in those terms.
		let matrix = cool.Matrix2D.IDENTITY;
		for (const call of context.calls) {
			if (call.method === 'transform')
				matrix = cool.Matrix2D.fromArray(call.args).then(matrix);
			else if (call.method === 'scale')
				matrix = cool.Matrix2D.IDENTITY.scale(call.args[0], call.args[1]).then(
					matrix,
				);
		}
		const right = matrix.apply(1, 0);
		const bottom = matrix.apply(0, 1);
		assert.ok(
			Math.abs(right.x - (100 + 100 * Math.SQRT2)) < 1e-9,
			'rim on the right edge, got ' + right.x,
		);
		assert.ok(
			Math.abs(bottom.y - (50 + 50 * Math.SQRT2)) < 1e-9,
			'rim on the bottom edge, got ' + bottom.y,
		);
	});

	it('breaks a stepped ramp into flat bands', function () {
		const context = render({
			type: 'fillGradient',
			outputRange: [0, 0, 100, 100],
			gradient: { style: 'linear', steps: 4, colorStops: blackToWhite },
		});

		const stops = context.gradients[0].sortedStops();
		// A band is a pair of stops holding one color, so the color
		// changes only at a boundary.
		assert.strictEqual(stops.length, 8);
		assert.strictEqual(stops[0].color, 'rgba(0,0,0,1)');
		assert.strictEqual(stops[1].color, 'rgba(0,0,0,1)');
		assert.strictEqual(stops[stops.length - 1].color, 'rgba(255,255,255,1)');
	});

	it('bands each pair of neighboring stops on its own', function () {
		// Three stops make two pairs, so the same step count gives
		// twice the bands.
		const context = render({
			type: 'fillGradient',
			outputRange: [0, 0, 100, 100],
			gradient: {
				style: 'linear',
				steps: 3,
				colorStops: [
					{ offset: 0, color: '#000000' },
					{ offset: 0.5, color: '#ff0000' },
					{ offset: 1, color: '#ffffff' },
				],
			},
		});

		assert.strictEqual(context.gradients[0].stops.length, 12);
	});

	it('asks for fewer bands between two nearly equal colors', function () {
		// The engine caps the bands by the color distance.
		const context = render({
			type: 'fillGradient',
			outputRange: [0, 0, 100, 100],
			gradient: {
				style: 'linear',
				steps: 64,
				colorStops: [
					{ offset: 0, color: '#000000' },
					{ offset: 1, color: '#020202' },
				],
			},
		});

		assert.ok(
			context.gradients[0].stops.length < 10,
			'the band count follows the color distance',
		);
	});

	it('takes the transparency of an alpha ramp from each stop luminance', function () {
		// The polygon keeps its color, black opaque, white clear.
		const context = render({
			type: 'polyPolygonAlphaGradient',
			color: '#3366cc',
			path: 'M 0 0 L 100 0 L 100 100 Z',
			bounds: [0, 0, 100, 100],
			alphaGradient: { style: 'linear', colorStops: blackToWhite },
		});

		const stops = context.gradients[0].sortedStops();
		assert.strictEqual(stops[0].color, 'rgba(51,102,204,1)');
		assert.strictEqual(stops[stops.length - 1].color, 'rgba(51,102,204,0)');
	});

	// The offsets at which a ramp read inwards from both ends of a
	// line places its last color, as a sorted list.
	function lastColorOffsets(gradient: any): number[] {
		return gradient
			.sortedStops()
			.filter((stop: any) => stop.color === 'rgba(255,255,255,1)')
			.map((stop: any) => stop.offset);
	}

	it('fills a boxed ramp from the first color at the edge to the last at the center', function () {
		const context = render({
			type: 'fillGradient',
			outputRange: [0, 0, 100, 100],
			gradient: { style: 'square', colorStops: blackToWhite },
		});

		// One ramp runs top to bottom and one side to side. Each reads
		// inwards from both edges, so the first color sits at both ends
		// and the last one in the middle.
		assert.strictEqual(context.gradients.length, 2);
		for (const gradient of context.gradients) {
			const stops = gradient.sortedStops();
			assert.strictEqual(stops[0].color, 'rgba(0,0,0,1)');
			assert.strictEqual(stops[stops.length - 1].color, 'rgba(0,0,0,1)');
			assert.deepStrictEqual(lastColorOffsets(gradient), [0.5, 0.5]);
		}
	});

	it('keeps the last color on a segment across the center of a wide box', function () {
		const context = render({
			type: 'fillGradient',
			outputRange: [0, 0, 200, 100],
			gradient: { style: 'rect', colorStops: blackToWhite },
		});

		// Top to bottom the ramp still meets in the middle. Side to side
		// the box is twice as wide, so the last color holds the middle
		// half and the ramp only runs over the outer quarters.
		assert.strictEqual(context.gradients.length, 2);
		const [vertical, horizontal] = context.gradients;
		assert.deepStrictEqual(lastColorOffsets(vertical), [0.5, 0.5]);
		assert.deepStrictEqual(lastColorOffsets(horizontal), [0.25, 0.75]);
	});

	it('leaves a gradient with no stops undrawn', function () {
		const context = render({
			type: 'fillGradient',
			outputRange: [0, 0, 100, 100],
			gradient: { style: 'linear', colorStops: [] },
		});

		assert.strictEqual(context.countOf('fillRect'), 0);
	});
});
