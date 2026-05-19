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
