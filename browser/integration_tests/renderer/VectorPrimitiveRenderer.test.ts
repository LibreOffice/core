/* -*- js-indent-level: 8 -*- */
/*
 * Copyright the Collabora Office contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

/// The browser renderer draws the primitive samples that a running coolwsd writes, so the JSON
/// the engine writes today is the JSON that these tests draw.
describe('Integration: drawing the primitive samples of the engine', function () {
	this.timeout(60000);

	// The renderer constructs new Path2D and new Image. Stand-ins capture the constructor
	// arguments and let the renderer treat the resulting object as a ready resource.
	let originalPath2D: typeof Path2D;
	let originalImage: typeof Image;

	before(function () {
		originalPath2D = globalThis.Path2D;
		originalImage = globalThis.Image;
		Object.assign(globalThis, { Path2D: Path2DRecorder, Image: ImageRecorder });
	});

	after(function () {
		Object.assign(globalThis, { Path2D: originalPath2D, Image: originalImage });
	});

	afterEach(function () {
		ScratchCanvasStub.uninstall();
	});

	describe('Primitives', function () {
		it('fills the slide rectangle for backgroundcolor', async function () {
			const primitive = (await vectorPrimitiveSample('backgroundColor'))
				.primitives[0];
			nodeassert.strictEqual(primitive.type, 'backgroundcolor');
			nodeassert.strictEqual(typeof primitive.color, 'string');

			const recorder = new CanvasRecorder(200, 150);
			const renderer = new cool.VectorPrimitiveRenderer();
			// Slide smaller than the canvas: the fill must follow the slide,
			// leaving the surrounding area for the workspace color.
			renderer.setSlideBounds(120, 90);
			renderer.renderPrimitive(recorder as any, primitive);

			const fillRect = recorder.findCall('fillRect');
			nodeassert.ok(fillRect, 'fillRect not called');
			nodeassert.deepStrictEqual(fillRect.args, [0, 0, 120, 90]);
			nodeassert.strictEqual(recorder.properties.fillStyle, primitive.color);
			// The background fill must not leak its canvas state to
			// anything drawn after it.
			nodeassert.ok(recorder.findCall('save'), 'save not called');
			nodeassert.ok(recorder.findCall('restore'), 'restore not called');
		});

		it('respects backgroundcolor transparency', async function () {
			const primitive = (
				await vectorPrimitiveSample('backgroundColorTransparent')
			).primitives[0];
			nodeassert.strictEqual(primitive.type, 'backgroundcolor');
			nodeassert.strictEqual(typeof primitive.transparency, 'number');
			nodeassert.ok(
				primitive.transparency > 0,
				'the sample must be partly transparent',
			);

			const recorder = new CanvasRecorder();
			const renderer = new cool.VectorPrimitiveRenderer();
			renderer.renderPrimitive(recorder as any, primitive);

			nodeassert.strictEqual(recorder.properties.fillStyle, primitive.color);
			nodeassert.strictEqual(
				recorder.properties.globalAlpha,
				1 - primitive.transparency,
			);
		});

		it('renders polyPolygonColor with its fill color', async function () {
			const primitive = (await vectorPrimitiveSample('polyPolygonColor'))
				.primitives[0];
			nodeassert.strictEqual(primitive.type, 'polyPolygonColor');
			nodeassert.strictEqual(typeof primitive.color, 'string');
			nodeassert.strictEqual(typeof primitive.path, 'string');

			const recorder = new CanvasRecorder();
			const renderer = new cool.VectorPrimitiveRenderer();
			renderer.renderPrimitive(recorder as any, primitive);

			const fillCall = recorder.findCall('fill');
			nodeassert.ok(fillCall, 'fill not called');
			nodeassert.strictEqual(recorder.properties.fillStyle, primitive.color);
			nodeassert.strictEqual(
				(fillCall.args[0] as Path2DRecorder).path,
				primitive.path,
			);
			// The fill uses the even-odd rule, matching the engine, so
			// a subpath inside another subpath reads as a hole.
			nodeassert.strictEqual(fillCall.args[1], 'evenodd');
		});

		it('keeps a polypolygon hole unpainted', async function () {
			// The engine ships the outer ring and the hole as two
			// subpaths of one path, both travelling in the same
			// direction. A single even-odd fill leaves the hole
			// unpainted.
			const primitive = (
				await vectorPrimitiveSample('polyPolygonColorWithHole')
			).primitives[0];
			nodeassert.strictEqual(primitive.type, 'polyPolygonColor');
			nodeassert.strictEqual(
				(primitive.path.match(/[Mm]/g) || []).length,
				2,
				'the sample must carry two subpaths',
			);

			const recorder = new CanvasRecorder();
			const renderer = new cool.VectorPrimitiveRenderer();
			renderer.renderPrimitive(recorder as any, primitive);

			const fills = recorder.callsOf('fill');
			nodeassert.strictEqual(fills.length, 1, 'expected a single fill');
			nodeassert.strictEqual(
				(fills[0].args[0] as Path2DRecorder).path,
				primitive.path,
			);
			nodeassert.strictEqual(fills[0].args[1], 'evenodd');
		});

		it('applies transparency to polyPolygonRGBA at fill time', async function () {
			// polyPolygonRGBA paints with its color and respects the
			// optional transparency. The alpha must apply only to this
			// primitive and not leak to anything drawn after.
			const primitive = (await vectorPrimitiveSample('polyPolygonRGBA'))
				.primitives[0];
			nodeassert.strictEqual(primitive.type, 'polyPolygonRGBA');
			nodeassert.strictEqual(typeof primitive.color, 'string');
			nodeassert.strictEqual(typeof primitive.transparency, 'number');
			nodeassert.ok(
				primitive.transparency > 0,
				'the sample must be partly transparent',
			);

			const recorder = new CanvasRecorder();
			const renderer = new cool.VectorPrimitiveRenderer();
			renderer.renderPrimitive(recorder as any, primitive);

			const fill = recorder.findCall('fill');
			nodeassert.ok(fill, 'fill not called');
			nodeassert.strictEqual(fill?.properties.fillStyle, primitive.color);
			nodeassert.strictEqual(
				fill?.properties.globalAlpha,
				1 - primitive.transparency,
			);
			nodeassert.strictEqual(
				(fill?.args[0] as Path2DRecorder).path,
				primitive.path,
			);
			nodeassert.ok(recorder.findCall('save'), 'save not called');
			nodeassert.ok(recorder.findCall('restore'), 'restore not called');
		});

		it('strokes a Path2D for polygonStroke', async function () {
			const primitive = (await vectorPrimitiveSample('polygonStroke'))
				.primitives[0];
			nodeassert.strictEqual(primitive.type, 'polygonStroke');
			nodeassert.strictEqual(typeof primitive.path, 'string');
			nodeassert.strictEqual(typeof primitive.line?.color, 'string');
			nodeassert.strictEqual(typeof primitive.line?.width, 'number');

			const recorder = new CanvasRecorder();
			const renderer = new cool.VectorPrimitiveRenderer();
			renderer.renderPrimitive(recorder as any, primitive);

			const stroke = recorder.findCall('stroke');
			nodeassert.ok(stroke, 'stroke not called');
			// Use the per-call snapshot so we see the stroke properties
			// as they were when the renderer drew this primitive, not
			// whatever state another primitive left behind.
			nodeassert.strictEqual(
				stroke?.properties.strokeStyle,
				primitive.line.color,
			);
			nodeassert.strictEqual(
				stroke?.properties.lineWidth,
				primitive.line.width,
			);
			nodeassert.strictEqual(
				stroke?.properties.lineJoin,
				primitive.line.linejoin,
			);
			nodeassert.strictEqual(
				stroke?.properties.lineCap,
				primitive.line.linecap,
			);
			nodeassert.ok(
				stroke && stroke.depth > 0,
				'stroke must be inside save/restore',
			);
			nodeassert.strictEqual(
				(stroke?.args[0] as Path2DRecorder).path,
				primitive.path,
			);
		});

		it('applies dotDashArray for polygonStroke', async function () {
			const primitive = (await vectorPrimitiveSample('polygonStrokeDashed'))
				.primitives[0];
			nodeassert.strictEqual(primitive.type, 'polygonStroke');
			nodeassert.ok(
				Array.isArray(primitive.stroke?.dotDashArray),
				'the sample must carry a dotDashArray',
			);

			const recorder = new CanvasRecorder();
			const renderer = new cool.VectorPrimitiveRenderer();
			renderer.renderPrimitive(recorder as any, primitive);

			const setLineDash = recorder.findCall('setLineDash');
			nodeassert.ok(setLineDash, 'setLineDash not called');
			nodeassert.deepStrictEqual(setLineDash.args, [
				primitive.stroke.dotDashArray,
			]);
		});

		it('renders polyPolygonStroke as one stroked path', async function () {
			// polyPolygonStroke ships an SVG-D path that describes
			// more than one disjoint subpath. The whole thing strokes
			// in a single canvas call.
			const primitive = (await vectorPrimitiveSample('polyPolygonStroke'))
				.primitives[0];
			nodeassert.strictEqual(primitive.type, 'polyPolygonStroke');
			nodeassert.strictEqual(typeof primitive.path, 'string');
			nodeassert.strictEqual(typeof primitive.line?.color, 'string');

			const recorder = new CanvasRecorder();
			const renderer = new cool.VectorPrimitiveRenderer();
			renderer.renderPrimitive(recorder as any, primitive);

			const stroke = recorder.findCall('stroke');
			nodeassert.ok(stroke, 'stroke not called');
			nodeassert.strictEqual(recorder.countOf('stroke'), 1);
			nodeassert.strictEqual(
				stroke?.properties.strokeStyle,
				primitive.line.color,
			);
			nodeassert.strictEqual(
				(stroke?.args[0] as Path2DRecorder).path,
				primitive.path,
			);
		});

		it("clips a mask's children to its path", async function () {
			// mask carries a clip path and a child subtree.
			const primitive = (await vectorPrimitiveSample('mask')).primitives[0];
			nodeassert.strictEqual(primitive.type, 'mask');
			nodeassert.strictEqual(typeof primitive.clip, 'string');
			nodeassert.strictEqual(primitive.children.length, 1);

			const recorder = new CanvasRecorder();
			const renderer = new cool.VectorPrimitiveRenderer();
			renderer.renderPrimitive(recorder as any, primitive);

			// The clip is applied once with the wire path.
			const clip = recorder.findCall('clip');
			nodeassert.ok(clip, 'clip not called');
			nodeassert.strictEqual(recorder.countOf('clip'), 1);
			nodeassert.strictEqual(
				(clip?.args[0] as Path2DRecorder).path,
				primitive.clip,
			);
			// Even-odd, so disjoint subpaths in the clip behave the
			// same way as in the source path.
			nodeassert.strictEqual(clip?.args[1], 'evenodd');

			// The single child draws once and the canvas state is
			// balanced around it.
			nodeassert.strictEqual(recorder.countOf('fill'), 1);
			nodeassert.ok(recorder.findCall('save'), 'save not called');
			nodeassert.ok(recorder.findCall('restore'), 'restore not called');
		});

		it('renders textSimplePortion at the matrix anchor', async function () {
			// textSimplePortion paints text in the requested font and
			// color at the matrix anchor. The substring offsets pick
			// out part of the carried text.
			const primitive = (await vectorPrimitiveSample('textSimplePortion'))
				.primitives[0];
			nodeassert.strictEqual(primitive.type, 'textSimplePortion');
			nodeassert.strictEqual(primitive.text, 'Hello');
			nodeassert.strictEqual(primitive.textPosition, 0);
			nodeassert.strictEqual(primitive.textLength, 5);

			const recorder = new CanvasRecorder();
			const renderer = new cool.VectorPrimitiveRenderer();
			renderer.renderPrimitive(recorder as any, primitive);

			const fillText = recorder.findCall('fillText');
			nodeassert.ok(fillText, 'fillText not called');
			nodeassert.strictEqual(fillText?.args[0], 'Hello');
			// The anchor (translation components of the matrix) lands
			// in args[1] and args[2].
			nodeassert.strictEqual(fillText?.args[1], primitive.matrix[4]);
			nodeassert.strictEqual(fillText?.args[2], primitive.matrix[5]);
			// The fillStyle at the call matches the wire fontcolor.
			nodeassert.strictEqual(
				fillText?.properties.fillStyle,
				primitive.fontcolor,
			);
			// The font string carries the wire fontSize and family.
			nodeassert.ok(
				fillText?.properties.font.includes(`${primitive.fontSize}px`),
				'font string missing fontSize',
			);
			nodeassert.ok(
				fillText?.properties.font.includes(primitive.familyname),
				'font string missing family',
			);
		});

		it('draws underline and strikeout for textDecoratedPortion', async function () {
			// textDecoratedPortion paints the same text as the simple
			// portion and the lines the engine built for its underline
			// and strikeout on top.
			const primitive = (await vectorPrimitiveSample('textDecoratedPortion'))
				.primitives[0];
			nodeassert.strictEqual(primitive.type, 'textDecoratedPortion');
			nodeassert.ok(primitive.underline > 0, 'the sample must have underline');
			nodeassert.ok(primitive.strikeout > 0, 'the sample must have strikeout');

			const recorder = new CanvasRecorder();
			const renderer = new cool.VectorPrimitiveRenderer();
			renderer.renderPrimitive(recorder as any, primitive);

			// Text is still drawn.
			nodeassert.ok(recorder.findCall('fillText'), 'fillText not called');

			// One stroke of an engine path per decoration, in black as the
			// sample paints them.
			const strokes = recorder.callsOf('stroke');
			nodeassert.strictEqual(strokes.length, 2);
			for (const stroke of strokes) {
				nodeassert.ok(stroke.args[0] instanceof Path2DRecorder);
				nodeassert.strictEqual(stroke.properties.strokeStyle, '#000000');
			}
			nodeassert.strictEqual(recorder.callsOf('moveTo').length, 0);
		});

		it('renders a bitmap at its declared bounds', async function () {
			// bitmap maps the unit square (0,0)-(1,1) to its display
			// bounds through the wire matrix. The image content comes
			// from a checksum lookup populated by a separate fetch.
			const primitive = (await vectorPrimitiveSample('bitmap')).primitives[0];
			nodeassert.strictEqual(primitive.type, 'bitmap');
			nodeassert.strictEqual(typeof primitive.checksum, 'number');
			nodeassert.strictEqual(primitive.matrix.length, 6);

			const cachedImage = new ImageRecorder();
			const recorder = new CanvasRecorder();
			const renderer = new cool.VectorPrimitiveRenderer((checksum) =>
				checksum === primitive.checksum
					? (cachedImage as unknown as HTMLImageElement)
					: undefined,
			);
			renderer.renderPrimitive(recorder as any, primitive);

			// The matrix is applied to the context.
			const transform = recorder.findCall('transform');
			nodeassert.ok(transform, 'transform not called');
			nodeassert.strictEqual(transform.args.length, 6);
			for (let i = 0; i < 6; i++)
				nodeassert.strictEqual(transform.args[i], primitive.matrix[i]);

			// The image from the lookup is drawn into the unit square.
			const draw = recorder.findCall('drawImage');
			nodeassert.ok(draw, 'drawImage not called');
			nodeassert.strictEqual(draw.args[0], cachedImage);
			nodeassert.deepStrictEqual(draw.args.slice(1), [0, 0, 1, 1]);

			// The transform stays scoped to this primitive.
			nodeassert.ok(recorder.findCall('save'), 'save not called');
			nodeassert.ok(recorder.findCall('restore'), 'restore not called');
		});

		it('applies bitmapAlpha transparency as globalAlpha', async function () {
			// bitmapAlpha shares the bitmap shape and adds a
			// transparency in [0, 1]. The renderer maps it to
			// globalAlpha = 1 - transparency inside the save and
			// restore around the draw.
			const primitive = (await vectorPrimitiveSample('bitmapAlpha'))
				.primitives[0];
			nodeassert.strictEqual(primitive.type, 'bitmapAlpha');
			nodeassert.strictEqual(typeof primitive.checksum, 'number');
			nodeassert.strictEqual(typeof primitive.transparency, 'number');
			nodeassert.ok(
				primitive.transparency > 0,
				'the sample must be partly transparent',
			);

			const cachedImage = new ImageRecorder();
			const recorder = new CanvasRecorder();
			const renderer = new cool.VectorPrimitiveRenderer((checksum) =>
				checksum === primitive.checksum
					? (cachedImage as unknown as HTMLImageElement)
					: undefined,
			);
			renderer.renderPrimitive(recorder as any, primitive);

			const draw = recorder.findCall('drawImage');
			nodeassert.ok(draw, 'drawImage not called');
			nodeassert.strictEqual(draw.args[0], cachedImage);
			nodeassert.strictEqual(
				draw.properties.globalAlpha,
				1 - primitive.transparency,
			);
		});

		it('renders a raster graphic like a bitmap', async function () {
			// A raster graphic carries a matrix and a checksum. The
			// renderer resolves the image through the same checksum
			// lookup as a bitmap and draws it into the unit square.
			const primitive = (await vectorPrimitiveSample('graphic')).primitives[0];
			nodeassert.strictEqual(primitive.type, 'graphic');
			nodeassert.strictEqual(primitive.vector, undefined);
			nodeassert.strictEqual(typeof primitive.checksum, 'number');
			nodeassert.strictEqual(primitive.matrix.length, 6);

			const cachedImage = new ImageRecorder();
			const recorder = new CanvasRecorder();
			const renderer = new cool.VectorPrimitiveRenderer((checksum) =>
				checksum === primitive.checksum
					? (cachedImage as unknown as HTMLImageElement)
					: undefined,
			);
			renderer.renderPrimitive(recorder as any, primitive);

			const transform = recorder.findCall('transform');
			nodeassert.ok(transform, 'transform not called');
			for (let i = 0; i < 6; i++)
				nodeassert.strictEqual(transform.args[i], primitive.matrix[i]);

			const draw = recorder.findCall('drawImage');
			nodeassert.ok(draw, 'drawImage not called');
			nodeassert.strictEqual(draw.args[0], cachedImage);
			nodeassert.deepStrictEqual(draw.args.slice(1), [0, 0, 1, 1]);
		});

		it('renders an SVG-backed graphic through the raster path', async function () {
			// The engine's SVG branch emits the same wire shape as a
			// raster graphic. The renderer treats the primitive the
			// same way and resolves the image through the checksum
			// lookup.
			const primitive = (await vectorPrimitiveSample('graphicSvg'))
				.primitives[0];
			nodeassert.strictEqual(primitive.type, 'graphic');
			nodeassert.strictEqual(primitive.vector, undefined);
			nodeassert.strictEqual(typeof primitive.checksum, 'number');
			nodeassert.strictEqual(primitive.matrix.length, 6);

			const cachedImage = new ImageRecorder();
			const recorder = new CanvasRecorder();
			const renderer = new cool.VectorPrimitiveRenderer((checksum) =>
				checksum === primitive.checksum
					? (cachedImage as unknown as HTMLImageElement)
					: undefined,
			);
			renderer.renderPrimitive(recorder as any, primitive);

			const draw = recorder.findCall('drawImage');
			nodeassert.ok(draw, 'drawImage not called');
			nodeassert.strictEqual(draw.args[0], cachedImage);
			nodeassert.deepStrictEqual(draw.args.slice(1), [0, 0, 1, 1]);
		});

		it('descends into a vector graphic instead of drawing it', async function () {
			// An EMF-backed graphic arrives pre-decomposed. The
			// renderer must not call drawImage. It descends into the
			// children instead, so at least one canvas operation
			// comes from inside the subtree.
			const primitive = (await vectorPrimitiveSample('graphicEmf'))
				.primitives[0];
			nodeassert.strictEqual(primitive.type, 'graphic');
			nodeassert.strictEqual(primitive.vector, true);
			nodeassert.ok(Array.isArray(primitive.children));
			nodeassert.ok(primitive.children.length > 0);

			const recorder = new CanvasRecorder();
			const renderer = new cool.VectorPrimitiveRenderer();
			renderer.renderPrimitive(recorder as any, primitive);

			nodeassert.strictEqual(
				recorder.findCall('drawImage'),
				undefined,
				'drawImage should not be called for a vector graphic',
			);
			nodeassert.ok(
				recorder.calls.length > 0,
				'expected the children subtree to produce canvas calls',
			);
		});

		it('applies the alpha attribute as globalAlpha', async function () {
			// alpha is a byte where 255 is opaque. The renderer sets
			// globalAlpha to alpha / 255 inside the save/restore
			// scope, so the value does not leak to anything drawn
			// after this primitive.
			const primitive = (await vectorPrimitiveSample('graphicAlpha'))
				.primitives[0];
			nodeassert.strictEqual(primitive.type, 'graphic');
			nodeassert.strictEqual(typeof primitive.alpha, 'number');
			nodeassert.ok(
				primitive.alpha < 255,
				'the sample must be partly transparent',
			);

			const cachedImage = new ImageRecorder();
			const recorder = new CanvasRecorder();
			const renderer = new cool.VectorPrimitiveRenderer((checksum) =>
				checksum === primitive.checksum
					? (cachedImage as unknown as HTMLImageElement)
					: undefined,
			);
			renderer.renderPrimitive(recorder as any, primitive);

			const draw = recorder.findCall('drawImage');
			nodeassert.ok(draw, 'drawImage not called');
			nodeassert.strictEqual(
				draw.properties.globalAlpha,
				primitive.alpha / 255,
			);
			nodeassert.ok(recorder.findCall('save'), 'save not called');
			nodeassert.ok(recorder.findCall('restore'), 'restore not called');
		});

		it('flips a mirrored graphic along the requested axes', async function () {
			// mirror is a bitfield: bit 0 flips horizontally, bit 1
			// flips vertically. The renderer translates to the far
			// edge of each flipped axis, then scales by -1 on that
			// axis so the image still fills the unit square.
			const primitive = (await vectorPrimitiveSample('graphicMirror'))
				.primitives[0];
			nodeassert.strictEqual(primitive.type, 'graphic');
			nodeassert.strictEqual(primitive.mirror, 3);

			const cachedImage = new ImageRecorder();
			const recorder = new CanvasRecorder();
			const renderer = new cool.VectorPrimitiveRenderer((checksum) =>
				checksum === primitive.checksum
					? (cachedImage as unknown as HTMLImageElement)
					: undefined,
			);
			renderer.renderPrimitive(recorder as any, primitive);

			// With both axes flipped, the translate goes to (1, 1)
			// and the scale is (-1, -1).
			const translate = recorder.findCall('translate');
			nodeassert.ok(translate, 'translate not called');
			nodeassert.deepStrictEqual(translate.args, [1, 1]);

			const scale = recorder.findCall('scale');
			nodeassert.ok(scale, 'scale not called');
			nodeassert.deepStrictEqual(scale.args, [-1, -1]);

			const draw = recorder.findCall('drawImage');
			nodeassert.ok(draw, 'drawImage not called');
			nodeassert.strictEqual(draw.args[0], cachedImage);
		});

		it('grays the pixels of a graphic in the greys draw mode', async function () {
			// The image is grayed once, at its own size, and the copy is drawn.
			const primitive = (await vectorPrimitiveSample('graphicDrawMode'))
				.primitives[0];
			nodeassert.strictEqual(primitive.type, 'graphic');
			nodeassert.deepStrictEqual(primitive.colorModifiers, [
				{ modifier: 'gray' },
			]);

			ScratchCanvasStub.install();
			ScratchCanvasStub.imagePixels[0] = [255, 0, 0, 255];
			const cachedImage = new ImageRecorder();
			const recorder = new CanvasRecorder();
			const renderer = new cool.VectorPrimitiveRenderer((checksum) =>
				checksum === primitive.checksum
					? (cachedImage as unknown as HTMLImageElement)
					: undefined,
			);
			renderer.renderPrimitive(recorder as any, primitive);

			const copy = ScratchCanvasStub.recorder(0);
			nodeassert.strictEqual(copy.canvas.width, cachedImage.naturalWidth);
			nodeassert.strictEqual(copy.findCall('drawImage')?.args[0], cachedImage);
			const put = copy.findCall('putImageData');
			nodeassert.ok(put, 'the gray pixels are written back');
			// Red grays to the engine's 77 out of 256.
			nodeassert.deepStrictEqual(
				Array.from(put.args[0].data.slice(0, 4)),
				[77, 77, 77, 255],
			);

			const draw = recorder.findCall('drawImage');
			nodeassert.ok(draw, 'drawImage not called');
			nodeassert.strictEqual(draw.args[0], copy.canvas);
			nodeassert.strictEqual(draw.properties.filter, undefined);
		});

		it('lists every picture adjustment of a graphic in the order they apply', async function () {
			const primitive = (await vectorPrimitiveSample('graphicAdjusted'))
				.primitives[0];
			nodeassert.deepStrictEqual(
				primitive.colorModifiers.map((modifier: any) => modifier.modifier),
				['black_and_white', 'RGBLuminanceContrast', 'gamma', 'invert'],
			);

			ScratchCanvasStub.install();
			ScratchCanvasStub.imagePixels[0] = [0, 0, 0, 255];
			const cachedImage = new ImageRecorder();
			const recorder = new CanvasRecorder();
			new cool.VectorPrimitiveRenderer(
				() => cachedImage as unknown as HTMLImageElement,
			).renderPrimitive(recorder as any, primitive);

			// Black stays black in mono, then each other adjustment applies.
			const put = ScratchCanvasStub.recorder(0).findCall('putImageData');
			nodeassert.ok(put, 'the adjusted pixels are written back');
			const expected = [0, 0, 0];
			for (const modifier of primitive.colorModifiers)
				cool.VectorColorModifiers.colorChange(modifier)!(expected);
			nodeassert.deepStrictEqual(
				Array.from(put.args[0].data.slice(0, 3)),
				expected.map((channel) => Math.round(channel * 255)),
			);
			nodeassert.strictEqual(
				recorder.findCall('drawImage')?.args[0],
				ScratchCanvasStub.recorder(0).canvas,
			);
		});

		it('draws a cropped graphic onto its image rectangle', async function () {
			// The renderer draws the whole image onto the wire
			// rectangle and clips to the frame's unit square.
			const primitive = (await vectorPrimitiveSample('graphicCrop'))
				.primitives[0];
			nodeassert.strictEqual(primitive.type, 'graphic');
			nodeassert.ok(primitive.imageRect, 'imageRect missing');
			// All four edges cropped: the rectangle overflows the
			// unit square on every side.
			nodeassert.ok(primitive.imageRect.x < 0);
			nodeassert.ok(primitive.imageRect.y < 0);
			nodeassert.ok(primitive.imageRect.x + primitive.imageRect.width > 1);
			nodeassert.ok(primitive.imageRect.y + primitive.imageRect.height > 1);
			nodeassert.strictEqual(typeof primitive.checksum, 'number');

			const cachedImage = new ImageRecorder();
			const recorder = new CanvasRecorder();
			const renderer = new cool.VectorPrimitiveRenderer((checksum) =>
				checksum === primitive.checksum
					? (cachedImage as unknown as HTMLImageElement)
					: undefined,
			);
			renderer.renderPrimitive(recorder as any, primitive);

			const draw = recorder.findCall('drawImage');
			nodeassert.ok(draw, 'drawImage not called');
			nodeassert.deepStrictEqual(draw.args, [
				cachedImage,
				primitive.imageRect.x,
				primitive.imageRect.y,
				primitive.imageRect.width,
				primitive.imageRect.height,
			]);
			nodeassert.ok(recorder.findCall('clip'), 'clip not called');
		});

		it('rotates a graphic around the centre of the unit square', async function () {
			// The wire rotation is in tenths of a degree. The
			// renderer translates to the centre of the unit square,
			// rotates by that angle in radians, then translates back,
			// so the rotation pivots around the image's centre after
			// the matrix maps it onto the slide.
			const primitive = (await vectorPrimitiveSample('graphicRotation'))
				.primitives[0];
			nodeassert.strictEqual(primitive.type, 'graphic');
			nodeassert.strictEqual(typeof primitive.rotation, 'number');
			nodeassert.strictEqual(typeof primitive.checksum, 'number');

			const cachedImage = new ImageRecorder();
			const recorder = new CanvasRecorder();
			const renderer = new cool.VectorPrimitiveRenderer((checksum) =>
				checksum === primitive.checksum
					? (cachedImage as unknown as HTMLImageElement)
					: undefined,
			);
			renderer.renderPrimitive(recorder as any, primitive);

			// Two translates frame a single rotate, with the centre
			// of the unit square as the pivot.
			const translates = recorder.callsOf('translate');
			nodeassert.strictEqual(translates.length, 2);
			nodeassert.deepStrictEqual(translates[0].args, [0.5, 0.5]);
			nodeassert.deepStrictEqual(translates[1].args, [-0.5, -0.5]);

			const rotate = recorder.findCall('rotate');
			nodeassert.ok(rotate, 'rotate not called');
			const expectedRadians = ((primitive.rotation / 10) * Math.PI) / 180;
			nodeassert.strictEqual(rotate.args[0], expectedRadians);

			// The image is still drawn into the unit square. The
			// rotation lives in the canvas transform.
			const draw = recorder.findCall('drawImage');
			nodeassert.ok(draw, 'drawImage not called');
			nodeassert.strictEqual(draw.args[0], cachedImage);
			nodeassert.deepStrictEqual(draw.args.slice(1), [0, 0, 1, 1]);
		});

		it('paints each point of a pointArray', async function () {
			// pointArray lays down a single-pixel mark at every point,
			// all in the same color.
			const primitive = (await vectorPrimitiveSample('pointArray'))
				.primitives[0];
			nodeassert.strictEqual(primitive.type, 'pointArray');
			nodeassert.strictEqual(typeof primitive.color, 'string');
			nodeassert.ok(Array.isArray(primitive.points));

			const recorder = new CanvasRecorder();
			const renderer = new cool.VectorPrimitiveRenderer();
			renderer.renderPrimitive(recorder as any, primitive);

			const fills = recorder.callsOf('fillRect');
			nodeassert.strictEqual(fills.length, primitive.points.length);
			for (let i = 0; i < primitive.points.length; i++) {
				nodeassert.deepStrictEqual(fills[i].args, [
					primitive.points[i].x,
					primitive.points[i].y,
					1,
					1,
				]);
				nodeassert.strictEqual(fills[i].properties.fillStyle, primitive.color);
			}
		});

		it("renders a group's children in order", async function () {
			// A group node carries no drawing of its own. The renderer
			// must descend into the children in order.
			const primitive = (await vectorPrimitiveSample('group')).primitives[0];
			nodeassert.strictEqual(primitive.type, 'group');
			nodeassert.ok(Array.isArray(primitive.children));
			nodeassert.strictEqual(primitive.children.length, 2);

			const recorder = new CanvasRecorder();
			const renderer = new cool.VectorPrimitiveRenderer();
			renderer.renderPrimitive(recorder as any, primitive);

			// One fill, one stroke, in that order. The group itself
			// contributes nothing.
			nodeassert.strictEqual(recorder.countOf('fill'), 1);
			nodeassert.strictEqual(recorder.countOf('stroke'), 1);
			const fillIndex = recorder.calls.findIndex((c) => c.method === 'fill');
			const strokeIndex = recorder.calls.findIndex(
				(c) => c.method === 'stroke',
			);
			nodeassert.ok(
				fillIndex >= 0 && strokeIndex > fillIndex,
				'fill should be recorded before stroke',
			);
		});

		it('handles an empty group without drawing anything', async function () {
			const primitive = (await vectorPrimitiveSample('groupEmpty'))
				.primitives[0];
			nodeassert.strictEqual(primitive.type, 'group');
			nodeassert.strictEqual(primitive.children.length, 0);

			const recorder = new CanvasRecorder();
			const renderer = new cool.VectorPrimitiveRenderer();
			renderer.renderPrimitive(recorder as any, primitive);

			nodeassert.strictEqual(recorder.calls.length, 0);
			nodeassert.deepStrictEqual(recorder.properties, {});
		});

		it("renders children under the transform's matrix", async function () {
			// The transform primitive applies an affine matrix to its
			// child subtree. Children render under the transformed
			// coordinates and must not affect anything drawn after.
			const primitive = (await vectorPrimitiveSample('transform'))
				.primitives[0];
			nodeassert.strictEqual(primitive.type, 'transform');
			nodeassert.strictEqual(primitive.matrix.length, 6);
			nodeassert.strictEqual(primitive.children.length, 1);

			const recorder = new CanvasRecorder();
			const renderer = new cool.VectorPrimitiveRenderer();
			renderer.renderPrimitive(recorder as any, primitive);

			// The sample is a 30-degree rotation around the origin.
			const transform = recorder.findCall('transform');
			nodeassert.ok(transform, 'transform not called');
			nodeassert.strictEqual(recorder.countOf('transform'), 1);
			const expected = cool.Matrix2D.IDENTITY.rotateAround(
				0,
				0,
				Math.PI / 6,
			).toArray();
			nodeassert.strictEqual(transform.args.length, expected.length);
			// The wire ships the matrix as fixed-precision strings, so
			// the last digit can differ from Math.cos/Math.sin by a
			// floating-point ulp. Allow a small tolerance per element.
			for (let i = 0; i < expected.length; i++)
				nodeassert.ok(
					Math.abs(transform.args[i] - expected[i]) < 1e-12,
					`matrix[${i}] expected ~${expected[i]}, got ${transform.args[i]}`,
				);

			// Anything drawn after the transform must not carry its matrix.
			nodeassert.ok(recorder.findCall('save'), 'save not called');
			nodeassert.ok(recorder.findCall('restore'), 'restore not called');
			nodeassert.strictEqual(transform.depth, 1);

			// The single child renders exactly once.
			nodeassert.strictEqual(recorder.countOf('fill'), 1);
		});

		it('skips hiddenGeometry entirely', async function () {
			// hiddenGeometry is non-painting. Nothing is drawn,
			// even when a children array is present.
			const primitive = (await vectorPrimitiveSample('hiddenGeometry'))
				.primitives[0];
			nodeassert.strictEqual(primitive.type, 'hiddenGeometry');

			const recorder = new CanvasRecorder();
			const renderer = new cool.VectorPrimitiveRenderer();
			renderer.renderPrimitive(recorder as any, primitive);

			nodeassert.strictEqual(
				recorder.calls.length,
				0,
				'hiddenGeometry must not produce any draw calls',
			);
			nodeassert.deepStrictEqual(recorder.properties, {});
		});

		it("renders objectInfo's children while exposing its metadata", async function () {
			// objectInfo wraps a subtree with metadata: name, title and
			// description. The metadata travels on the wire alongside
			// the children.
			const primitive = (await vectorPrimitiveSample('objectInfo'))
				.primitives[0];
			nodeassert.strictEqual(primitive.type, 'objectInfo');
			nodeassert.strictEqual(typeof primitive.name, 'string');
			nodeassert.strictEqual(typeof primitive.title, 'string');
			nodeassert.strictEqual(typeof primitive.desc, 'string');
			nodeassert.strictEqual(primitive.children.length, 1);

			const recorder = new CanvasRecorder();
			const renderer = new cool.VectorPrimitiveRenderer();
			renderer.renderPrimitive(recorder as any, primitive);

			// Exactly one child fill, matching the wrapped triangle.
			nodeassert.strictEqual(recorder.countOf('fill'), 1);
		});

		it('applies the wire transparence to the children for unifiedTransparence', async function () {
			// unifiedTransparence carries one transparency value in
			// [0, 1] where 0 is fully opaque. The children draw with
			// (1 - transparence) as the canvas alpha, and the alpha
			// does not leak past the subtree.
			const primitive = (await vectorPrimitiveSample('unifiedTransparence'))
				.primitives[0];
			nodeassert.strictEqual(primitive.type, 'unifiedTransparence');
			const transparence: number = parseFloat(primitive.transparence);
			nodeassert.ok(
				transparence > 0 && transparence < 1,
				'the sample must be partly transparent',
			);

			const recorder = new CanvasRecorder();
			const renderer = new cool.VectorPrimitiveRenderer();
			renderer.renderPrimitive(recorder as any, primitive);

			// Use the per-call snapshot so we see the alpha as it was
			// when the child drew, not whatever the recorder saw last.
			const fill = recorder.findCall('fill');
			nodeassert.ok(fill, 'child fill missing');
			nodeassert.ok(
				Math.abs(fill!.properties.globalAlpha - (1 - transparence)) < 1e-9,
				'globalAlpha at fill time does not match 1 - transparence',
			);

			// Canvas state is balanced around the children.
			nodeassert.ok(recorder.findCall('save'), 'save not called');
			nodeassert.ok(recorder.findCall('restore'), 'restore not called');

			// The children render exactly once. A missing early return
			// in the dispatch would draw them again after the restore.
			nodeassert.strictEqual(recorder.countOf('fill'), 1);
		});

		it('skips exclusiveEditView entirely', async function () {
			// exclusiveEditView wraps content meant only for an
			// exclusive edit view. It does not appear in the
			// view-only output.
			const primitive = (await vectorPrimitiveSample('exclusiveEditView'))
				.primitives[0];
			nodeassert.strictEqual(primitive.type, 'exclusiveEditView');

			const recorder = new CanvasRecorder();
			const renderer = new cool.VectorPrimitiveRenderer();
			renderer.renderPrimitive(recorder as any, primitive);

			nodeassert.strictEqual(
				recorder.calls.length,
				0,
				'exclusiveEditView must not produce any draw calls',
			);
			nodeassert.deepStrictEqual(recorder.properties, {});
		});

		it('grays modifiedColor gray even without a scratch canvas', async function () {
			// The gray modifier maps to canvas filter "grayscale(1)".
			// The renderer wraps the children in save/restore so the
			// filter does not leak.
			ScratchCanvasStub.installUnavailable();
			const primitive = (await vectorPrimitiveSample('modifiedColorGray'))
				.primitives[0];
			nodeassert.strictEqual(primitive.type, 'modifiedColor');
			nodeassert.strictEqual(primitive.modifier, 'gray');

			const recorder = new CanvasRecorder();
			const renderer = new cool.VectorPrimitiveRenderer();
			renderer.renderPrimitive(recorder as any, primitive);

			nodeassert.strictEqual(recorder.properties.filter, 'grayscale(1)');
			nodeassert.ok(recorder.findCall('save'), 'save not called');
			nodeassert.ok(recorder.findCall('restore'), 'restore not called');
			// Exactly one child render. If the switch forgot the early
			// return, the child would draw a second time outside the
			// filter.
			nodeassert.strictEqual(recorder.countOf('fill'), 1);
		});

		it('inverts modifiedColor invert even without a scratch canvas', async function () {
			// The invert modifier maps to canvas filter "invert(1)".
			ScratchCanvasStub.installUnavailable();
			const primitive = (await vectorPrimitiveSample('modifiedColorInvert'))
				.primitives[0];
			nodeassert.strictEqual(primitive.type, 'modifiedColor');
			nodeassert.strictEqual(primitive.modifier, 'invert');

			const recorder = new CanvasRecorder();
			const renderer = new cool.VectorPrimitiveRenderer();
			renderer.renderPrimitive(recorder as any, primitive);

			nodeassert.strictEqual(recorder.properties.filter, 'invert(1)');
			nodeassert.strictEqual(recorder.countOf('fill'), 1);
		});

		it('grays modifiedColor gray without the canvas filter', async function () {
			ScratchCanvasStub.install();
			const primitive = (await vectorPrimitiveSample('modifiedColorGray'))
				.primitives[0];
			const recorder = new CanvasRecorder(200, 200);
			new cool.VectorPrimitiveRenderer().renderPrimitive(
				recorder as any,
				primitive,
			);

			// The child goes on the first canvas, and the second one
			// takes the saturation of a gray, which leaves only the
			// luminosity.
			const content = ScratchCanvasStub.recorder(0);
			const color = ScratchCanvasStub.recorder(1);
			nodeassert.strictEqual(content.countOf('fill'), 1);
			const gray = color
				.callsOf('fillRect')
				.find(
					(call) => call.properties.globalCompositeOperation === 'saturation',
				);
			nodeassert.ok(gray, 'the color is grayed by a blend');
			nodeassert.strictEqual(gray.properties.fillStyle, '#808080');
			nodeassert.ok(
				color
					.callsOf('drawImage')
					.some(
						(call) =>
							call.properties.globalCompositeOperation === 'color-dodge',
					),
				'the premultiplied color is divided by the alpha',
			);

			// The result lands once on the target, with no filter.
			nodeassert.strictEqual(recorder.countOf('drawImage'), 1);
			nodeassert.strictEqual(recorder.countOf('fill'), 0);
			nodeassert.strictEqual(recorder.properties.filter, undefined);
		});

		it('inverts modifiedColor invert without the canvas filter', async function () {
			ScratchCanvasStub.install();
			const primitive = (await vectorPrimitiveSample('modifiedColorInvert'))
				.primitives[0];
			const recorder = new CanvasRecorder(200, 200);
			new cool.VectorPrimitiveRenderer().renderPrimitive(
				recorder as any,
				primitive,
			);

			// The alpha as a gray level, less the premultiplied color, is
			// the premultiplied inverse.
			const color = ScratchCanvasStub.recorder(1);
			const alpha = ScratchCanvasStub.recorder(2);
			const difference = color
				.callsOf('drawImage')
				.find(
					(call) => call.properties.globalCompositeOperation === 'difference',
				);
			nodeassert.ok(difference, 'the color is inverted by a blend');
			nodeassert.strictEqual(difference.args[0], alpha.canvas);
			nodeassert.strictEqual(recorder.countOf('drawImage'), 1);
			nodeassert.strictEqual(recorder.properties.filter, undefined);
		});

		it('gives every pixel of a replaced subtree the replacement color', async function () {
			const primitive = (await vectorPrimitiveSample('modifiedColorReplace'))
				.primitives[0];
			nodeassert.strictEqual(primitive.type, 'modifiedColor');
			nodeassert.strictEqual(primitive.modifier, 'replace');
			nodeassert.strictEqual(typeof primitive.color, 'string');

			ScratchCanvasStub.install();
			const recorder = new CanvasRecorder(200, 200);
			new cool.VectorPrimitiveRenderer().renderPrimitive(
				recorder as any,
				primitive,
			);

			// The child keeps its shape and alpha, in the replacement color.
			const content = ScratchCanvasStub.recorder(0);
			nodeassert.strictEqual(content.countOf('fill'), 1);
			const replace = content.findCall('fillRect');
			nodeassert.ok(replace, 'the color is filled over the child');
			nodeassert.strictEqual(
				replace.properties.globalCompositeOperation,
				'source-in',
			);
			nodeassert.strictEqual(replace.properties.fillStyle, primitive.color);
			nodeassert.strictEqual(content.countOf('getImageData'), 0);
			nodeassert.strictEqual(recorder.countOf('drawImage'), 1);
			nodeassert.strictEqual(recorder.countOf('fill'), 0);
		});

		it('mixes the interpolation color over a subtree by one minus the value', async function () {
			const primitive = (await vectorPrimitiveSample('modifiedColorParameters'))
				.primitives[0];
			nodeassert.strictEqual(primitive.modifier, 'interpolate');

			ScratchCanvasStub.install();
			const recorder = new CanvasRecorder(200, 200);
			new cool.VectorPrimitiveRenderer().renderPrimitive(
				recorder as any,
				primitive,
			);

			const mix = ScratchCanvasStub.recorder(0).findCall('fillRect');
			nodeassert.ok(mix, 'the color is filled over the child');
			nodeassert.strictEqual(
				mix.properties.globalCompositeOperation,
				'source-atop',
			);
			nodeassert.strictEqual(mix.properties.globalAlpha, 1 - primitive.value);
			nodeassert.strictEqual(mix.properties.fillStyle, primitive.color);
			nodeassert.strictEqual(recorder.countOf('drawImage'), 1);
		});

		it('changes the pixels of a subtree for every other modifier', async function () {
			// Each modifier after the interpolation colors a red triangle as
			// the engine does, and the result is drawn once.
			const primitives = (
				await vectorPrimitiveSample('modifiedColorParameters')
			).primitives.slice(1);
			nodeassert.deepStrictEqual(
				primitives.map((primitive: any) => primitive.modifier),
				[
					'saturate',
					'hueRotate',
					'matrix',
					'black_and_white',
					'gamma',
					'luminance_to_alpha',
				],
			);

			for (const primitive of primitives) {
				ScratchCanvasStub.install();
				ScratchCanvasStub.imagePixels[0] = [255, 0, 0, 255];
				const recorder = new CanvasRecorder(200, 200);
				new cool.VectorPrimitiveRenderer().renderPrimitive(
					recorder as any,
					primitive,
				);
				ScratchCanvasStub.uninstall();

				const content = ScratchCanvasStub.recorder(0);
				nodeassert.strictEqual(content.countOf('fill'), 1, primitive.modifier);
				const put = content.findCall('putImageData');
				nodeassert.ok(put, primitive.modifier + ' writes its pixels back');
				const expected = [1, 0, 0];
				cool.VectorColorModifiers.colorChange(primitive)!(expected);
				nodeassert.deepStrictEqual(
					Array.from(put.args[0].data.slice(0, 4)),
					[
						...expected.map((channel) =>
							Math.round(Math.min(Math.max(channel, 0), 1) * 255),
						),
						255,
					],
					primitive.modifier,
				);
				nodeassert.strictEqual(
					recorder.countOf('drawImage'),
					1,
					primitive.modifier,
				);
				nodeassert.strictEqual(recorder.countOf('fill'), 0, primitive.modifier);
			}
		});

		it('draws a modified subtree through an SVG filter where the canvas has one', async function () {
			const primitive = (await vectorPrimitiveSample('modifiedColorParameters'))
				.primitives[1];
			nodeassert.strictEqual(primitive.modifier, 'saturate');

			ScratchCanvasStub.install();
			const recorder = new CanvasRecorder(200, 200);
			// A canvas that has filters starts with none.
			(recorder as any).filter = 'none';
			const renderer = new cool.VectorPrimitiveRenderer();
			renderer.renderPrimitive(recorder as any, primitive);

			// The colors change on the way to the target, not on the scratch.
			const content = ScratchCanvasStub.recorder(0);
			nodeassert.strictEqual(content.countOf('fill'), 1);
			nodeassert.strictEqual(content.countOf('getImageData'), 0);
			nodeassert.strictEqual(content.countOf('putImageData'), 0);
			nodeassert.strictEqual(recorder.countOf('drawImage'), 1);
			const draw = recorder.findCall('drawImage')!;
			const match = /^url\(#(.+)\)$/.exec(draw.properties.filter);
			nodeassert.ok(match, 'drawn through an SVG filter');
			nodeassert.ok(document.getElementById(match![1]));

			renderer.releaseColorChangedImages();
		});

		it('fills the bounds for filledRectangle', async function () {
			const primitive = (await vectorPrimitiveSample('filledRectangle'))
				.primitives[0];
			nodeassert.strictEqual(primitive.type, 'filledRectangle');

			const recorder = new CanvasRecorder();
			const renderer = new cool.VectorPrimitiveRenderer();
			renderer.renderPrimitive(recorder as any, primitive);

			const fillRect = recorder.findCall('fillRect');
			nodeassert.ok(fillRect, 'fillRect not called');
			const bounds = cool.Range2D.fromArray(primitive.bounds);
			nodeassert.deepStrictEqual(fillRect.args, [
				bounds.minX,
				bounds.minY,
				bounds.width,
				bounds.height,
			]);
			nodeassert.strictEqual(recorder.properties.fillStyle, primitive.color);
			nodeassert.ok(recorder.findCall('save'), 'save not called');
			nodeassert.ok(recorder.findCall('restore'), 'restore not called');
		});

		it('strokes the bounds for lineRectangle', async function () {
			const primitive = (await vectorPrimitiveSample('lineRectangle'))
				.primitives[0];
			nodeassert.strictEqual(primitive.type, 'lineRectangle');

			const recorder = new CanvasRecorder();
			const renderer = new cool.VectorPrimitiveRenderer();
			renderer.renderPrimitive(recorder as any, primitive);

			const strokeRect = recorder.findCall('strokeRect');
			nodeassert.ok(strokeRect, 'strokeRect not called');
			const bounds = cool.Range2D.fromArray(primitive.bounds);
			nodeassert.deepStrictEqual(strokeRect.args, [
				bounds.minX,
				bounds.minY,
				bounds.width,
				bounds.height,
			]);
			nodeassert.strictEqual(recorder.properties.strokeStyle, primitive.color);
			nodeassert.strictEqual(recorder.properties.lineWidth, 1);
			nodeassert.ok(recorder.findCall('save'), 'save not called');
			nodeassert.ok(recorder.findCall('restore'), 'restore not called');
		});

		it('strokes a single line from start to end', async function () {
			const primitive = (await vectorPrimitiveSample('singleLine'))
				.primitives[0];
			nodeassert.strictEqual(primitive.type, 'singleLine');

			const recorder = new CanvasRecorder();
			const renderer = new cool.VectorPrimitiveRenderer();
			renderer.renderPrimitive(recorder as any, primitive);

			const moveTo = recorder.findCall('moveTo');
			const lineTo = recorder.findCall('lineTo');
			nodeassert.ok(moveTo, 'moveTo not called');
			nodeassert.ok(lineTo, 'lineTo not called');
			nodeassert.deepStrictEqual(moveTo.args, [
				primitive.startX,
				primitive.startY,
			]);
			nodeassert.deepStrictEqual(lineTo.args, [primitive.endX, primitive.endY]);
			nodeassert.ok(recorder.findCall('stroke'), 'stroke not called');
			nodeassert.strictEqual(recorder.properties.strokeStyle, primitive.color);
			nodeassert.strictEqual(recorder.properties.lineWidth, 1);
		});
	});

	describe('Effects', function () {
		const assert = require('assert');

		beforeEach(function () {
			ScratchCanvasStub.install();
		});

		function render(primitive: any): CanvasRecorder {
			const context = new CanvasRecorder(200, 200);
			(context as any).globalAlpha = 1;
			new cool.VectorPrimitiveRenderer().renderPrimitive(
				context as any,
				primitive,
			);
			return context;
		}

		it('draws the reflection the engine writes, flipped and faded', async function () {
			// The content of the engine is 100 by 50, and its reflection starts 10
			// below it and fades from half transparent to clear at 0.9.
			const primitive = (await vectorPrimitiveSample('reflection'))
				.primitives[0];
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

		it('draws the invert the engine writes over the area it covers', async function () {
			const primitive = (await vectorPrimitiveSample('invert')).primitives[0];
			render(primitive);

			// The bounds are 100 by 50, with a pixel more on each side for
			// the antialiased edge.
			const content = ScratchCanvasStub.recorder(0);
			assert.strictEqual(content.canvas.width, 102);
			assert.strictEqual(content.canvas.height, 52);
			assert.strictEqual(content.countOf('putImageData'), 1);
		});
	});

	describe('Arrow heads', function () {
		const assert = require('assert');

		it('draws the line and the head that the engine sends', async function () {
			const primitive = (await vectorPrimitiveSample('polygonStrokeArrow'))
				.primitives[0];
			assert.strictEqual(primitive.type, 'polygonStrokeArrow');

			const context = new CanvasRecorder(200, 200);
			new cool.VectorPrimitiveRenderer().renderPrimitive(
				context as any,
				primitive,
			);

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
});
