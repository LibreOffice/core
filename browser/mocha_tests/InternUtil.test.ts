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

describe('InternPoint', function () {

	describe('InternPointUtil.flexConstruct()', function () {
		it('flexConstruct(null) should return undefined', function () {
			nodeassert.equal(InternPointUtil.flexConstruct(null), undefined);
		});

		it('flexConstruct(undefined) should return undefined', function () {
			nodeassert.equal(InternPointUtil.flexConstruct(undefined), undefined);
		});

		it('flexConstruct(internPoint) should return the argument', function () {
			const internPoint = InternPointUtil.flexConstruct(-10, 20);
			nodeassert.deepEqual(InternPointUtil.flexConstruct(internPoint), internPoint);
		});

		it('flexConstruct([]) should return undefined', function () {
			nodeassert.equal(InternPointUtil.flexConstruct([]), undefined);
		});

		it('flexConstruct([200]) should return undefined', function () {
			nodeassert.equal(InternPointUtil.flexConstruct([200,]), undefined);
		});

		it('flexConstruct([number, number]) should return InternPointUtil', function () {
			const a = -10;
			const b = 20;
			const internPoint = InternPointUtil.flexConstruct(a, b);
			nodeassert.deepEqual(InternPointUtil.flexConstruct([a, b]), internPoint);
		});

		it('flexConstruct([\'100\', \'200\']) should return undefined', function () {
			nodeassert.equal(InternPointUtil.flexConstruct(['100', '200']), undefined);
		});

		it('flexConstruct([number, number, number]) should return undefined', function () {
			nodeassert.equal(InternPointUtil.flexConstruct([200, 300, 400]), undefined);
		});

		it('flexConstruct(NormPointLike) should return InternPoint', function () {
			const a = -10;
			const b = 20;
			const internPoint = InternPointUtil.flexConstruct(a, b);
			const obj = { x: a, y: b };
			nodeassert.deepEqual(InternPointUtil.flexConstruct(obj), internPoint);
		});

		it('flexConstruct(unknown object) should return undefined', function () {
			const a = -10;
			const b = 20;
			const obj = { A: a, red: b };
			nodeassert.equal(InternPointUtil.flexConstruct(obj), undefined);
		});

		it('flexConstruct(number, number) should return InternPoint', function () {
			const a = -10;
			const b = 20;
			const internPoint = InternPointUtil.flexConstruct(a, b);
			nodeassert.deepEqual(InternPointUtil.flexConstruct(a, b), internPoint);
		});
	});

});

describe('NormalBounds', function () {

	const IBU = InternBoundsUtil;

	describe('InternBoundsUtil.flexConstruct()', function () {

		it('flexConstruct(null) should return undefined', function () {
			nodeassert.equal(IBU.flexConstruct(null), undefined);
		});

		it('flexConstruct(undefined) should return undefined', function () {
			nodeassert.equal(IBU.flexConstruct(undefined), undefined);
		});

		it('flexConstruct(internBounds) should return InternBounds', function () {
			const a = -10;
			const b = 20;
			const c = 30;
			const d = 55;
			const internPoint1 = InternPointUtil.flexConstruct(a, b);
			const internPoint2 = InternPointUtil.flexConstruct(c, d);
			const bounds = IBU.flexConstruct(internPoint1, internPoint2);
			nodeassert.deepEqual(IBU.flexConstruct(bounds), bounds);
		});

		it('flexConstruct(InternPoint, InternPoint) should return InternBounds', function () {
			const a = -10;
			const b = 20;
			const c = 30;
			const d = 55;
			const internPoint1 = InternPointUtil.flexConstruct(a, b);
			const internPoint2 = InternPointUtil.flexConstruct(c, d);
			const bounds = IBU.flexConstruct(internPoint1, internPoint2);
			const rect = new cool.SimpleRectangle(a, b, c - a, d - b);
			nodeassert.deepEqual(bounds, rect);
		});

		it('flexConstruct([InternPoint, InternPoint]) should return InternBounds', function () {
			const a = -10;
			const b = 20;
			const c = 30;
			const d = 55;
			const internPoint1 = InternPointUtil.flexConstruct(a, b);
			const internPoint2 = InternPointUtil.flexConstruct(c, d);
			const bounds = IBU.flexConstruct(internPoint1, internPoint2);
			nodeassert.deepEqual(IBU.flexConstruct([internPoint1, internPoint2]), bounds);
		});

		it('flexConstruct([number, number], [number, number]) should return InternBounds', function () {
			const a = -10;
			const b = 20;
			const c = 30;
			const d = 55;
			const internPoint1 = InternPointUtil.flexConstruct(a, b);
			const internPoint2 = InternPointUtil.flexConstruct(c, d);
			const bounds = IBU.flexConstruct(internPoint1, internPoint2);
			nodeassert.deepEqual(IBU.flexConstruct([a, b], [c, d]), bounds);
		});

		it('flexConstruct([[number, number], [number, number]]) should return InternBounds', function () {
			const a = -10;
			const b = 20;
			const c = 30;
			const d = 55;
			const internPoint1 = InternPointUtil.flexConstruct(a, b);
			const internPoint2 = InternPointUtil.flexConstruct(c, d);
			const bounds = IBU.flexConstruct(internPoint1, internPoint2);
			nodeassert.deepEqual(IBU.flexConstruct([[a, b], [c, d]]), bounds);
		});

		it('flexConstruct([number, number]) should return InternBounds', function () {
			const a = -10;
			const b = 20;
			const internPoint1 = InternPointUtil.flexConstruct(a, b);
			const bounds = IBU.flexConstruct(internPoint1, internPoint1);
			nodeassert.deepEqual(IBU.flexConstruct([a, b]), bounds);
		});
	});
});
