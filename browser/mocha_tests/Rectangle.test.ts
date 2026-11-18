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

describe('Rectangle', function () {
	interface ExpectedRectangle {
		x1?: number;
		x2?: number;
		width?: number;

		y1?: number;
		y2?: number;
		height?: number;

		area?: number;

		// rounded versions

		rx1?: number;
		rx2?: number;
		rwidth?: number;

		ry1?: number;
		ry2?: number;
		rheight?: number;

		rarea?: number;

		eps: number;
	}

	function assertRectangle(
		actual: cool.Rectangle,
		expected: ExpectedRectangle,
	) {
		// x-coordinate

		if (expected.x1 !== undefined)
			assertFloat(actual.getX1(), expected.x1, expected.eps, 'Wrong x1');
		if (expected.x2 !== undefined)
			assertFloat(actual.getX2(), expected.x2, expected.eps, 'Wrong x2');
		if (expected.width !== undefined)
			assertFloat(
				actual.getWidth(),
				expected.width,
				expected.eps,
				'Wrong width',
			);

		if (expected.rx1 !== undefined)
			assertFloat(
				actual.getPxX1(),
				expected.rx1,
				expected.eps,
				'Wrong rounded x1',
			);
		if (expected.rx2 !== undefined)
			assertFloat(
				actual.getPxX2(),
				expected.rx2,
				expected.eps,
				'Wrong rounded x2',
			);
		if (expected.rwidth !== undefined)
			assertFloat(
				actual.getPxWidth(),
				expected.rwidth,
				expected.eps,
				'Wrong rounded width',
			);

		// y-coordinate

		if (expected.y1 !== undefined)
			assertFloat(actual.getY1(), expected.y1, expected.eps, 'Wrong y1');
		if (expected.y2 !== undefined)
			assertFloat(actual.getY2(), expected.y2, expected.eps, 'Wrong y2');
		if (expected.height !== undefined)
			assertFloat(
				actual.getHeight(),
				expected.height,
				expected.eps,
				'Wrong height',
			);

		if (expected.ry1 !== undefined)
			assertFloat(
				actual.getPxY1(),
				expected.ry1,
				expected.eps,
				'Wrong rounded y1',
			);
		if (expected.ry2 !== undefined)
			assertFloat(
				actual.getPxY2(),
				expected.ry2,
				expected.eps,
				'Wrong rounded y2',
			);
		if (expected.rheight !== undefined)
			assertFloat(
				actual.getPxHeight(),
				expected.rheight,
				expected.eps,
				'Wrong rounded height',
			);

		// area

		if (expected.area !== undefined)
			assertFloat(actual.getArea(), expected.area, expected.eps, 'Wrong area');
		if (expected.rarea !== undefined)
			assertFloat(
				actual.getPxArea(),
				expected.rarea,
				expected.eps,
				'Wrong rounded area',
			);
	}

	const x1 = 3.4;
	const y1 = 6.3;
	const width = 2.3;
	const height = 8.2;

	const eps = 0.01;

	describe('Validity of Rectangle', function () {
		it('Invalid rectangle', function () {
			let invalidRect = new cool.Rectangle(3, 4, -1, 10);
			nodeassert.ok(!invalidRect.isValid(), 'Rectangle should be invalid');
			invalidRect = new cool.Rectangle(3, 4, 1, -10);
			nodeassert.ok(!invalidRect.isValid(), 'Rectangle should be invalid');
		});

		it('Valid rectangle', function () {
			const validRect = new cool.Rectangle(3, 4, 1, 10);
			nodeassert.ok(validRect.isValid(), 'Rectangle should be valid');
		});
	});

	describe('construction test', function () {
		it('construct', function () {
			const rect = cool.createRectangle(x1, y1, width, height);
			assertRectangle(rect, {
				x1: x1,
				x2: x1 + width,
				width: width,

				y1: y1,
				y2: y1 + height,
				height: height,

				rx1: Math.round(x1),
				rx2: Math.round(x1 + width),
				rwidth: Math.round(x1 + width) - Math.round(x1),

				ry1: Math.round(y1),
				ry2: Math.round(y1 + height),
				rheight: Math.round(y1 + height) - Math.round(y1),

				area: width * height,
				rarea:
					(Math.round(x1 + width) - Math.round(x1)) *
					(Math.round(y1 + height) - Math.round(y1)),
				eps: eps,
			});
		});
	});

	describe('coordinate API tests', function () {
		const coords = ['x', 'y'];
		coords.forEach(function (coord: string) {
			const isX = coord === 'x';
			const isY = !isX;

			describe(coord + '-coordinate API tests', function () {
				it(isX ? 'setX1' : 'setY1', function () {
					const rect = cool.createRectangle(x1, y1, width, height);
					const newC1 = 1.3;
					const newSize = (isX ? width + x1 : height + y1) - newC1;

					isX ? rect.setX1(newC1) : rect.setY1(newC1);

					const newX1 = isX ? newC1 : x1;
					const newY1 = isY ? newC1 : y1;
					const newWidth = isX ? newSize : width;
					const newHeight = isY ? newSize : height;

					assertRectangle(rect, {
						x1: newX1,
						x2: x1 + width,
						width: newWidth,

						y1: newY1,
						y2: y1 + height,
						height: newHeight,

						rx1: Math.round(newX1),
						rx2: Math.round(x1 + width),
						rwidth: Math.round(x1 + width) - Math.round(newX1),

						ry1: Math.round(newY1),
						ry2: Math.round(y1 + height),
						rheight: Math.round(y1 + height) - Math.round(newY1),

						area: newWidth * newHeight,
						rarea:
							(Math.round(x1 + width) - Math.round(newX1)) *
							(Math.round(y1 + height) - Math.round(newY1)),
						eps: eps,
					});
				});

				it(isX ? 'setX2' : 'setY2', function () {
					const rect = cool.createRectangle(x1, y1, width, height);
					const newC2 = 10.3;
					const newSize = newC2 - (isX ? x1 : y1);

					isX ? rect.setX2(newC2) : rect.setY2(newC2);

					const newX2 = isX ? newC2 : x1 + width;
					const newY2 = isY ? newC2 : y1 + height;
					const newWidth = isX ? newSize : width;
					const newHeight = isY ? newSize : height;

					assertRectangle(rect, {
						x1: x1,
						x2: newX2,
						width: newWidth,

						y1: y1,
						y2: newY2,
						height: newHeight,

						rx1: Math.round(x1),
						rx2: Math.round(newX2),
						rwidth: Math.round(newX2) - Math.round(x1),

						ry1: Math.round(y1),
						ry2: Math.round(newY2),
						rheight: Math.round(newY2) - Math.round(y1),

						area: newWidth * newHeight,
						rarea:
							(Math.round(newX2) - Math.round(x1)) *
							(Math.round(newY2) - Math.round(y1)),
						eps: eps,
					});
				});

				it(isX ? 'setWidth' : 'setHeight', function () {
					const rect = cool.createRectangle(x1, y1, width, height);
					const newSize = 1.4;
					const newC2 = (isX ? x1 : y1) + newSize;

					isX ? rect.setWidth(newSize) : rect.setHeight(newSize);

					const newX2 = isX ? newC2 : x1 + width;
					const newY2 = isY ? newC2 : y1 + height;
					const newWidth = isX ? newSize : width;
					const newHeight = isY ? newSize : height;

					assertRectangle(rect, {
						x1: x1,
						x2: newX2,
						width: newWidth,

						y1: y1,
						y2: newY2,
						height: newHeight,

						rx1: Math.round(x1),
						rx2: Math.round(newX2),
						rwidth: Math.round(newX2) - Math.round(x1),

						ry1: Math.round(y1),
						ry2: Math.round(newY2),
						rheight: Math.round(newY2) - Math.round(y1),

						area: newWidth * newHeight,
						rarea:
							(Math.round(newX2) - Math.round(x1)) *
							(Math.round(newY2) - Math.round(y1)),
						eps: eps,
					});
				});

				it(
					'setArea (Preserve ' + (isX ? 'height' : 'width') + ')',
					function () {
						const rect = cool.createRectangle(x1, y1, width, height);
						const newArea = 20.4;
						const newWidth = isX ? newArea / height : width;
						const newHeight = isY ? newArea / width : height;
						const newX2 = isX ? x1 + newWidth : x1 + width;
						const newY2 = isY ? y1 + newHeight : y1 + height;

						rect.setArea(newArea, isX /* preserveHeight */);

						assertRectangle(rect, {
							x1: x1,
							x2: newX2,
							width: newWidth,

							y1: y1,
							y2: newY2,
							height: newHeight,

							rx1: Math.round(x1),
							rx2: Math.round(newX2),
							rwidth: Math.round(newX2) - Math.round(x1),

							ry1: Math.round(y1),
							ry2: Math.round(newY2),
							rheight: Math.round(newY2) - Math.round(y1),

							area: newWidth * newHeight,
							rarea:
								(Math.round(newX2) - Math.round(x1)) *
								(Math.round(newY2) - Math.round(y1)),
							eps: eps,
						});
					},
				);

				it('moveBy', function () {
					const rect = cool.createRectangle(x1, y1, width, height);
					const dc = 1.3;
					const newX1 = isX ? x1 + dc : x1;
					const newX2 = isX ? x1 + width + dc : x1 + width;
					const newY1 = isY ? y1 + dc : y1;
					const newY2 = isY ? y1 + height + dc : y1 + height;
					isX ? rect.moveBy(dc, 0) : rect.moveBy(0, dc);

					assertRectangle(rect, {
						x1: newX1,
						x2: newX2,
						width: width,

						y1: newY1,
						y2: newY2,
						height: height,

						rx1: Math.round(newX1),
						rx2: Math.round(newX2),
						rwidth: Math.round(newX2) - Math.round(newX1),

						ry1: Math.round(newY1),
						ry2: Math.round(newY2),
						rheight: Math.round(newY2) - Math.round(newY1),

						area: width * height,
						rarea:
							(Math.round(newX2) - Math.round(newX1)) *
							(Math.round(newY2) - Math.round(newY1)),
						eps: eps,
					});
				});

				it('moveTo', function () {
					const rect = cool.createRectangle(x1, y1, width, height);
					const newC1 = 50.3;

					const newX1 = isX ? newC1 : x1;
					const newX2 = newX1 + width;

					const newY1 = isY ? newC1 : y1;
					const newY2 = newY1 + height;

					isX ? rect.moveTo(newC1, y1) : rect.moveTo(x1, newC1);

					assertRectangle(rect, {
						x1: newX1,
						x2: newX2,
						width: width,

						y1: newY1,
						y2: newY2,
						height: height,

						rx1: Math.round(newX1),
						rx2: Math.round(newX2),
						rwidth: Math.round(newX2) - Math.round(newX1),

						ry1: Math.round(newY1),
						ry2: Math.round(newY2),
						rheight: Math.round(newY2) - Math.round(newY1),

						area: width * height,
						rarea:
							(Math.round(newX2) - Math.round(newX1)) *
							(Math.round(newY2) - Math.round(newY1)),
						eps: eps,
					});
				});
			}); // describe(coord + '-coordinate API tests'
		}); // coords.forEach
	}); // describe('coordinate API tests', function () {
}); // root describe.
