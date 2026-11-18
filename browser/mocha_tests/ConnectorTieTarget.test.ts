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

describe('ConnectorTieTarget', function () {
	// A handle is 10 pixels wide and a pixel is 15 twips, so an end ties within 150 twips.
	const reach = 150;

	let objects: Map<number, any>;
	let order: number[];
	let savedCurrentPart: any;
	let savedHandleSize: any;
	let savedPixelsToTwips: number;

	// An object with that box in twips, standing upright on the slide.
	function objectAt(
		left: number,
		top: number,
		width: number,
		height: number,
		more: any = {},
	): any {
		return {
			transform: [width, 0, 0, height, left, top],
			x: left,
			y: top,
			width: width,
			height: height,
			parent: 0,
			primitives: [],
			...more,
		};
	}

	/*
		Object 2 is a polygon whose box runs from 1000 to 3000 across and from 1000 to 2000
		down. It paints nothing here, so any point counts as off its outline, as the middles of
		the sides of a polygon can be.
	*/
	beforeEach(function () {
		objects = new Map<number, any>();
		objects.set(1, { kind: 'page', parent: -1 });
		objects.set(
			2,
			objectAt(1000, 1000, 2000, 1000, {
				sideGluePoints: [
					{ x: 0.5, y: 0 },
					{ x: 1, y: 0.5 },
					{ x: 0.5, y: 1 },
					{ x: 0, y: 0.5 },
				],
			}),
		);
		order = [1, 2];

		savedCurrentPart = RenderGeometrySection.currentPart;
		(RenderGeometrySection as any).currentPart = () => ({
			objects: objects,
			order: order,
		});
		savedHandleSize = ShapeHandlesSection.handleSize;
		(ShapeHandlesSection as any).handleSize = () => 10;
		savedPixelsToTwips = app.pixelsToTwips;
		app.pixelsToTwips = 15;
	});

	afterEach(function () {
		(RenderGeometrySection as any).currentPart = savedCurrentPart;
		(ShapeHandlesSection as any).handleSize = savedHandleSize;
		app.pixelsToTwips = savedPixelsToTwips;
	});

	it('an end near the middle of a side ties there, off the outline of the shape', function () {
		const at = new cool.Point(2000, 1000 - reach / 2);
		nodeassert.strictEqual(SelectionSection.tieTargetAt(at), 2);

		const tie = SelectionSection.tieOn(2, at);
		nodeassert.deepStrictEqual(tie?.tie, { autoVertex: true, point: 0 });
		nodeassert.ok(tie?.point?.equals(new cool.Point(2000, 1000)));
	});

	it('an end farther from the box than the reach of a tie aims at nothing', function () {
		const at = new cool.Point(2000, 1000 - 2 * reach);
		nodeassert.strictEqual(SelectionSection.tieTargetAt(at), null);
	});

	it('the frontmost box takes the end, unless it is the one left out', function () {
		objects.set(3, objectAt(2400, 1200, 200, 200));
		order.push(3);

		const at = new cool.Point(2500, 1300);
		nodeassert.strictEqual(SelectionSection.tieTargetAt(at), 3);
		nodeassert.strictEqual(SelectionSection.tieTargetAt(at, 3), 2);
	});

	it('a connector in front takes no end', function () {
		objects.set(3, objectAt(2400, 1200, 200, 200, { objectKind: 24 }));
		order.push(3);

		const at = new cool.Point(2500, 1300);
		nodeassert.strictEqual(SelectionSection.tieTargetAt(at), 2);
	});

	it('the members of a group take the end, and the box of the group does not', function () {
		objects.set(5, objectAt(0, 0, 5000, 5000));
		objects.get(2).parent = 5;
		order.splice(1, 0, 5);

		nodeassert.strictEqual(
			SelectionSection.tieTargetAt(new cool.Point(2000, 1500)),
			2,
		);
		nodeassert.strictEqual(
			SelectionSection.tieTargetAt(new cool.Point(4000, 4000)),
			null,
		);
	});
});
