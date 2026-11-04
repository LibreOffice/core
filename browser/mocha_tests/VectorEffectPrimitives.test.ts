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

	// The child flipped onto the 80 by 80 square below it, starting at 110.
	const reflection = {
		type: 'reflection',
		matrix: [1, 0, 0, -1, 0, 200],
		range: [10, 110, 90, 190],
		startTransparency: 0.25,
		startPosition: 0.1,
		endTransparency: 1,
		endPosition: 0.8,
		bounds: [10, 110, 90, 190],
		children: [child],
	};

	function render(
		primitive: any,
		size: number = 200,
		canFilter: boolean = false,
	): CanvasRecorder {
		const context = new CanvasRecorder(size, size);
		(context as any).globalAlpha = 1;
		if (canFilter) (context as any).filter = 'none';
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

		it('leaves out a reflection rather than repainting the object', function () {
			const context = render(reflection);
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

		it('flips a reflection by its matrix', function () {
			render(reflection);
			const transform = ScratchCanvasStub.recorder(0).findCall('transform');
			assert.ok(transform);
			assert.deepStrictEqual(transform.args, [1, 0, 0, -1, 0, 200]);
		});

		it('fades a reflection from the start transparency to the end one', function () {
			render(reflection);

			const content = ScratchCanvasStub.recorder(0);
			const gradient = content.gradients[0];
			assert.ok(gradient, 'the fade is a gradient');
			// The fade runs down the range.
			assert.deepStrictEqual(gradient.args, [0, 110, 0, 190]);
			assert.deepStrictEqual(
				gradient.sortedStops().map((stop) => stop.offset),
				[0, 0.1, 0.8, 1],
			);
			assert.deepStrictEqual(
				gradient.sortedStops().map((stop) => stop.color),
				[
					'rgba(0,0,0,0.75)',
					'rgba(0,0,0,0.75)',
					'rgba(0,0,0,0)',
					'rgba(0,0,0,0)',
				],
			);

			// The fade scales the alpha of the reflection.
			const fade = content.findCall('fillRect');
			assert.ok(fade);
			assert.strictEqual(
				fade.properties.globalCompositeOperation,
				'destination-in',
			);
			assert.deepStrictEqual(fade.args, [10, 110, 80, 80]);
		});

		it('draws a reflection without a blur pixel for pixel', function () {
			const context = render(reflection);

			assert.strictEqual(
				ScratchCanvasStub.recorder(0).countOf('getImageData'),
				0,
			);
			const draw = context.findCall('drawImage');
			assert.ok(draw, 'the reflection lands on the target');
			assert.strictEqual(draw.args.length, 3);
		});

		it('leaves out a reflection blur of less than a pixel', function () {
			render({ ...reflection, blur: 0.5 });
			assert.strictEqual(
				ScratchCanvasStub.recorder(0).countOf('getImageData'),
				0,
			);
		});

		it('blurs a reflection through the canvas filter where there is one', function () {
			const context = render({ ...reflection, blur: 8 }, 200, true);

			// A radius of 8 is two deviations of 4 pixels.
			const draw = context.findCall('drawImage');
			assert.ok(draw);
			assert.strictEqual(draw.properties.filter, 'blur(4px)');
			assert.strictEqual(draw.args.length, 3, 'drawn at full size');
			assert.strictEqual(
				ScratchCanvasStub.recorder(0).countOf('getImageData'),
				0,
			);
		});

		it('blurs a large reflection through the filter at full size', function () {
			render(
				{ ...reflection, blur: 8, bounds: [0, 0, 1000, 1000] },
				1000,
				true,
			);
			const content = ScratchCanvasStub.recorder(0);
			assert.ok(content.canvas.width > 1000 && content.canvas.height > 1000);
		});

		it('blurs a reflection in its own colors without the canvas filter', function () {
			const context = render({ ...reflection, blur: 8 });

			// The blur runs on the pixels.
			const content = ScratchCanvasStub.recorder(0);
			assert.strictEqual(content.countOf('getImageData'), 1);
			assert.strictEqual(content.countOf('putImageData'), 1);

			// The canvas shadow is not used, as it paints in one color.
			const blurring = allCalls(context).filter(
				(call) => call.properties.filter || call.properties.shadowBlur,
			);
			assert.deepStrictEqual(blurring, []);
		});

		it('blurs a large reflection on fewer pixels without the filter', function () {
			// Over a million pixels, so it draws at about half size in each
			// direction, as in the engine.
			const context = render(
				{ ...reflection, blur: 8, bounds: [0, 0, 1000, 1000] },
				1000,
			);

			const content = ScratchCanvasStub.recorder(0);
			assert.ok(content.canvas.width < 600 && content.canvas.height < 600);
			assert.ok(content.canvas.width * content.canvas.height <= 252000);

			// One pixel more on each side for the antialiased edge.
			const draw = context.findCall('drawImage');
			assert.ok(draw);
			assert.deepStrictEqual(draw.args.slice(1, 3), [-1, -1]);
			assert.ok(
				Math.abs(draw.args[3] - 1002) < 3,
				'drawn ' + draw.args[3] + ' wide',
			);
			assert.ok(
				Math.abs(draw.args[4] - 1002) < 3,
				'drawn ' + draw.args[4] + ' high',
			);
		});

		it('draws the reflection the engine writes, flipped and faded', function () {
			// The engine's content is 100 by 50, and its reflection starts 10
			// below it and fades from half transparent to clear at 0.9.
			const primitive =
				loadVectorRenderingReference('testReflection').primitives[0];
			render(primitive);

			const content = ScratchCanvasStub.recorder(0);
			const transform = content.findCall('transform');
			assert.ok(transform);
			const flip = [1, 0, 0, -1, 0, 110];
			for (let index = 0; index < 6; index++)
				assert.ok(Math.abs(transform.args[index] - flip[index]) < 1e-9);

			const gradient = content.gradients[0];
			assert.ok(gradient);
			assert.deepStrictEqual(gradient.args, [0, 60, 0, 110]);
			assert.deepStrictEqual(
				gradient.sortedStops().map((stop) => stop.color),
				[
					'rgba(0,0,0,0.5)',
					'rgba(0,0,0,0.5)',
					'rgba(0,0,0,0)',
					'rgba(0,0,0,0)',
				],
			);

			// A blur of 4 is two deviations of two pixels here.
			assert.strictEqual(content.countOf('putImageData'), 1);
		});

		it('draws nothing of a reflection that has nothing to show', function () {
			// The engine sends no range or children for an invisible one.
			const context = render({ type: 'reflection' });
			assert.strictEqual(context.countOf('drawImage'), 0);
			assert.strictEqual(ScratchCanvasStub.created.length, 0);
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
