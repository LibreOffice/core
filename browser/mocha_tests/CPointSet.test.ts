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

describe('CPointSet', function () {
	describe('new CPointSet()', function () {
		it('should be empty', function () {
			nodeassert.ok(new CPointSet().empty());
		});
	});

	describe('CPointSet constructed from Points', function () {
		it('should be not be empty', function () {
			var pointArray: Array<cool.Point> = [
				new cool.Point(10, 40),
				new cool.Point(50, 100),
			];
			nodeassert.ok(!CPointSet.fromPointArray(pointArray).empty());
		});
	});

	describe('CPointSet constructed from array of CPointSets', function () {
		it('should be not be empty', function () {
			var pointArray: Array<cool.Point> = [
				new cool.Point(10, 40),
				new cool.Point(50, 100),
			];
			var pSet1 = CPointSet.fromPointArray(pointArray);
			pointArray = [new cool.Point(100, 400), new cool.Point(500, 1000)];
			var pSet2 = CPointSet.fromPointArray(pointArray);
			var pSetArray = [pSet1, pSet2];
			nodeassert.ok(!CPointSet.fromSetArray(pSetArray).empty());
		});
	});

	describe('After setting an empty Point array to a non-empty CPointSet', function () {
		it('the CPointSet should be empty', function () {
			var pointArray: Array<cool.Point> = [
				new cool.Point(10, 40),
				new cool.Point(50, 100),
			];
			var pSet = CPointSet.fromPointArray(pointArray);
			pSet.setPointArray([]);
			nodeassert.ok(pSet.empty());
		});
	});
});
