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

describe('Scene3D', function () {
	const assert = require('assert');
	let originalPath2D: any;
	let originalContext: any;

	before(function () {
		originalPath2D = (globalThis as any).Path2D;
		(globalThis as any).Path2D = Path2DRecorder;
		originalContext = (cool.VectorHitTest as any)._context;
		(cool.VectorHitTest as any)._context = new HitTestContext();
	});

	after(function () {
		(globalThis as any).Path2D = originalPath2D;
		(cool.VectorHitTest as any)._context = originalContext;
	});

	/*
		The records the engine sends for a scene of 9 cm, seen in perspective, holding a sphere of
		5 cm in its middle and a scene moved 4 cm across that holds a cube of 2 cm, as in
		sd/qa/unit/data/vector-3d-scene.fodp. The boxes are the engine's own, from the projection
		of each object.
	*/
	function sceneData(): any {
		const objects: any[] = [
			{
				id: 1,
				parent: 0,
				objectKind: 202,
				x: 1134,
				y: 1417,
				width: 5103,
				height: 5104,
				primitives: [],
				scene3D: {
					placement: [
						5102.36220472441, 0, 0, 5102.36220472441, 1133.85826771654,
						1417.32283464567,
					],
					transformation: [1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1],
					orientation: [1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, -16885, 0, 0, 0, 1],
					projection: [
						3.836, 0, 0.333333333333333, 0, 0, 5.754, 0, 0, 0, 0, -6.754,
						-111541.29, 0, 0, -1, 0,
					],
					deviceToView: [
						0.5, 0, 0, 0.5, 0, -0.5, 0, 0.5, 0, 0, 0.5, 0.5, 0, 0, 0, 1,
					],
					perspective: true,
					shadeMode: 2,
					ambientLight: '#666666',
					lights: [],
				},
			},
			{
				id: 2,
				parent: 1,
				objectKind: 205,
				x: 1134,
				y: 1417,
				width: 3402,
				height: 5104,
				transform3D: [1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1],
				primitives3D: [
					{
						type: 'sphere',
						horizontalSegments: 24,
						verticalSegments: 24,
						transform: [
							5000, 0, 0, -2500, 0, 5000, 0, -2500, 0, 0, 5000, -2500, 0, 0, 0,
							1,
						],
					},
				],
			},
			{
				id: 3,
				parent: 1,
				objectKind: 202,
				x: 4476,
				y: 3044,
				width: 1440,
				height: 1850,
				transform3D: [1, 0, 0, 4000, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1],
			},
			{
				id: 4,
				parent: 3,
				objectKind: 204,
				x: 4476,
				y: 3044,
				width: 1440,
				height: 1850,
				transform3D: [1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1],
				primitives3D: [
					{
						type: 'cube',
						transform: [
							2000, 0, 0, -1000, 0, 2000, 0, -1000, 0, 0, 2000, -1000, 0, 0, 0,
							1,
						],
					},
				],
			},
		];
		const map = new Map<number, any>();
		objects.forEach((object: any) => map.set(object.id, object));
		return {
			partId: 'scene',
			version: 1,
			order: objects.map((object: any) => object.id),
			objects: map,
		};
	}

	// the straight pieces a recorded path draws, from its moves, lines and closes
	function segmentsOf(path: any): number[][] {
		const segments: number[][] = [];
		let start: number[] = [];
		let at: number[] = [];
		for (const call of path.calls) {
			if (call.method === 'moveTo') start = at = call.args;
			else if (call.method === 'lineTo') {
				segments.push(at.concat(call.args));
				at = call.args;
			} else if (call.method === 'closePath') {
				segments.push(at.concat(start));
				at = start;
			}
		}
		return segments;
	}

	it('a transform followed by another maps a point through both, in that order', function () {
		const move = cool.Matrix3D.fromArray([
			1, 0, 0, 10, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1,
		])!;
		const double = cool.Matrix3D.fromArray([
			2, 0, 0, 0, 0, 2, 0, 0, 0, 0, 2, 0, 0, 0, 0, 1,
		])!;
		const point = move.then(double).apply(new cool.Point3D(1, 1, 1));
		assert.deepStrictEqual([point.x, point.y, point.z], [22, 2, 2]);
	});

	it('a transform with a perspective divides by the fourth value', function () {
		const half = cool.Matrix3D.fromArray([
			1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 2,
		])!;
		const point = half.apply(new cool.Point3D(4, 6, 8));
		assert.deepStrictEqual([point.x, point.y, point.z], [2, 3, 4]);
	});

	it('the inverse of a transform takes a point back where it was', function () {
		const camera = cool.Matrix3D.fromArray(
			sceneData().objects.get(1).scene3D.projection,
		)!;
		const there = camera.apply(new cool.Point3D(300, -200, -18000));
		const back = camera.invert()!.apply(there);
		assert.ok(Math.abs(back.x - 300) < 1e-6, String(back.x));
		assert.ok(Math.abs(back.y + 200) < 1e-6, String(back.y));
		assert.ok(Math.abs(back.z + 18000) < 1e-6, String(back.z));
	});

	it('the corners of the cube land on the box the engine gives the cube', function () {
		// the cube is laid out through the scene in between, the camera and the placement
		const data = sceneData();
		const view = data.objects.get(1).scene3D;
		const toView = cool.Matrix3D.fromArray(
			data.objects.get(4).primitives3D[0].transform,
		)!
			.then(cool.Matrix3D.fromArray(data.objects.get(4).transform3D)!)
			.then(cool.Matrix3D.fromArray(data.objects.get(3).transform3D)!)
			.then(cool.Matrix3D.fromArray(view.transformation)!)
			.then(cool.Matrix3D.fromArray(view.orientation)!)
			.then(cool.Matrix3D.fromArray(view.projection)!)
			.then(cool.Matrix3D.fromArray(view.deviceToView)!);
		const placement = cool.Matrix2D.fromArray(view.placement)!;
		const corners: cool.Point[] = [];
		for (const x of [0, 1])
			for (const y of [0, 1])
				for (const z of [0, 1]) {
					const seen = toView.apply(new cool.Point3D(x, y, z));
					corners.push(placement.apply(seen.x, seen.y));
				}
		const box = cool.Range2D.fromPoints(corners);
		assert.ok(Math.abs(box.minX - 4476) < 2, String(box.minX));
		assert.ok(Math.abs(box.minY - 3044) < 2, String(box.minY));
		assert.ok(Math.abs(box.width - 1440) < 2, String(box.width));
		assert.ok(Math.abs(box.height - 1850) < 2, String(box.height));
	});

	it('the scene is worked out from its 3D objects where each is of a kind made here', function () {
		const data = sceneData();
		assert.strictEqual(cool.Scene3D.takes(data, 1), true);
		data.objects.get(4).primitives3D[0].type = 'tube';
		assert.strictEqual(
			cool.Scene3D.takes(data, 1),
			false,
			'a kind not made here',
		);
	});

	it('a point hits the 3D object it lies on, and nothing in an empty corner', function () {
		const data = sceneData();
		const cube = cool.Scene3D.hitAt(data, 1, 4476 + 720, 3044 + 925);
		assert.strictEqual(cube?.id, 4, 'the middle of the cube');
		const sphere = cool.Scene3D.hitAt(data, 1, 1134 + 2551, 1417 + 2552);
		assert.strictEqual(
			sphere?.id,
			2,
			'the middle of the scene, where the sphere is',
		);
		assert.strictEqual(
			cool.Scene3D.hitAt(data, 1, 1134 + 5000, 1417 + 100),
			undefined,
			'the upper right corner of the scene, where nothing is',
		);
		assert.strictEqual(
			cool.Scene3D.hitAt(data, 1, 1134 - 100, 1417 + 2552),
			undefined,
			'beside the scene',
		);
	});

	it('a point over two 3D objects hits both, the one in front first', function () {
		// the cube is moved in front of the sphere, half over it
		const data = sceneData();
		data.objects.get(3).transform3D = [
			1, 0, 0, 1500, 0, 1, 0, 0, 0, 0, 1, 3000, 0, 0, 0, 1,
		];
		const hits = cool.Scene3D.hitsAt(data, 1, 1134 + 2800, 1417 + 2552);
		assert.deepStrictEqual(
			hits.map((hit: { id: number }) => hit.id),
			[4, 2],
		);
		assert.ok(hits[0].depth < hits[1].depth);
		assert.strictEqual(
			cool.Scene3D.hitAt(data, 1, 1134 + 2800, 1417 + 2552)?.id,
			4,
		);
	});

	it('the outline of one 3D object holds its edges only', function () {
		const segments = segmentsOf(cool.Scene3D.outlineOf(sceneData(), 4));
		assert.strictEqual(segments.length, 12, 'the twelve edges of the cube');
		for (const segment of segments)
			assert.ok(segment[0] >= 4470 && segment[2] >= 4470, String(segment));
	});

	// all faces of a primitive, each as its polygons
	function facesOf(primitive: any): cool.Point3D[][][] {
		const faces: cool.Point3D[][][] = [];
		cool.Scene3D.forEachFace(primitive, (polygons: cool.Point3D[][]) => {
			faces.push(polygons);
			return true;
		});
		return faces;
	}

	function square(from: number, to: number): any {
		return { closed: true, points: [from, from, to, from, to, to, from, to] };
	}

	it('an extruded square closed at both ends has two lids and four sides', function () {
		const faces = facesOf({
			type: 'extrude',
			outline: [square(0, 1000)],
			depth: 500,
			closeFront: true,
			closeBack: true,
		});
		assert.strictEqual(faces.length, 6);
		const lids = faces.filter((face) =>
			face[0].every((point) => point.z === face[0][0].z),
		);
		assert.deepStrictEqual(
			lids.map((face) => face[0][0].z).sort((a, b) => a - b),
			[0, 500],
		);
	});

	it('an extrusion with a hole keeps the hole in its lids and walls it in', function () {
		const hole = {
			closed: true,
			points: [300, 300, 300, 700, 700, 700, 700, 300],
		};
		const faces = facesOf({
			type: 'extrude',
			outline: [square(0, 1000), hole],
			depth: 500,
			closeFront: true,
			closeBack: true,
		});
		const lids = faces.filter((face) => face.length === 2);
		assert.strictEqual(lids.length, 2, 'each lid with its hole');
		assert.strictEqual(
			faces.length - lids.length,
			8,
			'four walls outside, four inside',
		);
	});

	it('a bevel takes the front lid in from the outline', function () {
		const faces = facesOf({
			type: 'extrude',
			outline: [square(0, 1000)],
			depth: 500,
			diagonal: 0.2,
			closeFront: true,
		});
		const front = faces.find((face) =>
			face[0].every((point) => point.z === 500),
		);
		assert.ok(front, 'a front lid');
		for (const point of front![0]) {
			assert.ok(point.x > 0 && point.x < 1000, String(point.x));
			assert.ok(point.y > 0 && point.y < 1000, String(point.y));
		}
	});

	it('a body turned all round has no lids and closes on itself', function () {
		const faces = facesOf({
			type: 'lathe',
			outline: [{ closed: false, points: [500, 0, 500, 1000] }],
			rotation: 2 * Math.PI,
			horizontalSegments: 8,
			closeFront: true,
			closeBack: true,
		});
		assert.strictEqual(
			faces.length,
			8,
			'one face for each step, none for a lid',
		);
	});

	it('a body turned half way round has its sides and a lid at each end', function () {
		const faces = facesOf({
			type: 'lathe',
			outline: [{ closed: true, points: [200, 0, 600, 0, 400, 500] }],
			rotation: Math.PI,
			horizontalSegments: 6,
			closeFront: true,
			closeBack: true,
		});
		assert.strictEqual(faces.length, 6 * 3 + 2);
	});

	it('an extruded square placed as the cube is hit where the cube is', function () {
		const data = sceneData();
		data.objects.get(4).objectKind = 206;
		data.objects.get(4).primitives3D = [
			{
				type: 'extrude',
				outline: [square(0, 2000)],
				depth: 2000,
				closeFront: true,
				closeBack: true,
				transform: [1, 0, 0, -1000, 0, 1, 0, -1000, 0, 0, 1, -1000, 0, 0, 0, 1],
			},
		];
		assert.strictEqual(cool.Scene3D.takes(data, 1), true);
		assert.strictEqual(
			cool.Scene3D.hitAt(data, 1, 4476 + 720, 3044 + 925)?.id,
			4,
		);
		assert.strictEqual(
			cool.Scene3D.hitAt(data, 1, 4476 + 1500, 3044 + 925),
			undefined,
		);
	});

	it('a sphere is outlined by eight meridians and five rings', function () {
		// 24 by 24 segments: each meridian runs over 24 pieces down, each ring over 24 round
		const data = sceneData();
		const segments = segmentsOf(cool.Scene3D.outlineOf(data, 2));
		assert.strictEqual(segments.length, 8 * 24 + 5 * 24);
	});

	it('a body turned all round is outlined by its profiles and the rings of its ends', function () {
		const data = sceneData();
		// a changed record comes with a new version of the part
		data.version = 2;
		data.objects.get(2).primitives3D = [
			{
				type: 'lathe',
				outline: [{ closed: false, points: [500, 0, 500, 1000] }],
				rotation: 2 * Math.PI,
				horizontalSegments: 8,
			},
		];
		// eight profiles of one edge each, and the two rings of eight
		const segments = segmentsOf(cool.Scene3D.outlineOf(data, 2));
		assert.strictEqual(segments.length, 8 + 2 * 8);
	});

	it('a body of many steps turned all round is outlined by eight of its profiles', function () {
		const data = sceneData();
		data.version = 3;
		data.objects.get(2).primitives3D = [
			{
				type: 'lathe',
				outline: [{ closed: false, points: [500, 0, 500, 1000] }],
				rotation: 2 * Math.PI,
				horizontalSegments: 24,
			},
		];
		// eight profiles of one edge each, and the two rings of twenty four
		const segments = segmentsOf(cool.Scene3D.outlineOf(data, 2));
		assert.strictEqual(segments.length, 8 + 2 * 24);
	});

	it('a body turned half way round has a profile for each eighth of a turn', function () {
		const data = sceneData();
		data.version = 4;
		data.objects.get(2).primitives3D = [
			{
				type: 'lathe',
				outline: [{ closed: false, points: [500, 0, 500, 1000] }],
				rotation: Math.PI,
				horizontalSegments: 12,
			},
		];
		// five profiles from start to end, and the two open rings of twelve steps
		const segments = segmentsOf(cool.Scene3D.outlineOf(data, 2));
		assert.strictEqual(segments.length, 5 + 2 * 12);
	});

	it('the outline follows the shapes, each edge drawn once', function () {
		const outline: any = cool.Scene3D.outlineOf(sceneData(), 1);
		const segments = segmentsOf(outline);
		assert.ok(segments.length > 0, 'some edges');

		// no edge twice
		const keys = new Set<string>();
		for (const segment of segments) {
			const one = segment.slice(0, 2).map((value: number) => value.toFixed(3));
			const other = segment.slice(2).map((value: number) => value.toFixed(3));
			const key = [one.join(','), other.join(',')].sort().join(' ');
			assert.ok(!keys.has(key), 'an edge drawn twice: ' + key);
			keys.add(key);
		}

		// the edges run on in chains, fewer moves than edges
		const moves = outline.calls.filter((call: any) => call.method === 'moveTo');
		assert.ok(
			moves.length < segments.length,
			moves.length + ' ' + segments.length,
		);

		// a sphere has no edges of its own, a cube shows all its twelve edges
		const insideTheCube = segments.filter(
			(segment: number[]) =>
				segment[0] >= 4470 &&
				segment[0] <= 5922 &&
				segment[1] >= 3038 &&
				segment[1] <= 4900 &&
				segment[2] >= 4470 &&
				segment[2] <= 5922,
		);
		assert.strictEqual(insideTheCube.length, 12);
	});
});
