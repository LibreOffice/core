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

describe('VectorPrimitiveRenderer effects', function () {
	const assert = require('assert');

	let originalPath2D: any;

	before(function () {
		originalPath2D = (globalThis as any).Path2D;
		(globalThis as any).Path2D = Path2DRecorder;
	});

	after(function () {
		(globalThis as any).Path2D = originalPath2D;
	});

	afterEach(function () {
		ScratchCanvasStub.uninstall();
	});

	// A color of its own, apart from any effect color.
	const child = {
		type: 'polyPolygonColor',
		color: '#00ff00',
		path: 'M 10 10 L 90 10 L 90 90 Z',
	};

	function render(primitive: any): CanvasRecorder {
		const context = new CanvasRecorder(200, 200);
		(context as any).globalAlpha = 1;
		new cool.VectorPrimitiveRenderer().renderPrimitive(
			context as any,
			primitive,
		);
		return context;
	}

	/// The draw on a scratch canvas that blurs through the canvas shadow.
	function silhouetteOn(
		scratch: CanvasRecorder,
	): CanvasRecorderCall | undefined {
		return scratch
			.callsOf('drawImage')
			.find((call) => call.properties.shadowBlur !== undefined);
	}

	/// All calls on the target and the scratch canvases.
	function allCalls(context: CanvasRecorder): CanvasRecorderCall[] {
		let calls = context.calls.slice();
		for (const scratch of ScratchCanvasStub.created)
			calls = calls.concat(scratch.calls);
		return calls;
	}

	describe('with no scratch canvas to draw on', function () {
		beforeEach(function () {
			ScratchCanvasStub.installUnavailable();
		});

		it('leaves out a shadow rather than repainting the object', function () {
			// The object is its own primitive, so the shadow draws nothing.
			const context = render({
				type: 'shadow',
				color: '#000000',
				blur: 10,
				children: [child],
			});
			assert.strictEqual(context.countOf('fill'), 0);
		});

		it('leaves out a glow rather than repainting the object', function () {
			const context = render({
				type: 'glow',
				color: '#ff0000',
				radius: 20,
				children: [child],
			});
			assert.strictEqual(context.countOf('fill'), 0);
		});

		it('draws the children of a soft edge without the fade', function () {
			const context = render({
				type: 'softEdge',
				radius: 20,
				children: [child],
			});
			const fill = context.findCall('fill');
			assert.ok(fill, 'the children are still drawn');
			assert.strictEqual(fill.properties.fillStyle, '#00ff00');
		});

		it('shows nothing of a transparence with an empty mask', function () {
			const context = render({
				type: 'transparence',
				children: [child],
				transparence: [],
			});
			assert.strictEqual(context.countOf('fill'), 0);
		});

		it('draws the children of a transparence without the mask', function () {
			const context = render({
				type: 'transparence',
				children: [child],
				transparence: [child],
			});
			const fill = context.findCall('fill');
			assert.ok(fill, 'the children are still drawn');
			assert.strictEqual(fill.properties.fillStyle, '#00ff00');
		});
	});

	describe('on a scratch canvas', function () {
		beforeEach(function () {
			ScratchCanvasStub.install();
		});

		it('repaints a shadow in the shadow color', function () {
			render({
				type: 'shadow',
				color: '#112233',
				blur: 4,
				children: [child],
			});

			// The outline comes out in the shadow color.
			const silhouette = silhouetteOn(ScratchCanvasStub.recorder(1));
			assert.ok(silhouette, 'the outline is drawn in one color');
			assert.strictEqual(silhouette.properties.shadowColor, '#112233');
		});

		it('moves a shadow by its matrix', function () {
			render({
				type: 'shadow',
				color: '#000000',
				blur: 4,
				matrix: [1, 0, 0, 1, 30, 40],
				children: [child],
			});

			const scratch = ScratchCanvasStub.recorder(0);
			const transform = scratch.findCall('transform');
			assert.ok(transform);
			assert.deepStrictEqual(transform.args, [1, 0, 0, 1, 30, 40]);
		});

		it('grows a glow before softening it', function () {
			// The halo grows by half the radius before the blur.
			render({
				type: 'glow',
				color: '#ff0000',
				radius: 40,
				children: [child],
			});

			const scratch = ScratchCanvasStub.recorder(0);
			// The grow draws the canvas onto itself at offsets around a ring.
			const spread = scratch
				.callsOf('drawImage')
				.filter((call) => call.args.length === 3 && call.args[1] !== 0);
			assert.ok(spread.length >= 8, 'the halo is grown around a ring');
		});

		it('does not grow a shadow, only soften it', function () {
			render({
				type: 'shadow',
				color: '#000000',
				blur: 40,
				children: [child],
			});

			const scratch = ScratchCanvasStub.recorder(0);
			const spread = scratch
				.callsOf('drawImage')
				.filter((call) => call.args.length === 3 && call.args[1] !== 0);
			assert.strictEqual(spread.length, 0);
		});

		it('blurs a glow by a sixth of its radius', function () {
			render({
				type: 'glow',
				color: '#ff0000',
				radius: 40,
				children: [child],
			});

			// Half the radius is three deviations, and shadowBlur is two.
			const silhouette = silhouetteOn(ScratchCanvasStub.recorder(1));
			assert.ok(silhouette);
			assert.ok(Math.abs(silhouette.properties.shadowBlur - 80 / 6) < 1e-9);
		});

		it('blurs a shadow by a third of its radius', function () {
			render({
				type: 'shadow',
				color: '#000000',
				blur: 40,
				children: [child],
			});

			// The radius is three deviations, and shadowBlur is two.
			const silhouette = silhouetteOn(ScratchCanvasStub.recorder(1));
			assert.ok(silhouette);
			assert.ok(Math.abs(silhouette.properties.shadowBlur - 80 / 3) < 1e-9);
		});

		it('fades a soft edge through a blurred mask', function () {
			render({
				type: 'softEdge',
				radius: 20,
				children: [child],
			});

			const silhouette = silhouetteOn(ScratchCanvasStub.recorder(2));
			assert.ok(silhouette, 'the shrunk outline is blurred');
			assert.ok(Math.abs(silhouette.properties.shadowBlur - 40 / 3) < 1e-9);
		});

		it('blurs every effect without the canvas filter', function () {
			// Safari has no canvas filter.
			for (const effect of [
				{ type: 'shadow', color: '#000000', blur: 40 },
				{ type: 'glow', color: '#ff0000', radius: 40 },
				{ type: 'softEdge', radius: 20 },
			]) {
				ScratchCanvasStub.reset();
				const context = render({ ...effect, children: [child] });
				const filtered = allCalls(context).filter(
					(call) => call.properties.filter,
				);
				assert.deepStrictEqual(filtered, [], effect.type + ' sets a filter');
			}
		});

		it('takes a transparence mask from the luminance over white', function () {
			render({
				type: 'transparence',
				children: [child],
				transparence: [child],
			});

			const mask = ScratchCanvasStub.recorder(1);
			const flood = mask.findCall('fillRect');
			assert.ok(flood, 'the mask starts out clear');
			// White is fully clear, so an untouched mask hides all.
			assert.strictEqual(flood.properties.fillStyle, '#ffffff');
		});

		it('shows a glow as transparent as its color', function () {
			const context = render({
				type: 'glow',
				color: '#ff0000',
				transparency: 0.6,
				radius: 40,
				children: [child],
			});

			const draw = context.findCall('drawImage');
			assert.ok(draw, 'the halo lands on the target');
			assert.ok(Math.abs(draw.properties.globalAlpha - 0.4) < 1e-9);
		});

		it('takes the mask luminance with the engine weights', function () {
			// The content is opaque green and the mask pure red.
			ScratchCanvasStub.imagePixels.push([0, 255, 0, 255], [255, 0, 0, 255]);
			render({
				type: 'transparence',
				children: [child],
				transparence: [child],
			});

			// Red weighs 0.2125, so the content keeps 78.75% of its alpha.
			const put = ScratchCanvasStub.recorder(0).findCall('putImageData');
			assert.ok(put);
			assert.strictEqual(put.args[0].data[3], 201);
		});

		it('draws an effect on a canvas the size of its bounds', function () {
			const context = render({
				type: 'shadow',
				color: '#000000',
				blur: 0,
				bounds: [10, 10, 50, 50],
				children: [child],
			});

			// A pixel more on each side for the antialiased edge.
			const scratch = ScratchCanvasStub.recorder(0);
			assert.strictEqual(scratch.canvas.width, 42);
			assert.strictEqual(scratch.canvas.height, 42);
			const draw = context.findCall('drawImage');
			assert.ok(draw);
			assert.deepStrictEqual(draw.args.slice(1), [9, 9]);
		});

		it('keeps a blurred effect to the bounds the engine gives', function () {
			// The bounds include the glow, so the canvas covers only them.
			const context = render({
				type: 'glow',
				color: '#ff0000',
				radius: 8,
				bounds: [10, 10, 50, 50],
				children: [child],
			});

			const scratch = ScratchCanvasStub.recorder(0);
			assert.strictEqual(scratch.canvas.width, 42);
			assert.strictEqual(scratch.canvas.height, 42);
			const draw = context.findCall('drawImage');
			assert.ok(draw);
			assert.deepStrictEqual(draw.args.slice(1), [9, 9]);
		});

		it('keeps the soft edge solid where the object runs past the target', function () {
			// The object crosses the right edge of the 200 pixel target.
			// The canvas reaches past that edge, so the fade stays off it.
			const context = render({
				type: 'softEdge',
				radius: 20,
				bounds: [150, 10, 260, 90],
				children: [child],
			});

			const content = ScratchCanvasStub.recorder(0);
			const draw = context.findCall('drawImage');
			assert.ok(draw);
			const left = draw.args[1];
			assert.ok(left > 0 && left < 150, 'the canvas starts near the object');
			assert.ok(
				left + content.canvas.width > 200 + 20,
				'the canvas reaches past the target edge by more than the radius',
			);
		});

		it('draws nothing of an effect whose bounds are off the target', function () {
			const context = render({
				type: 'glow',
				color: '#ff0000',
				radius: 4,
				bounds: [1000, 1000, 1100, 1100],
				children: [child],
			});
			assert.strictEqual(context.countOf('drawImage'), 0);
			assert.strictEqual(ScratchCanvasStub.created.length, 0);
		});

		it('stops making canvases once the effects nest too deep', function () {
			// One level past the slots, so the innermost draws its children
			// plainly.
			const nested = {
				type: 'softEdge',
				radius: 20,
				children: [
					{
						type: 'softEdge',
						radius: 20,
						children: [
							{
								type: 'softEdge',
								radius: 20,
								children: [{ type: 'softEdge', radius: 20, children: [child] }],
							},
						],
					},
				],
			};
			render(nested);

			// Three levels of three slots is the ceiling.
			assert.ok(
				ScratchCanvasStub.created.length <= 9,
				'canvases stop being made past the nesting limit, got ' +
					ScratchCanvasStub.created.length,
			);
		});
	});
});
