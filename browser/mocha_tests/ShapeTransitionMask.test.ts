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

describe('ShapeTransitionMask', function () {
	// A shape twice as wide as it is tall, at 100, 50 in twips.
	const wideBox = { x: 100, y: 50, width: 400, height: 200 } as DOMRect;

	function mask(
		type: TransitionType,
		subtype: TransitionSubType,
		isDirectionForward = true,
		isModeIn = true,
	): ShapeTransitionMask {
		return new ShapeTransitionMask(
			new TransitionFilterInfo(type, subtype, isDirectionForward, isModeIn),
		);
	}

	it('uncovers a wipe from the left up to its progress', function () {
		const clip = mask(
			TransitionType.BARWIPE,
			TransitionSubType.LEFTTORIGHT,
		).clipAt(0.25, wideBox);

		nodeassert.ok(clip);
		nodeassert.deepStrictEqual(clip.transform, [400, 0, 0, 200, 100, 50]);
		nodeassert.deepStrictEqual(clip.frame, [0, 0, 1, 1]);
		nodeassert.strictEqual(clip.inverted, false);
		nodeassert.strictEqual(clip.pieces.length, 1);
		const [left, , width] = (clip.pieces[0] as any).rectangle;
		nodeassert.strictEqual(left + width, 0.25);
	});

	it('uncovers a reversed wipe from the right', function () {
		const clip = mask(
			TransitionType.BARWIPE,
			TransitionSubType.LEFTTORIGHT,
			false,
		).clipAt(0.25, wideBox);

		nodeassert.strictEqual((clip.pieces[0] as any).rectangle[0], 0.75);
	});

	it('takes an exit wipe away from the left', function () {
		const clip = mask(
			TransitionType.BARWIPE,
			TransitionSubType.LEFTTORIGHT,
			true,
			false,
		).clipAt(0.25, wideBox);

		// The same band as the entrance, drawn as everything but the band.
		nodeassert.strictEqual(clip.inverted, true);
		const [left, , width] = (clip.pieces[0] as any).rectangle;
		nodeassert.strictEqual(left + width, 0.25);
	});

	it('uncovers a reversed box from the edges in', function () {
		const box = mask(
			TransitionType.IRISWIPE,
			TransitionSubType.RECTANGLE,
			false,
		);

		// At the start the box left out covers the whole shape, so nothing
		// shows. At the end nothing is left out.
		const start = box.clipAt(0, wideBox);
		nodeassert.strictEqual(start.inverted, true);
		const [left, top, width, height] = (start.pieces[0] as any).rectangle;
		nodeassert.ok(left <= 0 && top <= 0);
		nodeassert.ok(left + width >= 1 && top + height >= 1);

		const end = box.clipAt(1, wideBox);
		nodeassert.strictEqual(end.inverted, true);
		nodeassert.strictEqual(end.pieces.length, 0);
	});

	it('keeps a circle round on a wide shape', function () {
		const clip = mask(
			TransitionType.ELLIPSEWIPE,
			TransitionSubType.CIRCLE,
		).clipAt(0.2, wideBox);

		// The width spans the mask, and the height its half about the middle.
		nodeassert.deepStrictEqual(clip.frame, [0, 0.25, 1, 0.5]);
		nodeassert.deepStrictEqual(clip.transform, [400, 0, 0, 400, 100, -50]);
		const [centerX, centerY, radiusX, radiusY] = (clip.pieces[0] as any)
			.ellipse;
		nodeassert.deepStrictEqual([centerX, centerY], [0.5, 0.5]);
		nodeassert.strictEqual(radiusX, radiusY);
	});

	it('shows no square of a dissolve at the start and all at the end', function () {
		const dissolve = mask(TransitionType.DISSOLVE, TransitionSubType.DEFAULT);

		nodeassert.strictEqual(dissolve.clipAt(0, wideBox).pieces.length, 0);

		// At the end every row is one piece across the mask.
		const pieces = dissolve.clipAt(1, wideBox).pieces as any[];
		nodeassert.strictEqual(pieces.length, 16);
		for (const piece of pieces) nodeassert.strictEqual(piece.rectangle[2], 1);
	});

	it('opens the blades of a wheel together', function () {
		const clip = mask(
			TransitionType.PINWHEELWIPE,
			TransitionSubType.FOURBLADE,
		).clipAt(0.5, wideBox);

		nodeassert.strictEqual(clip.pieces.length, 4);
		for (const piece of clip.pieces as any[]) {
			const [, , , start, end] = piece.slice;
			nodeassert.ok(Math.abs(end - start - Math.PI / 4) < 1e-9);
		}
	});

	it('shows nothing of a cut until the end', function () {
		const cut = mask(TransitionType.BARWIPE, TransitionSubType.FADEOVERCOLOR);

		nodeassert.strictEqual(cut.clipAt(0.9, wideBox).inverted, false);
		nodeassert.strictEqual(cut.clipAt(0.9, wideBox).pieces.length, 0);
		nodeassert.strictEqual(cut.clipAt(1, wideBox).inverted, true);
	});

	it('fades a shape for a filter without an area of its own', function () {
		const slide = mask(TransitionType.SLIDEWIPE, TransitionSubType.FROMLEFT);
		nodeassert.strictEqual(slide.hasArea(), false);
		nodeassert.strictEqual(slide.clipAt(0.5, wideBox), null);
		nodeassert.strictEqual(slide.fadeAt(0.25), 0.25);

		const exit = mask(
			TransitionType.SLIDEWIPE,
			TransitionSubType.FROMLEFT,
			true,
			false,
		);
		nodeassert.strictEqual(exit.fadeAt(0.25), 0.75);
	});

	it('draws nothing over a box without area', function () {
		const clip = mask(
			TransitionType.BARWIPE,
			TransitionSubType.LEFTTORIGHT,
		).clipAt(0.5, { x: 0, y: 0, width: 0, height: 10 } as DOMRect);

		nodeassert.strictEqual(clip, null);
	});
});
