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

describe('VectorSlideLayers', function () {
	// A part of the given objects, each an id and the group it sits in, in
	// paint order.
	function part(objects: [number, number?][]): any {
		return {
			order: objects.map(([id]) => id),
			objects: new Map(objects.map(([id, parent]) => [id, { id, parent }])),
		};
	}

	it('keeps the objects around an animated one in runs of their own', function () {
		const data = part([[0], [11], [22], [33], [44]]);

		nodeassert.deepStrictEqual(
			VectorSlideLayers.paintRuns(data, new Set([22])),
			[
				{ start: 0, end: 2, owner: undefined },
				{ start: 2, end: 3, owner: 22 },
				{ start: 3, end: 5, owner: undefined },
			],
		);
	});

	it('puts the members of an animated group in the run of the group', function () {
		const data = part([[0], [50], [51, 50], [52, 50], [60]]);

		nodeassert.deepStrictEqual(
			VectorSlideLayers.paintRuns(data, new Set([50])),
			[
				{ start: 0, end: 1, owner: undefined },
				{ start: 1, end: 4, owner: 50 },
				{ start: 4, end: 5, owner: undefined },
			],
		);
	});

	it('gives an animated member of an animated group a run of its own', function () {
		const data = part([[0], [50], [51, 50], [52, 50]]);

		nodeassert.deepStrictEqual(
			VectorSlideLayers.paintRuns(data, new Set([50, 52])),
			[
				{ start: 0, end: 1, owner: undefined },
				{ start: 1, end: 3, owner: 50 },
				{ start: 3, end: 4, owner: 52 },
			],
		);
	});

	it('draws a slide without animations as one run', function () {
		const data = part([[0], [11], [22]]);

		nodeassert.deepStrictEqual(VectorSlideLayers.paintRuns(data, new Set()), [
			{ start: 0, end: 3, owner: undefined },
		]);
	});

	it('draws a master video over the master objects below it and under those above it', function () {
		const data = part([[0], [11], [22]]);
		const master = part([[0], [5], [6], [7]]);

		nodeassert.deepStrictEqual(
			VectorSlideLayers.splitAtMasterVideos(
				VectorSlideLayers.paintRuns(data, new Set()),
				master,
				new Set([6]),
			),
			[
				{ start: 0, end: 1, owner: undefined, master: { start: 0, end: 3 } },
				{ start: 0, end: 3, owner: undefined, master: { start: 3, end: 4 } },
			],
		);
	});

	it('keeps the runs after the page when the master holds a video', function () {
		const data = part([[0], [11], [22]]);
		const master = part([[0], [5], [6]]);

		nodeassert.deepStrictEqual(
			VectorSlideLayers.splitAtMasterVideos(
				VectorSlideLayers.paintRuns(data, new Set([22])),
				master,
				new Set([5]),
			),
			[
				{ start: 0, end: 1, owner: undefined, master: { start: 0, end: 2 } },
				{ start: 0, end: 2, owner: undefined, master: { start: 2, end: 3 } },
				{ start: 2, end: 3, owner: 22 },
			],
		);
	});

	it('draws the page whole when its master holds no video', function () {
		const data = part([[0], [11]]);
		const master = part([[0], [5]]);
		const runs = VectorSlideLayers.paintRuns(data, new Set());

		nodeassert.deepStrictEqual(
			VectorSlideLayers.splitAtMasterVideos(runs, master, new Set([99])),
			runs,
		);
	});
});
