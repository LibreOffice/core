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
});
