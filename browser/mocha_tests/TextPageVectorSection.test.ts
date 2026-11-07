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

describe('TextPageVectorSection kept pixels', function () {
	// A page of 3000 by 6000 twips, which is 300 by 600 pixels at the scale of the tests.
	const rectangle = [0, 0, 3000, 6000];
	const scale = 0.1;
	// The margin the section keeps around a page, in pixels.
	const margin = 16;

	let section: any;
	let view: CanvasRecorder;
	// The contexts the pages were drawn into from their primitives, in order.
	let drawnInto: any[];
	let savedSectionContainer: any;

	// A page with one entry that draws, whose box is given in twips of the page.
	function pageWithEntry(
		partId: string,
		x: number,
		y: number,
		width: number,
		height: number,
	): any {
		return {
			partId,
			mode: 0,
			version: 1,
			coordinates: 'entry',
			slideWidth: rectangle[2],
			slideHeight: rectangle[3],
			objects: new Map([
				[1, { x, y, width, height, primitives: [{ type: 'group' }] }],
			]),
		};
	}

	// A page whose entry lies well inside it.
	function page(partId: string): any {
		return pageWithEntry(partId, 100, 100, 1000, 1000);
	}

	function draw(
		data: any,
		origin: number[] = [0, 0],
		inView: string[] = [data.partId],
		atScale: number = scale,
	): boolean {
		return section._drawKeptPage(
			data.partId,
			data,
			origin,
			rectangle,
			atScale,
			new Set(inView),
		);
	}

	beforeEach(function () {
		ScratchCanvasStub.install();
		// Without a section container the paper has its default color.
		savedSectionContainer = app.sectionContainer;
		app.sectionContainer = undefined as any;

		drawnInto = [];
		view = new CanvasRecorder(1000, 800);
		// The section is made without its constructor, so it adds no listeners, which need the
		// whole view.
		section = Object.create(cool.TextPageVectorSection.prototype);
		section._kept = new Map();
		section._changes = 0;
		section.size = [1000, 800];
		section.context = view;
		section._manager = {
			renderTextPage: (context: any) => drawnInto.push(context),
		};
	});

	afterEach(function () {
		app.sectionContainer = savedSectionContainer;
		ScratchCanvasStub.uninstall();
	});

	it('copies the pixels of a page that did not change', function () {
		const data = page('p1');
		nodeassert.strictEqual(draw(data), true);
		nodeassert.strictEqual(draw(data), true);

		// The page was drawn from its primitives once, onto a canvas of its own.
		nodeassert.strictEqual(drawnInto.length, 1);
		nodeassert.strictEqual(ScratchCanvasStub.created.length, 1);
		nodeassert.strictEqual(drawnInto[0], ScratchCanvasStub.recorder(0));
		// Each redraw copied that canvas into the view.
		nodeassert.strictEqual(view.callsOf('drawImage').length, 2);
	});

	it('puts the page on the same part of a pixel as a direct draw', function () {
		const data = page('p1');
		draw(data, [10.5, 20.25]);

		// The canvas holds the page and a margin on each side.
		const canvas = ScratchCanvasStub.recorder(0);
		nodeassert.strictEqual(canvas.canvas.width, 333);
		nodeassert.strictEqual(canvas.canvas.height, 633);
		// The page starts in the canvas after the margin, at the fraction of a pixel it has in
		// the view.
		nodeassert.deepStrictEqual(canvas.findCall('translate')!.args, [
			margin + 0.5,
			margin + 0.25,
		]);
		nodeassert.deepStrictEqual(canvas.findCall('scale')!.args, [scale, scale]);
		// The canvas is copied to whole pixels, the margin to the left of and above the page.
		const copy = view.findCall('drawImage')!;
		nodeassert.deepStrictEqual(copy.args.slice(1), [10 - margin, 20 - margin]);
	});

	it('draws the page anew when what its pixels depend on changed', function () {
		const data = page('p1');
		draw(data);
		nodeassert.strictEqual(drawnInto.length, 1);

		// A change of the vector content was announced: a delta, a picture or a font.
		++section._changes;
		draw(data);
		nodeassert.strictEqual(drawnInto.length, 2);

		// The page has other content.
		const changed = page('p1');
		changed.version = 2;
		draw(changed);
		nodeassert.strictEqual(drawnInto.length, 3);

		// The zoom changed.
		draw(changed, [0, 0], ['p1'], 0.2);
		nodeassert.strictEqual(drawnInto.length, 4);

		// The page moved by a part of a pixel.
		draw(changed, [0.5, 0], ['p1'], 0.2);
		nodeassert.strictEqual(drawnInto.length, 5);

		// The page moved by whole pixels, which copies the same pixels elsewhere.
		draw(changed, [3.5, 7], ['p1'], 0.2);
		nodeassert.strictEqual(drawnInto.length, 5);
	});

	it('keeps no pixels of a page with an entry that reaches past its edge', function () {
		// The entry starts 200 twips left of the page, which is 20 pixels and more than half the
		// margin.
		const data = pageWithEntry('p1', -200, 100, 1000, 1000);
		nodeassert.strictEqual(draw(data), false);

		// Nothing was drawn, so that the page is drawn whole from its primitives.
		nodeassert.strictEqual(ScratchCanvasStub.created.length, 0);
		nodeassert.strictEqual(drawnInto.length, 0);
		nodeassert.strictEqual(view.callsOf('drawImage').length, 0);

		// The same holds past the right and the bottom edge.
		nodeassert.strictEqual(
			draw(pageWithEntry('p2', 2500, 100, 700, 1000)),
			false,
		);
		nodeassert.strictEqual(
			draw(pageWithEntry('p3', 100, 5500, 1000, 700)),
			false,
		);
	});

	it('keeps the pixels of a page with an entry just past its edge', function () {
		// The entry starts 50 twips left of the page, which is 5 pixels and inside the margin.
		const data = pageWithEntry('p1', -50, 100, 1000, 1000);
		nodeassert.strictEqual(draw(data), true);
		nodeassert.strictEqual(drawnInto.length, 1);
	});

	it('keeps no pixels of a page with an entry that has no box', function () {
		const data = page('p1');
		data.objects.get(1).width = undefined;
		nodeassert.strictEqual(draw(data), false);
		nodeassert.strictEqual(ScratchCanvasStub.created.length, 0);
	});

	it('keeps no pixels of a page larger than its budget', function () {
		// The budget is three times the pixels of the section.
		section.size = [100, 100];
		nodeassert.strictEqual(draw(page('p1')), false);
		nodeassert.strictEqual(ScratchCanvasStub.created.length, 0);
		nodeassert.strictEqual(drawnInto.length, 0);
	});

	it('gives up the pixels of a page out of view to make room', function () {
		// A kept page takes 332 by 632 pixels. The budget of 450000 pixels holds two of them.
		section.size = [500, 300];
		nodeassert.strictEqual(draw(page('p1')), true);
		nodeassert.strictEqual(draw(page('p2')), true);

		// The third page comes into view with the second, and the first is out of view.
		nodeassert.strictEqual(draw(page('p3'), [0, 0], ['p2', 'p3']), true);
		nodeassert.deepStrictEqual(Array.from(section._kept.keys()).sort(), [
			'p2',
			'p3',
		]);

		// With all three in view the third finds no room and keeps no pixels.
		section._kept.clear();
		draw(page('p1'), [0, 0], ['p1', 'p2', 'p3']);
		draw(page('p2'), [0, 0], ['p1', 'p2', 'p3']);
		nodeassert.strictEqual(draw(page('p3'), [0, 0], ['p1', 'p2', 'p3']), false);
	});

	it('keeps the pixels of the wanted pages only', function () {
		draw(page('p1'));
		draw(page('p2'));
		section._keepPixelsOf(new Set(['p2']));
		nodeassert.deepStrictEqual(Array.from(section._kept.keys()), ['p2']);
	});
});
