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

describe('VectorPrimitiveRenderer hatches', function () {
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

	it('strokes one set of lines for a single hatch', function () {
		const context = render({
			type: 'fillHatch',
			outputRange: [0, 0, 100, 100],
			backgroundColor: '#ffffff',
			hatch: { style: 'single', distance: 10, angle: 0, color: '#ff0000' },
		});

		const stroke = context.findCall('stroke');
		assert.ok(stroke, 'the lines are stroked');
		assert.strictEqual(stroke.properties.strokeStyle, '#ff0000');
		// Horizontal lines, so every line keeps one y.
		const moves = context.callsOf('moveTo');
		assert.ok(moves.length > 1);
		for (const move of moves) {
			const line = context.callsOf('lineTo')[moves.indexOf(move)];
			assert.ok(Math.abs(move.args[1] - line.args[1]) < 1e-9);
		}
	});

	it('adds a second set at a right angle for a double hatch', function () {
		const single = render({
			type: 'fillHatch',
			outputRange: [0, 0, 100, 100],
			hatch: { style: 'single', distance: 10, angle: 0 },
		});
		const double = render({
			type: 'fillHatch',
			outputRange: [0, 0, 100, 100],
			hatch: { style: 'double', distance: 10, angle: 0 },
		});

		assert.strictEqual(double.countOf('moveTo'), single.countOf('moveTo') * 2);
	});

	it('paints the background only when the hatch asks for it', function () {
		const without = render({
			type: 'fillHatch',
			outputRange: [0, 0, 100, 100],
			backgroundColor: '#00ff00',
			hatch: { style: 'single', distance: 10 },
		});
		assert.strictEqual(without.countOf('fillRect'), 0);

		const with_ = render({
			type: 'fillHatch',
			outputRange: [0, 0, 100, 100],
			backgroundColor: '#00ff00',
			hatch: { style: 'single', distance: 10, fillBackground: true },
		});
		const fill = with_.findCall('fillRect');
		assert.ok(fill);
		assert.strictEqual(fill.properties.fillStyle, '#00ff00');
	});

	it('draws nothing for a hatch with no spacing', function () {
		const context = render({
			type: 'fillHatch',
			outputRange: [0, 0, 100, 100],
			hatch: { style: 'single', distance: 0 },
		});
		assert.strictEqual(context.countOf('stroke'), 0);
	});
});
