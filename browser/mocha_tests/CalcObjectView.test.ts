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

describe('calcPlacedObject', function () {
	const content: cool.CalcDrawingContent = {
		layer: 'back',
		parent: 0,
		anchorColumn: 2,
		anchorRow: 3,
		x: 100,
		y: 200,
		width: 50,
		height: 60,
		transform: [50, 0, 0, 60, 100, 200],
		handles: [{ kind: 7, x: 110, y: 210 }],
		path: [
			{
				points: [
					{ x: 100, y: 200, aheadX: 120, aheadY: 200 },
					{ x: 150, y: 260 },
				],
			},
		],
		gluePoints: [{ x: 0.5, y: 0.5 }],
		primitives: [],
	};

	const moved: cool.CalcObjectPlacing = {
		scaleX: 1,
		scaleY: 1,
		moveX: 3,
		moveY: -4,
	};

	it('moves every position in twips by the move of the placing', function () {
		const object = cool.calcPlacedObject(content, moved);
		nodeassert.strictEqual(object.x, 103);
		nodeassert.strictEqual(object.y, 196);
		nodeassert.deepStrictEqual(object.transform, [50, 0, 0, 60, 103, 196]);
		nodeassert.deepStrictEqual(object.handles, [{ kind: 7, x: 113, y: 206 }]);
		const points = (object.path ?? [])[0].points;
		nodeassert.deepStrictEqual(points[0], {
			x: 103,
			y: 196,
			aheadX: 123,
			aheadY: 196,
		});
		nodeassert.deepStrictEqual(points[1], { x: 153, y: 256 });
	});

	it('stretches the box and everything in it by the scale', function () {
		const object = cool.calcPlacedObject(content, {
			scaleX: 2,
			scaleY: 0.5,
			moveX: 10,
			moveY: 20,
		});
		nodeassert.strictEqual(object.x, 210);
		nodeassert.strictEqual(object.y, 120);
		nodeassert.strictEqual(object.width, 100);
		nodeassert.strictEqual(object.height, 30);
		nodeassert.deepStrictEqual(object.transform, [100, 0, 0, 30, 210, 120]);
		nodeassert.deepStrictEqual(object.handles, [{ kind: 7, x: 230, y: 125 }]);
		const points = (object.path ?? [])[0].points;
		nodeassert.deepStrictEqual(points[1], { x: 310, y: 150 });
		const place = (object.primitives ?? [])[0] as cool.TransformPrimitive;
		nodeassert.deepStrictEqual(place.matrix, [2, 0, 0, 0.5, 10, 20]);
	});

	it('leaves the places given on the object where they are', function () {
		const object = cool.calcPlacedObject(content, moved);
		nodeassert.deepStrictEqual(object.gluePoints, [{ x: 0.5, y: 0.5 }]);
	});

	it('draws the primitives moved by the placing', function () {
		const object = cool.calcPlacedObject(content, moved);
		const primitives = object.primitives ?? [];
		nodeassert.strictEqual(primitives.length, 1);
		const move = primitives[0] as cool.TransformPrimitive;
		nodeassert.strictEqual(move.type, 'transform');
		nodeassert.deepStrictEqual(move.matrix, [1, 0, 0, 1, 3, -4]);
	});

	it('leaves the entry of the engine as it was', function () {
		cool.calcPlacedObject(content, moved);
		nodeassert.strictEqual(content.x, 100);
		nodeassert.deepStrictEqual(content.transform, [50, 0, 0, 60, 100, 200]);
	});

	it('names the drawing layer by the number the engine gives it', function () {
		nodeassert.strictEqual(
			cool.calcPlacedObject(content, cool.calcIdentityPlacing).layer,
			1,
		);
	});
});

