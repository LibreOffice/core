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
		data.objects.get(4).primitives3D[0].type = 'lathe';
		assert.strictEqual(
			cool.Scene3D.takes(data, 1),
			false,
			'a lathe is not made yet',
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
		const cube: any = cool.Scene3D.outlineOf(sceneData(), 4);
		const lines = cube.calls.filter((call: any) => call.method === 'lineTo');
		assert.ok(lines.length >= 6 && lines.length <= 9, String(lines.length));
		for (const line of lines)
			assert.ok(line.args[0] >= 4470, String(line.args[0]));
	});

	it('the outline follows the shapes, each edge drawn once', function () {
		const outline: any = cool.Scene3D.outlineOf(sceneData(), 1);
		const lines = outline.calls.filter((call: any) => call.method === 'lineTo');
		const starts = outline.calls.filter(
			(call: any) => call.method === 'moveTo',
		);
		assert.ok(lines.length > 0, 'some edges');
		assert.strictEqual(lines.length, starts.length);

		// no edge twice
		const keys = new Set<string>();
		for (let at = 0; at < lines.length; ++at) {
			const one = starts[at].args.map((value: number) => value.toFixed(3));
			const other = lines[at].args.map((value: number) => value.toFixed(3));
			const key = [one.join(','), other.join(',')].sort().join(' ');
			assert.ok(!keys.has(key), 'an edge drawn twice: ' + key);
			keys.add(key);
		}

		// a sphere has no edges of its own, a cube seen from the front and a little aside shows
		// at most its nine edges towards us
		const insideTheCube = lines.filter(
			(call: any) =>
				call.args[0] >= 4470 &&
				call.args[0] <= 5922 &&
				call.args[1] >= 3038 &&
				call.args[1] <= 4900,
		);
		assert.ok(
			insideTheCube.length >= 6 && insideTheCube.length <= 9,
			String(insideTheCube.length),
		);
	});
});
