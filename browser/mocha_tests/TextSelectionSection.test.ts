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

describe('TextSelectionSection glide', function () {
	// A rectangle of selected text on a line 300 twips high, from left to right.
	function line(
		left: number,
		right: number,
		top = 1000,
	): Array<cool.SimplePoint> {
		return [
			new cool.SimplePoint(left, top),
			new cool.SimplePoint(right, top),
			new cool.SimplePoint(right, top + 300),
			new cool.SimplePoint(left, top + 300),
		];
	}

	function nearby(
		before: Array<Array<cool.SimplePoint>>,
		after: Array<Array<cool.SimplePoint>>,
	): boolean {
		return (TextSelectionSection as any).nearby(before, after);
	}

	it('glides when an end of the selection moves a character along its line', function () {
		nodeassert.strictEqual(nearby([line(100, 400)], [line(100, 550)]), true);
		nodeassert.strictEqual(nearby([line(100, 400)], [line(250, 400)]), true);
	});

	it('is drawn at once when a selection of the same shape appears elsewhere', function () {
		nodeassert.strictEqual(nearby([line(100, 400)], [line(2100, 2400)]), false);
		nodeassert.strictEqual(
			nearby([line(100, 400)], [line(100, 400, 3000)]),
			false,
		);
	});

	it('is drawn at once when the selection changes its shape', function () {
		nodeassert.strictEqual(
			nearby([line(100, 400)], [line(100, 400), line(100, 400, 1300)]),
			false,
		);
		nodeassert.strictEqual(nearby([], [line(100, 400)]), false);
		nodeassert.strictEqual(nearby([[]], [[]]), false);
	});
});