describe('calcMessageInPrintTwips', function () {
	// The placings move print twips to tile twips. Objects 2 and 4 are in
	// group 3. Object 5 is stretched to twice its width.
	const placing = (moveX: number, moveY: number): cool.CalcObjectPlacing => ({
		scaleX: 1,
		scaleY: 1,
		moveX,
		moveY,
	});
	const placement: cool.CalcPlacement = {
		placingOf: (id: number) =>
			id === 1
				? placing(10, 20)
				: id === 2 || id === 4
					? placing(5, 5)
					: id === 5
						? { scaleX: 2, scaleY: 1, moveX: 10, moveY: 20 }
						: undefined,
		outermostOf: (id: number) => (id === 2 || id === 4 ? 3 : id),
		membersOf: (outermost: number) => (outermost === 3 ? [2, 4] : [outermost]),
		// A corner moved keeps the scale of its object.
		placingAt: (id: number) =>
			id === 5 ? { scaleX: 2, scaleY: 1, moveX: 10, moveY: 20 } : placing(7, 9),
	};

	it('takes a handle back to print twips', function () {
		nodeassert.strictEqual(
			cool.calcMessageInPrintTwips(
				'setobjecthandle id=1 kind=7 polygon=0 point=0 at=-1 x=110 y=220 ortho=1',
				placement,
			),
			'setobjecthandle id=1 kind=7 polygon=0 point=0 at=-1 x=100 y=200 ortho=1',
		);
	});

	it('takes a handle of a stretched object back through its scale', function () {
		nodeassert.strictEqual(
			cool.calcMessageInPrintTwips(
				'setobjecthandle id=5 kind=7 polygon=0 point=0 at=-1 x=210 y=220 ortho=1',
				placement,
			),
			'setobjecthandle id=5 kind=7 polygon=0 point=0 at=-1 x=100 y=200 ortho=1',
		);
	});

	it('takes the mapping of a stretched object back through its scale', function () {
		nodeassert.strictEqual(
			cool.calcMessageInPrintTwips(
				'setobjecttransform to=5,200,0,0,50,2010,2020 as=scale',
				placement,
			),
			'setobjecttransform to=5,100.0000,0.0000,0.0000,50.0000,1000.0000,2000.0000 as=scale',
		);
	});

	it('takes the corners of a crop back to print twips', function () {
		nodeassert.strictEqual(
			cool.calcMessageInPrintTwips(
				'cropobject id=1 x1=10 y1=20 x2=110 y2=220',
				placement,
			),
			'cropobject id=1 x1=0 y1=0 x2=100 y2=200',
		);
	});

	it('takes the points of a path back to print twips', function () {
		nodeassert.strictEqual(
			cool.calcMessageInPrintTwips(
				'setobjectpoints id=1 changes=0,1,0,110,220,0;0,2,1,10,20,2',
				placement,
			),
			'setobjectpoints id=1 changes=0,1,0,100,200,0;0,2,1,0,0,2',
		);
	});

	it('anchors a moved object where the drag left it', function () {
		nodeassert.strictEqual(
			cool.calcMessageInPrintTwips(
				'setobjecttransform to=1,100,0,0,50,1007,2009 as=move',
				placement,
			),
			'setobjecttransform to=1,100.0000,0.0000,0.0000,50.0000,1000.0000,2000.0000 as=move',
		);
	});

	it('keeps an object of a group moved on its own with the group', function () {
		nodeassert.strictEqual(
			cool.calcMessageInPrintTwips(
				'setobjecttransform to=2,100,0,0,50,1005,2005 as=scale',
				placement,
			),
			'setobjecttransform to=2,100.0000,0.0000,0.0000,50.0000,1000.0000,2000.0000 as=scale',
		);
	});

	it('anchors a group moved whole where the drag left it', function () {
		nodeassert.strictEqual(
			cool.calcMessageInPrintTwips(
				'setobjecttransform to=2,100,0,0,50,1007,2009;4,100,0,0,50,1207,2009 as=move',
				placement,
			),
			'setobjecttransform to=2,100.0000,0.0000,0.0000,50.0000,1000.0000,2000.0000;' +
				'4,100.0000,0.0000,0.0000,50.0000,1200.0000,2000.0000 as=move',
		);
	});

	it('anchors a flipped object at the corner its mapping covers', function () {
		// A mapping with a negative width puts its box left of its origin.
		const corners: cool.Point[] = [];
		const watching: cool.CalcPlacement = {
			...placement,
			placingAt: (id: number, point: cool.Point) => {
				corners.push(point);
				return placing(7, 9);
			},
		};
		cool.calcMessageInPrintTwips(
			'setobjecttransform to=1,-100,0,0,50,1107,2009 as=mirror',
			watching,
		);
		nodeassert.deepStrictEqual(corners, [new cool.Point(1007, 2009)]);
	});

	it('leaves the glue points, which are given on the object, as they are', function () {
		const message = 'setobjectgluepoint id=1 at=0 x=0.5 y=0.5';
		nodeassert.strictEqual(
			cool.calcMessageInPrintTwips(message, placement),
			message,
		);
	});

	it('leaves an object it does not know as it is', function () {
		const message = 'setobjecthandle id=9 kind=7 x=110 y=220';
		nodeassert.strictEqual(
			cool.calcMessageInPrintTwips(message, placement),
			message,
		);
	});
});

describe('calcTwipsMap', function () {
	// The sheet is 1000 print twips long and 800 tile twips long.
	const dimension = {
		getSize: (unit: string) => (unit === 'printtwips' ? 1000 : 800),
		getTileTwipsPosFromPrint: (position: number) => position * 0.8,
	} as unknown as cool.SheetDimension;

	it('maps a position inside the sheet by the geometry', function () {
		nodeassert.strictEqual(cool.calcTwipsMap(dimension)(500), 400);
	});

	it('keeps the distance of a position past the end of the sheet', function () {
		nodeassert.strictEqual(cool.calcTwipsMap(dimension)(1200), 1000);
	});

	it('keeps a position before the start of the sheet', function () {
		nodeassert.strictEqual(cool.calcTwipsMap(dimension)(-50), -50);
	});
});
