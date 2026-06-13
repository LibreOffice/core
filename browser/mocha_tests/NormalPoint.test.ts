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
			const obj = { x: a, y: b };
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

	const NB = NormalBounds;

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

	describe('NormalBounds.contains()', function () {

		const xmin = 10;
		const ymin = 20;
		const xmax = 30;
		const ymax = 50;
		const bounds = new NormalBounds([xmin, ymin], [xmax, ymax]);
		it('contains(inside min [xmin + 1, ymin + 1]) must return true', function () {
			nodeassert.equal(bounds.contains([xmin + 1, ymin + 1]), true);
		});

		it('contains(inside min edge [xmin, ymin]) must return true', function () {
			nodeassert.equal(bounds.contains([xmin, ymin]), true);
		});

		it('contains(inside max [xmax - 1, ymax - 1]) must return true', function () {
			nodeassert.equal(bounds.contains([xmax-1, ymax-1]), true);
		});

		it('contains(inside max edge [xmax, ymax]) must return true', function () {
			nodeassert.equal(bounds.contains([xmax, ymax]), true);
		});

		it('contains(outside min [xmin - 1, ymin - 1]) must return false', function () {
			nodeassert.equal(bounds.contains([xmin - 1, ymin - 1]), false);
		});

		it('contains(outise max [xmax + 1, ymax + 1]) must return false', function () {
			nodeassert.equal(bounds.contains([xmax + 1, ymax + 1]), false);
		});

		it('contains(center) must return true', function () {
			nodeassert.equal(bounds.contains(bounds.getCenter()), true);
		});

		it('contains(original bounds itself) must return true', function () {
			nodeassert.equal(bounds.contains(bounds), true);
		});

		it('contains(bounds min inside) must return true', function () {
			nodeassert.equal(bounds.contains([[xmin + 1, ymin + 1], [xmax, ymax]]), true);
		});

		it('contains(bounds max inside) must return true', function () {
			nodeassert.equal(bounds.contains([[xmin, ymin], [xmax - 1, ymax - 1]]), true);
		});

		it('contains(bounds min outside) must return false', function () {
			nodeassert.equal(bounds.contains([[xmin - 1, ymin - 1], [xmax, ymax]]), false);
		});

		it('contains(bounds max outside) must return false', function () {
			nodeassert.equal(bounds.contains([[xmin, ymin], [xmax + 1, ymax + 1]]), false);
		});

		it('contains(bounds without intersection) must return false', function () {
			const width = bounds.getWidth();
			const height = bounds.getHeight();
			nodeassert.equal(bounds.contains([[xmin - width, ymin - height], [xmin - 1, ymin - 1]]), false);
		});

	});

	describe('NormalBounds.intersects()', function () {

		const xmin = 10;
		const ymin = 20;
		const xmax = 30;
		const ymax = 50;
		const bounds = new NormalBounds([xmin, ymin], [xmax, ymax]);
		const width = bounds.getWidth();
		const height = bounds.getHeight();

		it('intersects(bounds without intersection #1) must return false', function () {
			nodeassert.equal(bounds.intersects([[xmin - width, ymin - height], [xmin - 1, ymin - 1]]), false);
		});

		it('intersects(bounds without intersection #2) must return false', function () {
			nodeassert.equal(bounds.intersects([[xmax + 1, ymax + 1], [xmax + width, ymax + height]]), false);
		});

		it('intersects(itself) must return true', function () {
			nodeassert.equal(bounds.intersects(bounds), true);
		});

		it('intersects(bounds completely inside) must return true', function () {
			nodeassert.equal(bounds.intersects([[xmin + 1, ymin + 1], [xmax - 1, ymax - 1]]), true);
		});

		it('intersects(bounds that intersects left) must return true', function () {
			nodeassert.equal(bounds.intersects([[xmin - width, ymin], [xmin, ymax]]), true);
		});

		it('intersects(bounds that intersects right) must return true', function () {
			nodeassert.equal(bounds.intersects([[xmax, ymin], [xmax + width, ymax]]), true);
		});

		it('intersects(bounds that intersects top) must return true', function () {
			nodeassert.equal(bounds.intersects([[xmin, ymin - height], [xmax, ymin]]), true);
		});

		it('intersects(bounds that intersects bottom) must return true', function () {
			nodeassert.equal(bounds.intersects([[xmin, ymax], [xmax, ymax + height]]), true);
		});

		it('intersects(bounds that intersects top-left) must return true', function () {
			nodeassert.equal(bounds.intersects([[xmin - width, ymin - height], [xmin, ymin]]), true);
		});

		it('intersects(bounds that intersects top-right) must return true', function () {
			nodeassert.equal(bounds.intersects([[xmax, ymin - height], [xmax + width, ymin]]), true);
		});

		it('intersects(bounds that intersects bottom-left) must return true', function () {
			nodeassert.equal(bounds.intersects([[xmin - width, ymax], [xmin, ymax + height]]), true);
		});

		it('intersects(bounds that intersects bottom-right) must return true', function () {
			nodeassert.equal(bounds.intersects([[xmax, ymax], [xmax + width, ymax + height]]), true);
		});
	});
});
