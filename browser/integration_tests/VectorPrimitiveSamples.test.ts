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

import { strict as nodeassert } from 'assert';
import { SocketClient } from './helper/SocketClient';
import { BASE_URL, sampleUrl } from './helper/Environment';

/// Asserts that actual is a number within tolerance of expected.
function assertNear(actual: number, expected: number, tolerance = 1e-9): void {
	nodeassert.ok(
		typeof actual === 'number' && Math.abs(actual - expected) <= tolerance,
		actual + ' is not ' + expected,
	);
}

/// Whether an SVG path string has a curve command.
function hasCurve(path: string): boolean {
	return /[csqt]/.test(path);
}

describe('Integration: the JSON of each primitive type', function () {
	this.timeout(60000);

	let client: SocketClient;

	before(async function () {
		client = await SocketClient.connectAndLoad(
			BASE_URL,
			sampleUrl('impress.fodp'),
		);
	});

	after(function () {
		if (client) client.close();
	});

	/// The primitives of the sample with the given name, which the test cannot go on without.
	async function primitivesOf(name: string): Promise<any[]> {
		return (await client.primitiveSample(name)).primitives;
	}

	it('answers a sample name it does not know with no primitives', async function () {
		const answer = await client.vectorPrimitiveSample('noSuchSample');
		nodeassert.strictEqual(answer.name, 'noSuchSample');
		nodeassert.strictEqual(answer.primitives, undefined);
	});

	/// An opaque background leaves the transparency out.
	it('writes the color of an opaque background', async function () {
		const [primitive] = await primitivesOf('backgroundColor');
		nodeassert.strictEqual(primitive.type, 'backgroundcolor');
		nodeassert.strictEqual(primitive.color, '#ff0000');
		nodeassert.strictEqual(primitive.transparency, undefined);
	});

	it('writes the transparency of a background', async function () {
		const [primitive] = await primitivesOf('backgroundColorTransparent');
		nodeassert.strictEqual(primitive.type, 'backgroundcolor');
		nodeassert.strictEqual(primitive.color, '#00ffff');
		assertNear(primitive.transparency, 0.25);
	});

	it('writes a filled polygon with its color and path', async function () {
		const [primitive] = await primitivesOf('polyPolygonColor');
		nodeassert.strictEqual(primitive.type, 'polyPolygonColor');
		nodeassert.strictEqual(primitive.color, '#00ff00');
		nodeassert.ok(primitive.path);
	});

	it('writes a filled polygon with a hole as one path', async function () {
		const [primitive] = await primitivesOf('polyPolygonColorWithHole');
		nodeassert.strictEqual(primitive.type, 'polyPolygonColor');
		nodeassert.strictEqual(primitive.color, '#ff0000');
		nodeassert.ok(primitive.path);
	});

	it('writes the transparency of a filled polygon', async function () {
		const [primitive] = await primitivesOf('polyPolygonRGBA');
		nodeassert.strictEqual(primitive.type, 'polyPolygonRGBA');
		nodeassert.strictEqual(primitive.color, '#0000ff');
		assertNear(primitive.transparency, 0.25);
		nodeassert.ok(primitive.path);
	});

	it('writes the color, width, join and cap of a line', async function () {
		const [primitive] = await primitivesOf('polygonStroke');
		nodeassert.strictEqual(primitive.type, 'polygonStroke');
		nodeassert.strictEqual(primitive.line.color, '#000000');
		assertNear(primitive.line.width, 2);
		nodeassert.strictEqual(primitive.line.linejoin, 'miter');
		nodeassert.strictEqual(primitive.line.linecap, 'butt');
		nodeassert.ok(primitive.path);
	});

	/// A line with no join goes out as the areas the drawing layer fills, one for each edge. The
	/// round caps are at the two ends of the line only, so the middle area has no curve.
	it('writes a line with no join as one area for each edge', async function () {
		const [primitive] = await primitivesOf('polygonStrokeNoJoin');
		nodeassert.strictEqual(primitive.type, 'group');
		nodeassert.strictEqual(primitive.children[0].type, 'group');
		const edges = primitive.children[0].children;
		nodeassert.strictEqual(edges.length, 3);
		edges.forEach((edge: any, index: number) => {
			nodeassert.strictEqual(edge.type, 'polyPolygonColor');
			nodeassert.strictEqual(hasCurve(edge.path), index !== 1);
		});
	});

	it('writes the dash pattern of a line', async function () {
		const [primitive] = await primitivesOf('polygonStrokeDashed');
		nodeassert.strictEqual(primitive.type, 'polygonStroke');
		nodeassert.strictEqual(primitive.stroke.dotDashArray.length, 2);
	});

	/// Two disjoint lines stroked by one primitive.
	it('writes the line of a stroked polygon with several parts', async function () {
		const [primitive] = await primitivesOf('polyPolygonStroke');
		nodeassert.strictEqual(primitive.type, 'polyPolygonStroke');
		nodeassert.strictEqual(primitive.line.color, '#000000');
		assertNear(primitive.line.width, 2);
		nodeassert.strictEqual(primitive.line.linejoin, 'miter');
		nodeassert.strictEqual(primitive.line.linecap, 'butt');
		nodeassert.ok(primitive.path);
	});

	/// The mapping puts 0 at the start of the gradient and 1 at its end, so the unit gradient
	/// spans the width of the rectangle from (100, 100) to (300, 200).
	it('writes the stops, spread and mapping of a linear gradient', async function () {
		const [primitive] = await primitivesOf('svgLinearGradient');
		nodeassert.strictEqual(primitive.type, 'svgLinearGradient');
		nodeassert.strictEqual(primitive.spreadMethod, 'repeat');
		nodeassert.strictEqual(primitive.colorStops.length, 2);
		assertNear(primitive.colorStops[1].opacity, 0.5);
		nodeassert.strictEqual(primitive.unitToObject.length, 6);
		assertNear(primitive.unitToObject[0], 200);
		assertNear(primitive.unitToObject[4], 100);
		assertNear(primitive.unitToObject[5], 100);
	});

	/// The line is cut back under the head, and the point of the head is on the end of the line.
	it('writes an arrow as its line and its filled head', async function () {
		const [primitive] = await primitivesOf('polygonStrokeArrow');
		nodeassert.strictEqual(primitive.type, 'polygonStrokeArrow');
		nodeassert.strictEqual(primitive.path, undefined);
		const group = primitive.children[0];
		nodeassert.strictEqual(group.type, 'group');
		nodeassert.strictEqual(group.children.length, 2);

		nodeassert.strictEqual(group.children[0].type, 'polygonStroke');
		nodeassert.strictEqual(group.children[0].line.color, '#0000ff');

		nodeassert.strictEqual(group.children[1].type, 'polyPolygonColor');
		nodeassert.strictEqual(group.children[1].color, '#0000ff');
		assertNear(group.children[1].bounds[2], 1000, 1e-3);
	});

	it('writes the children of a group in their order', async function () {
		const [primitive] = await primitivesOf('group');
		nodeassert.strictEqual(primitive.type, 'group');
		nodeassert.strictEqual(primitive.children.length, 2);
		nodeassert.strictEqual(primitive.children[0].type, 'polyPolygonColor');
		nodeassert.strictEqual(primitive.children[1].type, 'polygonStroke');
	});

	it('writes an empty group with no children', async function () {
		const [primitive] = await primitivesOf('groupEmpty');
		nodeassert.strictEqual(primitive.type, 'group');
		nodeassert.strictEqual((primitive.children || []).length, 0);
	});

	it('writes the matrix and the children of a transform', async function () {
		const [primitive] = await primitivesOf('transform');
		nodeassert.strictEqual(primitive.type, 'transform');
		nodeassert.strictEqual(primitive.matrix.length, 6);
		nodeassert.strictEqual(primitive.children.length, 1);
		nodeassert.strictEqual(primitive.children[0].type, 'polyPolygonColor');
	});

	/// Content that is there to be hit and not seen travels, so a reader can hit-test it.
	it('writes the children of hidden geometry', async function () {
		const [primitive] = await primitivesOf('hiddenGeometry');
		nodeassert.strictEqual(primitive.type, 'hiddenGeometry');
		nodeassert.strictEqual(primitive.children[0].type, 'polyPolygonColor');
	});

	/// The JSON serves an editing view, so the content that such a view alone shows travels.
	it('writes the children that an editing view alone shows', async function () {
		const [primitive] = await primitivesOf('exclusiveEditView');
		nodeassert.strictEqual(primitive.type, 'exclusiveEditView');
		nodeassert.strictEqual(primitive.children.length, 1);
		nodeassert.strictEqual(primitive.children[0].type, 'polyPolygonColor');
	});

	it('writes the name, title and description of an object', async function () {
		const [primitive] = await primitivesOf('objectInfo');
		nodeassert.strictEqual(primitive.type, 'objectInfo');
		nodeassert.strictEqual(primitive.name, 'Rectangle 1');
		nodeassert.strictEqual(primitive.title, 'My title');
		nodeassert.strictEqual(primitive.desc, 'My description');
		nodeassert.strictEqual(primitive.children.length, 1);
		nodeassert.strictEqual(primitive.children[0].type, 'polyPolygonColor');
	});

	it('writes the transparence of a transparent subtree', async function () {
		const [primitive] = await primitivesOf('unifiedTransparence');
		nodeassert.strictEqual(primitive.type, 'unifiedTransparence');
		nodeassert.notStrictEqual(primitive.transparence, undefined);
		nodeassert.strictEqual(primitive.children.length, 1);
		nodeassert.strictEqual(primitive.children[0].type, 'polyPolygonColor');
	});

	it('writes a gray color modifier', async function () {
		const [primitive] = await primitivesOf('modifiedColorGray');
		nodeassert.strictEqual(primitive.type, 'modifiedColor');
		nodeassert.strictEqual(primitive.modifier, 'gray');
		nodeassert.strictEqual(primitive.children.length, 1);
	});

	it('writes an invert color modifier', async function () {
		const [primitive] = await primitivesOf('modifiedColorInvert');
		nodeassert.strictEqual(primitive.type, 'modifiedColor');
		nodeassert.strictEqual(primitive.modifier, 'invert');
	});

	it('writes the color that a replace modifier gives every pixel', async function () {
		const [primitive] = await primitivesOf('modifiedColorReplace');
		nodeassert.strictEqual(primitive.type, 'modifiedColor');
		nodeassert.strictEqual(primitive.modifier, 'replace');
		nodeassert.strictEqual(primitive.color, '#ff0000');
	});

	/// The luminance and contrast modifier comes with the adjusted graphic.
	it('writes the parameters of each color modifier that takes them', async function () {
		const primitives = await primitivesOf('modifiedColorParameters');

		nodeassert.strictEqual(primitives[0].modifier, 'interpolate');
		nodeassert.strictEqual(primitives[0].color, '#0000ff');
		assertNear(primitives[0].value, 0.25);

		// Saturate and hue rotate go out as their 3x3 matrix, row by row.
		nodeassert.strictEqual(primitives[1].modifier, 'saturate');
		nodeassert.strictEqual(primitives[1].matrix.length, 9);
		assertNear(primitives[1].matrix[0], 0.213 + 0.787 * 0.5);
		nodeassert.strictEqual(primitives[2].modifier, 'hueRotate');
		nodeassert.strictEqual(primitives[2].matrix.length, 9);

		// The general matrix keeps its 4x5 values as given.
		nodeassert.strictEqual(primitives[3].modifier, 'matrix');
		nodeassert.strictEqual(primitives[3].matrix.length, 20);
		assertNear(primitives[3].matrix[1], 1);

		nodeassert.strictEqual(primitives[4].modifier, 'black_and_white');
		assertNear(primitives[4].value, 0.25);

		nodeassert.strictEqual(primitives[5].modifier, 'gamma');
		assertNear(primitives[5].value, 2);

		nodeassert.strictEqual(primitives[6].modifier, 'luminance_to_alpha');
	});

	it('writes the color and bounds of a filled rectangle', async function () {
		const [primitive] = await primitivesOf('filledRectangle');
		nodeassert.strictEqual(primitive.type, 'filledRectangle');
		nodeassert.strictEqual(primitive.color, '#0000ff');
		nodeassert.strictEqual(primitive.bounds.length, 4);
	});

	it('writes the color and bounds of an outlined rectangle', async function () {
		const [primitive] = await primitivesOf('lineRectangle');
		nodeassert.strictEqual(primitive.type, 'lineRectangle');
		nodeassert.strictEqual(primitive.color, '#ff0000');
		nodeassert.strictEqual(primitive.bounds.length, 4);
	});

	it('writes the color and ends of a single line', async function () {
		const [primitive] = await primitivesOf('singleLine');
		nodeassert.strictEqual(primitive.type, 'singleLine');
		nodeassert.strictEqual(primitive.color, '#ff0000');
		assertNear(primitive.startX, 10);
		assertNear(primitive.startY, 20);
		assertNear(primitive.endX, 110);
		assertNear(primitive.endY, 70);
	});

	/// The mark lies 3% of the font height below the baseline, and has no wave of its own.
	it('writes the line under a misspelled word', async function () {
		const [primitive] = await primitivesOf('wrongSpell');
		nodeassert.strictEqual(primitive.type, 'wrongSpell');
		nodeassert.strictEqual(primitive.color, '#ff0000');
		nodeassert.strictEqual(primitive.children, undefined);
		assertNear(primitive.fontHeight, 200);
		assertNear(primitive.startX, 100);
		assertNear(primitive.startY, 506);
		assertNear(primitive.endX, 500);
		assertNear(primitive.endY, 506);
	});

	it('writes the text, font and color of a text portion', async function () {
		const [primitive] = await primitivesOf('textSimplePortion');
		nodeassert.strictEqual(primitive.type, 'textSimplePortion');
		nodeassert.strictEqual(primitive.text, 'Hello');
		nodeassert.strictEqual(primitive.textPosition, 0);
		nodeassert.strictEqual(primitive.textLength, 5);
		nodeassert.strictEqual(primitive.fontcolor, '#000000');
		nodeassert.strictEqual(primitive.familyname, 'Liberation Sans');
		// The normal weight of the font weight enum.
		nodeassert.strictEqual(primitive.weight, 5);
		assertNear(primitive.fontSize, 20);
		nodeassert.strictEqual(primitive.matrix.length, 6);
	});

	/// The highlight reaches from above the baseline at y 100 to below it, over the width of the
	/// advances from x 50.
	it('writes the area of the highlight behind a text', async function () {
		const [primitive] = await primitivesOf('textHighlight');
		nodeassert.strictEqual(primitive.fillcolor, '#ffff00');
		nodeassert.ok(primitive.highlight.path);
		const [left, top, right, bottom] = primitive.highlight.bounds;
		assertNear(left, 50, 1e-6);
		assertNear(right, 100, 1e-6);
		nodeassert.ok(top < 100, 'the highlight starts at ' + top);
		nodeassert.ok(bottom > 100, 'the highlight ends at ' + bottom);
	});

	it('writes no highlight for a text with no fill color', async function () {
		const [primitive] = await primitivesOf('textNoHighlight');
		nodeassert.strictEqual(primitive.fillcolor, undefined);
		nodeassert.strictEqual(primitive.highlight, undefined);
	});

	it('writes the underline and strikeout of a decorated text', async function () {
		const [primitive] = await primitivesOf('textDecoratedPortion');
		nodeassert.strictEqual(primitive.type, 'textDecoratedPortion');
		nodeassert.strictEqual(primitive.text, 'Hello');
		// The single line of the text line and strikeout enums.
		nodeassert.strictEqual(primitive.underline, 1);
		nodeassert.strictEqual(primitive.underlineColor, '#000000');
		nodeassert.strictEqual(primitive.strikeout, 1);
	});

	/// The red underline comes first, then the black strikeout.
	it('writes the decorations as strokes in their own colors', async function () {
		const [primitive] = await primitivesOf('textDecoratedPortionDecorations');
		nodeassert.strictEqual(primitive.type, 'textDecoratedPortion');
		nodeassert.strictEqual(primitive.decorations.length, 2);
		nodeassert.strictEqual(primitive.decorations[0].type, 'polygonStroke');
		nodeassert.strictEqual(primitive.decorations[0].line.color, '#ff0000');
		nodeassert.strictEqual(primitive.decorations[1].type, 'polygonStroke');
		nodeassert.strictEqual(primitive.decorations[1].line.color, '#000000');
		nodeassert.strictEqual(primitive.decorationsInTextColor, undefined);
	});

	/// An underline in the automatic color takes the color the text gets, while the overline has
	/// a fixed color.
	it('writes the decorations in the automatic color apart', async function () {
		const [primitive] = await primitivesOf(
			'textDecoratedPortionDecorationsInTextColor',
		);
		nodeassert.strictEqual(primitive.fontcolor, 'auto');
		nodeassert.strictEqual(primitive.decorations.length, 1);
		nodeassert.strictEqual(primitive.decorations[0].line.color, '#0000ff');
		// A double underline is a group of the stroke and the stroke moved down.
		nodeassert.strictEqual(primitive.decorationsInTextColor.length, 1);
		const double = primitive.decorationsInTextColor[0];
		nodeassert.strictEqual(double.type, 'group');
		nodeassert.strictEqual(double.children.length, 2);
		nodeassert.strictEqual(double.children[0].type, 'polygonStroke');
		nodeassert.strictEqual(double.children[1].type, 'transform');
	});

	/// The pixels travel apart from the primitive, named by the checksum.
	it('writes the matrix, pixel size and checksum of a bitmap', async function () {
		const [primitive] = await primitivesOf('bitmap');
		nodeassert.strictEqual(primitive.type, 'bitmap');
		nodeassert.strictEqual(primitive.matrix.length, 6);
		nodeassert.strictEqual(primitive.width, 4);
		nodeassert.strictEqual(primitive.height, 4);
		nodeassert.notStrictEqual(primitive.checksum, undefined);
	});

	it('writes the transparency of a bitmap', async function () {
		const [primitive] = await primitivesOf('bitmapAlpha');
		nodeassert.strictEqual(primitive.type, 'bitmapAlpha');
		nodeassert.strictEqual(primitive.matrix.length, 6);
		nodeassert.strictEqual(primitive.width, 4);
		nodeassert.strictEqual(primitive.height, 4);
		nodeassert.notStrictEqual(primitive.checksum, undefined);
		assertNear(primitive.transparency, 0.25);
	});

	it('writes a raster graphic like a bitmap', async function () {
		const [primitive] = await primitivesOf('graphic');
		nodeassert.strictEqual(primitive.type, 'graphic');
		nodeassert.strictEqual(primitive.matrix.length, 6);
		nodeassert.strictEqual(primitive.width, 8);
		nodeassert.strictEqual(primitive.height, 6);
		nodeassert.notStrictEqual(primitive.checksum, undefined);
		nodeassert.strictEqual(primitive.vector, undefined);
		nodeassert.strictEqual(primitive.children, undefined);
	});

	/// The image rectangle is where the whole image lies, in fractions of the unit square of the
	/// frame. The crop is 100 left, 200 top, 50 right and 25 bottom of an image 1000 by 800.
	it('writes the rectangle that the whole of a cropped graphic maps onto', async function () {
		const [primitive] = await primitivesOf('graphicCrop');
		nodeassert.strictEqual(primitive.type, 'graphic');
		assertNear(primitive.imageRect.x, -100 / 850);
		assertNear(primitive.imageRect.y, -200 / 575);
		assertNear(primitive.imageRect.width, 1 + 150 / 850);
		assertNear(primitive.imageRect.height, 1 + 225 / 575);
	});

	it('writes the same image rectangle for a cropped graphic upright and turned', async function () {
		const [upright, rotated] = await primitivesOf('graphicCropRotated');
		for (const field of ['x', 'y', 'width', 'height']) {
			nodeassert.strictEqual(typeof upright.imageRect[field], 'number');
			assertNear(rotated.imageRect[field], upright.imageRect[field], 1e-6);
		}
	});

	/// The angle is in tenths of a degree.
	it('writes the rotation of a graphic', async function () {
		const [primitive] = await primitivesOf('graphicRotation');
		nodeassert.strictEqual(primitive.type, 'graphic');
		nodeassert.strictEqual(primitive.rotation, 450);
	});

	/// The alpha is a byte, 0 transparent and 255 opaque.
	it('writes the alpha of a graphic', async function () {
		const [primitive] = await primitivesOf('graphicAlpha');
		nodeassert.strictEqual(primitive.type, 'graphic');
		nodeassert.strictEqual(primitive.alpha, 128);
	});

	/// Bit 0 is the horizontal mirror and bit 1 the vertical one.
	it('writes the mirror axes of a graphic', async function () {
		const [primitive] = await primitivesOf('graphicMirror');
		nodeassert.strictEqual(primitive.type, 'graphic');
		nodeassert.strictEqual(primitive.mirror, 3);
	});

	it('writes the greys draw mode of a graphic as the gray modifier', async function () {
		const [primitive] = await primitivesOf('graphicDrawMode');
		nodeassert.strictEqual(primitive.type, 'graphic');
		nodeassert.strictEqual(primitive.colorModifiers.length, 1);
		nodeassert.strictEqual(primitive.colorModifiers[0].modifier, 'gray');
	});

	it('writes the watermark draw mode as a luminance and contrast change', async function () {
		const [primitive] = await primitivesOf('graphicWatermark');
		nodeassert.strictEqual(primitive.colorModifiers.length, 1);
		const modifier = primitive.colorModifiers[0];
		nodeassert.strictEqual(modifier.modifier, 'RGBLuminanceContrast');
		assertNear(modifier.luminance, 0.5);
		assertNear(modifier.contrast, -0.7);
	});

	it('writes every picture adjustment of a graphic in the order they apply', async function () {
		const [primitive] = await primitivesOf('graphicAdjusted');
		nodeassert.strictEqual(primitive.type, 'graphic');
		nodeassert.strictEqual(primitive.vector, undefined);
		const modifiers = primitive.colorModifiers;
		nodeassert.strictEqual(modifiers.length, 4);
		nodeassert.strictEqual(modifiers[0].modifier, 'black_and_white');
		assertNear(modifiers[0].value, 0.5);
		nodeassert.strictEqual(modifiers[1].modifier, 'RGBLuminanceContrast');
		assertNear(modifiers[1].red, 0.05);
		assertNear(modifiers[1].green, -0.15);
		assertNear(modifiers[1].blue, 0.25);
		assertNear(modifiers[1].luminance, 0.2);
		assertNear(modifiers[1].contrast, -0.1);
		nodeassert.strictEqual(modifiers[2].modifier, 'gamma');
		assertNear(modifiers[2].value, 2);
		nodeassert.strictEqual(modifiers[3].modifier, 'invert');
	});

	it('writes no modifiers for a graphic in its own colors', async function () {
		const [primitive] = await primitivesOf('graphicUnadjusted');
		nodeassert.strictEqual(primitive.type, 'graphic');
		nodeassert.strictEqual(primitive.colorModifiers, undefined);
	});

	/// The SVG data travels apart from the primitive, named by the checksum, like raster pixels.
	it('writes an SVG graphic like a raster graphic', async function () {
		const [primitive] = await primitivesOf('graphicSvg');
		nodeassert.strictEqual(primitive.type, 'graphic');
		nodeassert.strictEqual(primitive.matrix.length, 6);
		nodeassert.notStrictEqual(primitive.checksum, undefined);
		nodeassert.strictEqual(primitive.vector, undefined);
		nodeassert.strictEqual(primitive.children, undefined);
		nodeassert.strictEqual(primitive.colorModifiers, undefined);
	});

	it('writes the color modifiers of an SVG graphic', async function () {
		const [primitive] = await primitivesOf('graphicSvgAdjusted');
		nodeassert.strictEqual(primitive.type, 'graphic');
		nodeassert.strictEqual(primitive.vector, undefined);
		nodeassert.strictEqual(primitive.colorModifiers.length, 2);
		nodeassert.strictEqual(primitive.colorModifiers[0].modifier, 'gray');
		nodeassert.strictEqual(primitive.colorModifiers[1].modifier, 'invert');
	});

	it('writes the crop, rotation, mirror and alpha of an SVG graphic, and no pixel size', async function () {
		const [primitive] = await primitivesOf('graphicSvgCropRotateMirrorAlpha');
		nodeassert.strictEqual(primitive.type, 'graphic');
		nodeassert.strictEqual(primitive.vector, undefined);
		nodeassert.strictEqual(primitive.children, undefined);
		nodeassert.notStrictEqual(primitive.checksum, undefined);
		nodeassert.notStrictEqual(primitive.imageRect, undefined);
		nodeassert.strictEqual(primitive.rotation, 450);
		nodeassert.strictEqual(primitive.mirror, 1);
		nodeassert.strictEqual(primitive.alpha, 128);
		nodeassert.strictEqual(primitive.width, undefined);
		nodeassert.strictEqual(primitive.height, undefined);
	});

	/// An EMF graphic is decomposed, and its children are already in the coordinates of the
	/// page, so there is nothing to place or to fetch at the level of the graphic.
	it('writes an EMF graphic as its decomposed children', async function () {
		const [primitive] = await primitivesOf('graphicEmf');
		nodeassert.strictEqual(primitive.type, 'graphic');
		nodeassert.strictEqual(primitive.vector, true);
		nodeassert.ok(primitive.children.length > 0);
		nodeassert.strictEqual(primitive.matrix, undefined);
		nodeassert.strictEqual(primitive.checksum, undefined);
	});

	it('writes the color and points of a point array', async function () {
		const [primitive] = await primitivesOf('pointArray');
		nodeassert.strictEqual(primitive.type, 'pointArray');
		nodeassert.strictEqual(primitive.color, '#ff0000');
		nodeassert.strictEqual(primitive.points.length, 3);
		assertNear(primitive.points[0].x, 10);
		assertNear(primitive.points[0].y, 10);
		assertNear(primitive.points[2].x, 30);
	});

	it('writes the clip path and the children of a mask', async function () {
		const [primitive] = await primitivesOf('mask');
		nodeassert.strictEqual(primitive.type, 'mask');
		nodeassert.ok(primitive.clip);
		nodeassert.strictEqual(primitive.children.length, 1);
		nodeassert.strictEqual(primitive.children[0].type, 'polyPolygonColor');
	});

	/// The content is 100 by 50 and the reflection starts 10 below it, so the browser blurs and
	/// fades it at its own zoom.
	it('writes a reflection with its content, fade and blur', async function () {
		const [primitive] = await primitivesOf('reflection');
		nodeassert.strictEqual(primitive.type, 'reflection');

		// Flipped at y 50 and moved down by 10, so y becomes 110 - y.
		nodeassert.strictEqual(primitive.matrix.length, 6);
		assertNear(primitive.matrix[0], 1);
		assertNear(primitive.matrix[3], -1);
		assertNear(primitive.matrix[4], 0);
		assertNear(primitive.matrix[5], 110);

		assertNear(primitive.range[1], 60);
		assertNear(primitive.range[3], 110);

		assertNear(primitive.startTransparency, 0.5);
		assertNear(primitive.startPosition, 0);
		assertNear(primitive.endTransparency, 1);
		assertNear(primitive.endPosition, 0.9);
		assertNear(primitive.blur, 4);

		// The bounds include the blur.
		assertNear(primitive.bounds[0], -4);
		assertNear(primitive.bounds[1], 56);
		assertNear(primitive.bounds[2], 104);
		assertNear(primitive.bounds[3], 114);

		nodeassert.strictEqual(primitive.children.length, 1);
		nodeassert.strictEqual(primitive.children[0].type, 'polyPolygonColor');
	});

	it('writes no children for a fully transparent reflection', async function () {
		const [primitive] = await primitivesOf('reflectionInvisible');
		nodeassert.strictEqual(primitive.type, 'reflection');
		nodeassert.strictEqual(primitive.children, undefined);
	});

	/// The bounds are the area whose colors the content painted with XOR changes.
	it('writes the bounds and the children of an invert', async function () {
		const [primitive] = await primitivesOf('invert');
		nodeassert.strictEqual(primitive.type, 'invert');
		assertNear(primitive.bounds[0], 10);
		assertNear(primitive.bounds[1], 20);
		assertNear(primitive.bounds[2], 110);
		assertNear(primitive.bounds[3], 70);
		nodeassert.strictEqual(primitive.children.length, 1);
		nodeassert.strictEqual(primitive.children[0].type, 'polyPolygonColor');
		nodeassert.strictEqual(primitive.children[0].color, '#ffffff');
	});
});
