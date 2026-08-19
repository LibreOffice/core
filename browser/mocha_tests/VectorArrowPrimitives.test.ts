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

describe('VectorPrimitiveRenderer arrow heads', function () {
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

	// The engine sends the line, cut back under the head, then the head.
	const shaft = {
		type: 'polygonStroke',
		path: 'M 0 0 L 85 0',
		line: { color: '#123456', width: 4 },
	};
	const head = {
		type: 'polyPolygonColor',
		color: '#123456',
		path: 'M 100 0 L 80 10 L 80 -10 Z',
	};

	it('draws the line and then the heads it holds', function () {
		const context = render({
			type: 'polygonStrokeArrow',
			children: [{ type: 'group', children: [shaft, head] }],
		});

		const drawn = context.calls
			.filter((call) => call.method === 'stroke' || call.method === 'fill')
			.map((call) => call.method);
		assert.deepStrictEqual(drawn, ['stroke', 'fill']);
		const stroke = context.findCall('stroke');
		assert.strictEqual(stroke.args[0].path, 'M 0 0 L 85 0');
		const fill = context.findCall('fill');
		assert.strictEqual(fill.args[0].path, 'M 100 0 L 80 10 L 80 -10 Z');
		assert.strictEqual(fill.properties.fillStyle, '#123456');
	});

	it('draws the line and the head that the engine sends', function () {
		const primitive = loadVectorRenderingReference('testPolygonStrokeArrow')
			.primitives[0];
		assert.strictEqual(primitive.type, 'polygonStrokeArrow');

		const context = render(primitive);

		const drawn = context.calls
			.filter((call) => call.method === 'stroke' || call.method === 'fill')
			.map((call) => call.method);
		assert.deepStrictEqual(drawn, ['stroke', 'fill']);
		const line = primitive.children[0].children[0];
		const head = primitive.children[0].children[1];
		assert.strictEqual(context.findCall('stroke').args[0].path, line.path);
		assert.strictEqual(context.findCall('fill').args[0].path, head.path);
		assert.strictEqual(
			context.findCall('fill').properties.fillStyle,
			head.color,
		);
	});
});
