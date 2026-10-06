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

describe('VectorColorFilters', function () {
	const assert = require('assert');

	let filters: cool.VectorColorFilters;

	beforeEach(function () {
		filters = new cool.VectorColorFilters();
	});

	afterEach(function () {
		filters.release();
	});

	// The filter element that a filter value points at.
	function filterElement(value: string | null): Element {
		assert.ok(value, 'a filter value');
		const match = /^url\(#(.+)\)$/.exec(value!);
		assert.ok(match, 'the value points at an element: ' + value);
		const element = document.getElementById(match![1]);
		assert.ok(element, 'the filter is in the document');
		return element!;
	}

	it('works on the stored sRGB values', function () {
		const filter = filterElement(filters.filterFor([{ modifier: 'gray' }]));
		assert.strictEqual(
			filter.getAttribute('color-interpolation-filters'),
			'sRGB',
		);
	});

	it('grays through a color matrix that keeps the alpha', function () {
		const filter = filterElement(filters.filterFor([{ modifier: 'gray' }]));
		const matrix = filter.querySelector('feColorMatrix');
		assert.ok(matrix);
		const values = matrix!.getAttribute('values')!.split(' ').map(Number);
		const row = [77 / 256, 151 / 256, 28 / 256, 0, 0];
		assert.deepStrictEqual(values, [...row, ...row, ...row, 0, 0, 0, 1, 0]);
	});

	it('adds the alpha column of a color matrix to its constant', function () {
		// The engine takes the alpha as one, not the pixel's alpha.
		const matrix = [
			1, 0, 0, 0.25, 0.5, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0,
		];
		const filter = filterElement(
			filters.filterFor([{ modifier: 'matrix', matrix: matrix }]),
		);
		const values = filter
			.querySelector('feColorMatrix')!
			.getAttribute('values')!
			.split(' ')
			.map(Number);
		assert.deepStrictEqual(values.slice(0, 5), [1, 0, 0, 0, 0.75]);
	});

	it('raises each channel to one over the gamma', function () {
		const filter = filterElement(
			filters.filterFor([{ modifier: 'gamma', value: 2 }]),
		);
		const channels = filter.querySelectorAll('feComponentTransfer > *');
		assert.deepStrictEqual(
			Array.from(channels).map((channel) => channel.tagName),
			['feFuncR', 'feFuncG', 'feFuncB'],
		);
		for (const channel of Array.from(channels)) {
			assert.strictEqual(channel.getAttribute('type'), 'gamma');
			assert.strictEqual(channel.getAttribute('exponent'), '0.5');
		}
	});

	it('chains the modifiers first to last', function () {
		const filter = filterElement(
			filters.filterFor([
				{ modifier: 'replace', color: '#ff0000' },
				{ modifier: 'invert' },
			]),
		);
		assert.deepStrictEqual(
			Array.from(filter.children).map((child) => child.tagName),
			['feColorMatrix', 'feComponentTransfer'],
		);
	});

	it('builds one filter for the same list of modifiers', function () {
		const first = filters.filterFor([{ modifier: 'invert' }]);
		const second = filters.filterFor([{ modifier: 'invert' }]);
		assert.strictEqual(first, second);
		assert.strictEqual(document.querySelectorAll('filter').length, 1);
	});

	it('builds one filter for modifiers that change the colors the same way', function () {
		// Other fields of a modified color primitive leave the filter as is.
		const first = filters.filterFor([{ modifier: 'invert' }]);
		const second = filters.filterFor([
			{
				modifier: 'invert',
				bounds: [0, 0, 10, 10],
				children: [{ type: 'group' }],
			} as cool.ColorModifierAttribute,
		]);
		assert.strictEqual(first, second);
		assert.strictEqual(document.querySelectorAll('filter').length, 1);
	});

	it('gives no filter for modifiers that change no color', function () {
		assert.strictEqual(
			filters.filterFor([{ modifier: 'gamma', value: 1 }]),
			null,
		);
		assert.strictEqual(document.querySelectorAll('filter').length, 0);
	});

	it('takes its filters out of the document on release', function () {
		filters.filterFor([{ modifier: 'invert' }]);
		filters.release();
		assert.strictEqual(document.querySelectorAll('filter').length, 0);
	});
});
