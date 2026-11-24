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

describe('SvgPathReader', function () {
	const assert = require('assert');

	function placesOf(data: string): string[] {
		return cool.SvgPathReader.read(data).map((polygon: any) =>
			polygon.points
				.map((point: any) => point.x.toFixed(6) + ',' + point.y.toFixed(6))
				.join(' '),
		);
	}

	// the engine writes a value next to nothing as a number with an exponent, as SVG allows;
	// a curve that goes on without its letter again may start with one
	it('a number with an exponent is read as one number', function () {
		const plain = 'm0 0c0 10 10 10 10 0 0 5-5 5-5 0s10-10 20 0z';
		assert.deepStrictEqual(
			placesOf('m0 0c0 10 10 10 10 0 1.8E-012 5-5 5-5 0s10-10 20 0z'),
			placesOf(plain),
		);
		assert.deepStrictEqual(
			placesOf('m0 0c0 10 10 10 10 0 -2.5e-13 5-5 5-5 0s10-10 20 0z'),
			placesOf(plain),
		);
	});
});
