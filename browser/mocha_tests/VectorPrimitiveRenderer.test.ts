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
	// The renderer constructs new Path2D. Use Path2DRecorder as a
	// stand-in to capture the path string.
	let originalPath2D: any;

	before(function () {
		originalPath2D = (globalThis as any).Path2D;
		(globalThis as any).Path2D = Path2DRecorder;
	});

	after(function () {
		(globalThis as any).Path2D = originalPath2D;
	});

	// Each fixture is a single primitive built with the drawinglayer
	// primitive constructor, so we can test the primitive in isolation.
	describe('Primitive references', function () {
		it('fills the whole canvas for backgroundcolor', function () {
			const primitive = loadVectorRenderingReference('testBackgroundColor').primitives[0];
			nodeassert.strictEqual(primitive.type, 'backgroundcolor');
			nodeassert.strictEqual(typeof primitive.color, 'string');

			const recorder = new CanvasRecorder(200, 150);
			const renderer = new cool.VectorPrimitiveRenderer();
			renderer.renderPrimitive(recorder as any, primitive);

			const fillRect = recorder.findCall('fillRect');
			nodeassert.ok(fillRect, 'fillRect not called');
			nodeassert.deepStrictEqual(fillRect.args, [0, 0, 200, 150]);
			nodeassert.strictEqual(recorder.properties.fillStyle, primitive.color);
			// The background fill must not leak its canvas state to
			// anything drawn after it.
			nodeassert.ok(recorder.findCall('save'), 'save not called');
			nodeassert.ok(recorder.findCall('restore'), 'restore not called');
		});

		it('respects backgroundcolor transparency', function () {
			const primitive = loadVectorRenderingReference(
				'testBackgroundColorTransparent',
			).primitives[0];
			nodeassert.strictEqual(primitive.type, 'backgroundcolor');
			nodeassert.strictEqual(typeof primitive.transparency, 'number');
			nodeassert.ok(primitive.transparency > 0, 'fixture must be partly transparent');

			const recorder = new CanvasRecorder();
			const renderer = new cool.VectorPrimitiveRenderer();
			renderer.renderPrimitive(recorder as any, primitive);

			nodeassert.strictEqual(recorder.properties.fillStyle, primitive.color);
			nodeassert.strictEqual(
				recorder.properties.globalAlpha,
				1 - primitive.transparency,
			);
		});

		it('renders polyPolygonColor with its fill colour', function () {
			const primitive = loadVectorRenderingReference('testPolyPolygonColor').primitives[0];
			nodeassert.strictEqual(primitive.type, 'polyPolygonColor');
			nodeassert.strictEqual(typeof primitive.color, 'string');
			nodeassert.strictEqual(typeof primitive.path, 'string');

			const recorder = new CanvasRecorder();
			const renderer = new cool.VectorPrimitiveRenderer();
			renderer.renderPrimitive(recorder as any, primitive);

			const fillCall = recorder.findCall('fill');
			nodeassert.ok(fillCall, 'fill not called');
			nodeassert.strictEqual(recorder.properties.fillStyle, primitive.color);
			nodeassert.strictEqual(fillCall.args.length, 1);
			nodeassert.strictEqual(
				(fillCall.args[0] as Path2DRecorder).path,
				primitive.path,
			);
		});
	});

	// Fixtures from documents. Each fixture is a full reply built
	// by the same pipeline a real Impress document would go through,
	// so the renderer is exercised against a realistic primitive
	// tree rather than a single isolated primitive.
	describe('Document references', function () {
		it('renders the filled-rectangle slide from its document reference', function () {
			const primitiveTree = loadVectorRenderingReference('testSingleRectangle');

			nodeassert.strictEqual(primitiveTree.type, 'vectortile');
			nodeassert.strictEqual(typeof primitiveTree.slideWidth, 'number');
			nodeassert.strictEqual(typeof primitiveTree.slideHeight, 'number');

			const recorder = new CanvasRecorder(
				primitiveTree.slideWidth,
				primitiveTree.slideHeight,
			);
			const renderer = new cool.VectorPrimitiveRenderer();

			for (const primitive of primitiveTree.masterPage.primitives)
				renderer.renderPrimitive(recorder as any, primitive);

			for (const object of primitiveTree.objects)
				for (const primitive of object.primitives)
					renderer.renderPrimitive(recorder as any, primitive);

			// backgroundcolor fills the whole canvas at the white page color
			const fillRect = recorder.findCall('fillRect');
			nodeassert.ok(fillRect, 'backgroundcolor primitive should fillRect');
			nodeassert.deepStrictEqual(fillRect.args, [
				0,
				0,
				primitiveTree.slideWidth,
				primitiveTree.slideHeight,
			]);

			// 2x page-fill from polyPolygonColor
			const fills = recorder.callsOf('fill');
			nodeassert.ok(fills.length >= 2, 'expected at least two fills');

			// The blue rectangle renders with its declared colour.
			nodeassert.strictEqual(recorder.properties.fillStyle, '#4472c4');

			// Path2D should match the path string from the reference
			// for the blue fill.
			const lastFill = fills[fills.length - 1];
			nodeassert.ok(
				(lastFill.args[0] as Path2DRecorder).path.startsWith('m4251'),
				'last fill path does not match the reference',
			);
		});
	});
});
