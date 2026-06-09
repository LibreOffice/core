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

describe('NormalPoint', function () {

	describe('NormalPoint.flexConstruct()', function () {
		it('flexConstruct(null) should return null', function () {
			nodeassert.equal(NormalPoint.flexConstruct(null), null);
		});

		it('flexConstruct(undefined) should return null', function () {
			nodeassert.equal(NormalPoint.flexConstruct(undefined), null);
		});

		it('flexConstruct(normPoint) should return the argument', function () {
			const normPoint = new NormalPoint(-10, 20);
			nodeassert.deepEqual(NormalPoint.flexConstruct(normPoint), normPoint);
		});

		it('flexConstruct([]) should return null', function () {
			nodeassert.equal(NormalPoint.flexConstruct([]), null);
		});

		it('flexConstruct([200]) should return null', function () {
			nodeassert.equal(NormalPoint.flexConstruct([200,]), null);
		});

		it('flexConstruct([number, number]) should return NormalPoint', function () {
			const a = -10;
			const b = 20;
			const normPoint = new NormalPoint(a, b);
			nodeassert.deepEqual(NormalPoint.flexConstruct([a, b]), normPoint);
		});

		it('flexConstruct([\'100\', \'200\']) should return null', function () {
			nodeassert.equal(NormalPoint.flexConstruct(['100', '200']), null);
		});

		it('flexConstruct([number, number, number]) should return null', function () {
			nodeassert.equal(NormalPoint.flexConstruct([200, 300, 400]), null);
		});

		it('flexConstruct(NormPointLike) should return NormalPoint', function () {
			const a = -10;
			const b = 20;
			const normPoint = new NormalPoint(a, b);
			const obj = { lat: a, lng: b };
			nodeassert.deepEqual(NormalPoint.flexConstruct(obj), normPoint);
		});

		it('flexConstruct(unknown object) should return null', function () {
			const a = -10;
			const b = 20;
			const obj = { A: a, red: b };
			nodeassert.equal(NormalPoint.flexConstruct(obj), null);
		});

		it('flexConstruct(number, number) should return NormalPoint', function () {
			const a = -10;
			const b = 20;
			const normPoint = new NormalPoint(a, b);
			nodeassert.deepEqual(NormalPoint.flexConstruct(a, b), normPoint);
		});
	});

});

describe('NormalBounds', function () {

	const NB = NormalBoundsBase;

	describe('NormalBounds.flexConstruct()', function () {

		it('flexConstruct(null) should return undefined', function () {
			nodeassert.equal(NB.flexConstruct(null), undefined);
		});

		it('flexConstruct(undefined) should return undefined', function () {
			nodeassert.equal(NB.flexConstruct(undefined), undefined);
		});

		it('flexConstruct(NormalBounds) should return NormalBounds', function () {
			const a = -10;
			const b = 20;
			const c = 30;
			const d = 55;
			const normPoint1 = new NormalPoint(a, b);
			const normPoint2 = new NormalPoint(c, d);
			const bounds = new NB(normPoint1, normPoint2);
			nodeassert.deepEqual(NB.flexConstruct(bounds), bounds);
		});

		it('flexConstruct(NormalPoint, NormalPoint) should return NormalBounds', function () {
			const a = -10;
			const b = 20;
			const c = 30;
			const d = 55;
			const normPoint1 = new NormalPoint(a, b);
			const normPoint2 = new NormalPoint(c, d);
			const bounds = new NB(normPoint1, normPoint2);
			nodeassert.deepEqual(NB.flexConstruct(normPoint1, normPoint2), bounds);
		});

		it('flexConstruct([NormalPoint, NormalPoint]) should return NormalBounds', function () {
			const a = -10;
			const b = 20;
			const c = 30;
			const d = 55;
			const normPoint1 = new NormalPoint(a, b);
			const normPoint2 = new NormalPoint(c, d);
			const bounds = new NB(normPoint1, normPoint2);
			nodeassert.deepEqual(NB.flexConstruct([normPoint1, normPoint2]), bounds);
		});

		it('flexConstruct([number, number], [number, number]) should return NormalBounds', function () {
			const a = -10;
			const b = 20;
			const c = 30;
			const d = 55;
			const normPoint1 = new NormalPoint(a, b);
			const normPoint2 = new NormalPoint(c, d);
			const bounds = new NB(normPoint1, normPoint2);
			nodeassert.deepEqual(NB.flexConstruct([a, b], [c, d]), bounds);
		});

		it('flexConstruct([[number, number], [number, number]]) should return NormalBounds', function () {
			const a = -10;
			const b = 20;
			const c = 30;
			const d = 55;
			const normPoint1 = new NormalPoint(a, b);
			const normPoint2 = new NormalPoint(c, d);
			const bounds = new NB(normPoint1, normPoint2);
			nodeassert.deepEqual(NB.flexConstruct([[a, b], [c, d]]), bounds);
		});

		it('flexConstruct([number, number]) should return NormalBounds', function () {
			const a = -10;
			const b = 20;
			const normPoint1 = new NormalPoint(a, b);
			const bounds = new NB(normPoint1, normPoint1);
			nodeassert.deepEqual(NB.flexConstruct([a, b]), bounds);
		});
	});
});
