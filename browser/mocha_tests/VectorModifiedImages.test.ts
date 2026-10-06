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

describe('VectorModifiedImages', function () {
	const assert = require('assert');

	const invert = [{ modifier: 'invert' }];

	beforeEach(function () {
		ScratchCanvasStub.install();
	});

	afterEach(function () {
		ScratchCanvasStub.uninstall();
	});

	function image(): HTMLImageElement {
		return new ImageRecorder() as unknown as HTMLImageElement;
	}

	it('makes the copy at the drawn size, with room to zoom in a little', function () {
		const copies = new cool.VectorModifiedImages();
		const source = image();
		const copy = copies.get(source, invert, 10, 20) as any;

		assert.strictEqual(copy.width, 13);
		assert.strictEqual(copy.height, 25);
		const draw = ScratchCanvasStub.recorder(0).findCall('drawImage');
		assert.deepStrictEqual(draw.args, [source, 0, 0, 13, 25]);
		assert.ok(ScratchCanvasStub.recorder(0).findCall('putImageData'));
	});

	it('never makes the copy larger than the image', function () {
		const copy = new cool.VectorModifiedImages().get(
			image(),
			invert,
			500,
			500,
		) as any;
		assert.strictEqual(copy.width, 100);
		assert.strictEqual(copy.height, 100);
	});

	it('makes the copy of an SVG image at the drawn size, past its own size', function () {
		// The SVG draws its detail at any size, up to 4096 pixels across.
		const svg = image();
		svg.src = 'data:image/svg+xml;base64,PHN2Zy8+';
		const copies = new cool.VectorModifiedImages();
		const copy = copies.get(svg, invert, 500, 500) as any;
		assert.strictEqual(copy.width, 625);
		assert.strictEqual(copy.height, 625);

		const largest = copies.get(svg, invert, 10000, 10000) as any;
		assert.strictEqual(largest.width, 4096);
		assert.strictEqual(largest.height, 4096);
	});

	it('reuses a copy for a smaller draw and makes a larger one when needed', function () {
		const copies = new cool.VectorModifiedImages();
		const source = image();
		const large = copies.get(source, invert, 40, 40);
		assert.strictEqual(copies.get(source, invert, 10, 10), large);
		assert.strictEqual(ScratchCanvasStub.created.length, 1);

		const larger = copies.get(source, invert, 60, 60) as any;
		assert.notStrictEqual(larger, large);
		assert.strictEqual(larger.width, 75);
		// The smaller copy gives its pixels back.
		assert.strictEqual((large as any).width, 0);
	});

	it('keeps a copy for each list of modifiers', function () {
		const copies = new cool.VectorModifiedImages();
		const source = image();
		const inverted = copies.get(source, invert, 10, 10);
		const grayed = copies.get(source, [{ modifier: 'gray' }], 10, 10);
		assert.notStrictEqual(inverted, grayed);
		assert.strictEqual(copies.get(source, invert, 10, 10), inverted);
	});

	it('drops the copy drawn longest ago to stay within the budget', function () {
		// Room for two copies of 10 by 10 pixels.
		const copies = new cool.VectorModifiedImages(2 * 10 * 10 * 4);
		const first = image();
		const second = image();
		const third = image();
		const firstCopy = copies.get(first, invert, 8, 8);
		const secondCopy = copies.get(second, invert, 8, 8);
		// Drawing the first again makes the second the oldest.
		copies.get(first, invert, 8, 8);
		copies.get(third, invert, 8, 8);

		assert.strictEqual((secondCopy as any).width, 0);
		assert.strictEqual(copies.get(first, invert, 8, 8), firstCopy);
		assert.notStrictEqual(copies.get(second, invert, 8, 8), secondCopy);
	});

	it('gives every copy back on release', function () {
		const copies = new cool.VectorModifiedImages();
		const source = image();
		const copy = copies.get(source, invert, 10, 10) as any;
		copies.release();
		assert.strictEqual(copy.width, 0);
		assert.notStrictEqual(copies.get(source, invert, 10, 10), copy);
	});

	it('draws the image itself when no canvas is left for a copy', function () {
		ScratchCanvasStub.uninstall();
		ScratchCanvasStub.installUnavailable();
		const source = image();
		assert.strictEqual(
			new cool.VectorModifiedImages().get(source, invert, 10, 10),
			source,
		);
	});
});
