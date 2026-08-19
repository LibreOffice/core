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

describe('VectorPrimitiveRenderer pattern fill', function () {
	const assert = require('assert');

	let originalPath2D: any;

	before(function () {
		originalPath2D = (globalThis as any).Path2D;
		(globalThis as any).Path2D = Path2DRecorder;
	});

	after(function () {
		(globalThis as any).Path2D = originalPath2D;
	});

	// Its color tells a tile apart from anything else drawn.
	const child = {
		type: 'polyPolygonColor',
		color: '#00ff00',
		path: 'M 10 10 L 90 10 L 90 90 Z',
	};

	function render(primitive: any, recorder?: CanvasRecorder): CanvasRecorder {
		const context = recorder ?? new CanvasRecorder(200, 200);
		new cool.VectorPrimitiveRenderer().renderPrimitive(
			context as any,
			primitive,
		);
		return context;
	}

	it('draws the children once per tile, clipped to it', function () {
		const context = render({
			type: 'patternFill',
			path: 'M 0 0 L 100 0 L 100 100 L 0 100 Z',
			bounds: [0, 0, 100, 100],
			// Quarter-size tiles, four rows of four.
			referenceRange: [0, 0, 0.25, 0.25],
			children: [child],
		});

		// Every other row starts one tile back, as the engine lays
		// them out, so two of the rows hold a fifth tile that lies
		// left of the square and is clipped away.
		assert.strictEqual(context.countOf('fill'), 18);
		// The polygon, and then each tile.
		assert.strictEqual(context.countOf('clip'), 1 + 18);
	});

	it('skips a pattern whose tiles are under a pixel', function () {
		const context = new CanvasRecorder(200, 200);
		// A tile this small shows nothing and there would be one per
		// pixel.
		new cool.VectorPrimitiveRenderer().renderPrimitive(
			context as any,
			{
				type: 'patternFill',
				path: 'M 0 0 L 1 0 L 1 1 Z',
				bounds: [0, 0, 1, 1],
				referenceRange: [0, 0, 0.001, 0.001],
				children: [child],
			} as any,
		);

		assert.strictEqual(context.countOf('fill'), 0);
	});
});
