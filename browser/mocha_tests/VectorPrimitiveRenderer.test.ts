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

describe('VectorPrimitiveRenderer', function () {
	// The renderer constructs new Path2D. Use Path2DRecorder
	// as a stand-in to capture the path string.
	let originalPath2D: any;
	before(function () {
		originalPath2D = (globalThis as any).Path2D;
		(globalThis as any).Path2D = Path2DRecorder;
	});
	after(function () {
		(globalThis as any).Path2D = originalPath2D;
	});

	it('fills the whole canvas for backgroundcolor', function () {
		const recorder = new CanvasRecorder(200, 150);
		const renderer = new cool.VectorPrimitiveRenderer();
		const primitive = {
			type: 'backgroundcolor',
			color: '#ff0000',
		};

		renderer.renderPrimitive(recorder as any, primitive);

		const fillRect = recorder.findCall('fillRect');
		nodeassert.ok(fillRect, 'fillRect not called');
		nodeassert.deepStrictEqual(fillRect.args, [0, 0, 200, 150]);
		nodeassert.strictEqual(recorder.properties.fillStyle, '#ff0000');
		// Save and restore must bracket the fill so the page transform is
		// not permanently reset.
		nodeassert.ok(recorder.findCall('save'), 'save not called');
		nodeassert.ok(recorder.findCall('restore'), 'restore not called');
	});

	it('applies transparency via globalAlpha for backgroundcolor', function () {
		const recorder = new CanvasRecorder();
		const renderer = new cool.VectorPrimitiveRenderer();
		const primitive = {
			type: 'backgroundcolor',
			color: '#abcdef',
			transparency: 0.25,
		};

		renderer.renderPrimitive(recorder as any, primitive);

		nodeassert.strictEqual(recorder.properties.fillStyle, '#abcdef');
		nodeassert.strictEqual(recorder.properties.globalAlpha, 0.75);
	});

	it('skips backgroundcolor with no color set', function () {
		const recorder = new CanvasRecorder();
		const renderer = new cool.VectorPrimitiveRenderer();
		const primitive = {
			type: 'backgroundcolor',
		};

		renderer.renderPrimitive(recorder as any, primitive);

		nodeassert.strictEqual(recorder.countOf('fillRect'), 0);
	});

	it('fills a Path2D for polyPolygonColor', function () {
		const recorder = new CanvasRecorder();
		const renderer = new cool.VectorPrimitiveRenderer();
		const primitive = {
			type: 'polyPolygonColor',
			color: '#00ff00',
			path: 'M 0 0 L 10 0 L 10 10 Z',
		};

		renderer.renderPrimitive(recorder as any, primitive);

		const fillCall = recorder.findCall('fill');
		nodeassert.ok(fillCall, 'fill not called');
		nodeassert.strictEqual(recorder.properties.fillStyle, '#00ff00');
		nodeassert.strictEqual(fillCall.args.length, 1);
		nodeassert.strictEqual(
			(fillCall.args[0] as Path2DRecorder).path,
			'M 0 0 L 10 0 L 10 10 Z',
		);
	});

	it('skips polyPolygonColor with missing path or color', function () {
		const recorder = new CanvasRecorder();
		const renderer = new cool.VectorPrimitiveRenderer();
		const noPath = { type: 'polyPolygonColor', color: '#fff' };
		const noColor = { type: 'polyPolygonColor', path: 'M 0 0' };

		renderer.renderPrimitive(recorder as any, noPath);
		renderer.renderPrimitive(recorder as any, noColor);

		nodeassert.strictEqual(recorder.countOf('fill'), 0);
	});

	it('recurses through children', function () {
		const recorder = new CanvasRecorder();
		const renderer = new cool.VectorPrimitiveRenderer();
		// A "unknown" type whose children include backgroundcolor.
		// The renderer should descend into children.
		const primitive = {
			type: 'unknown-xyz',
			children: [
				{
					type: 'backgroundcolor',
					color: '#000',
				},
			],
		};

		renderer.renderPrimitive(recorder as any, primitive);

		nodeassert.ok(
			recorder.findCall('fillRect'),
			'fillRect should fire from nested child',
		);
	});

	it('ignores unknown primitive types and primitives without type', function () {
		const recorder = new CanvasRecorder();
		const renderer = new cool.VectorPrimitiveRenderer();

		renderer.renderPrimitive(recorder as any, { type: 'unknown-xyz' });
		renderer.renderPrimitive(recorder as any, {});

		nodeassert.strictEqual(recorder.calls.length, 0, 'no canvas methods should be called');
		nodeassert.deepStrictEqual(recorder.properties, {}, 'no canvas properties should be set');
	});
});
