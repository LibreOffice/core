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

describe('VectorManager', function () {
	// A dropped part re-fetches in full, which sends a request. Give the
	// socket a no-op so those requests do not need a live connection.
	before(function () {
		const socket: any = (app as any).socket || ((app as any).socket = {});
		socket.sendMessage = function () {};
	});

	// Drawing builds Path2D objects, which node does not have. A stand-in
	// records the path so the drawing calls can be checked.
	let originalPath2D: any;
	before(function () {
		originalPath2D = (globalThis as any).Path2D;
		(globalThis as any).Path2D = Path2DRecorder;
	});
	after(function () {
		(globalThis as any).Path2D = originalPath2D;
	});

	function countCalls(recorder: CanvasRecorder, method: string): number {
		return recorder.calls.filter((call) => call.method === method).length;
	}

	// A primitive tree response carries a stable id per object. The
	// manager has to keep those ids on the cached objects, in document
	// order, so a later update can find an object by id.
	it('keeps each object id from a primitive tree response', function () {
		const manager = new VectorManager();

		// Two objects with empty primitive lists.
		manager.handleVectorPrimitivesResponse({
			part: 0,
			slideWidth: 1000,
			slideHeight: 800,
			objects: [
				{ id: 11, primitives: [] },
				{ id: 22, primitives: [] },
			],
		});

		const data = manager.requestPart(0);
		nodeassert.ok(data, 'part 0 is cached after its response');
		nodeassert.deepStrictEqual(
			data.objects.map((object) => object.id),
			[11, 22],
		);
	});

	// The engine stamps each part with a content version. The manager
	// keeps it on the cached tree so a later update can tell whether the
	// client is still in step with the engine.
	it('keeps the part version from a primitive tree response', function () {
		const manager = new VectorManager();

		manager.handleVectorPrimitivesResponse({
			part: 0,
			version: 7,
			slideWidth: 1000,
			slideHeight: 800,
			objects: [],
		});

		const data = manager.requestPart(0);
		nodeassert.ok(data, 'part 0 is cached after its response');
		nodeassert.strictEqual(data.version, 7);
	});

	// A delta rebuilds the part from its order: changed objects take the
	// new content, unchanged ones keep what was cached, and the order
	// list sets the result (here it also reorders the two objects).
	it('applies a delta, reusing cached content for unchanged objects', function () {
		const manager = new VectorManager();
		manager.handleVectorPrimitivesResponse({
			part: 0,
			version: 1,
			slideWidth: 1000,
			slideHeight: 800,
			objects: [
				{ id: 11, primitives: [] },
				{ id: 22, primitives: [] },
			],
		});

		// Object 22 changed and now carries one primitive. The order is
		// reversed. Object 11 is unchanged.
		const delta: any = {
			part: 0,
			version: 2,
			order: [22, 11],
			objects: [{ id: 22, primitives: [{ type: 'polygonHairline' }] }],
		};
		manager.handleVectorPrimitivesDelta(delta);

		const data: any = manager.requestPart(0);
		nodeassert.strictEqual(data.version, 2);
		nodeassert.deepStrictEqual(
			data.objects.map((object: cool.SlideObject) => object.id),
			[22, 11],
		);
		nodeassert.strictEqual(data.objects[0].primitives.length, 1);
		nodeassert.strictEqual(data.objects[1].primitives.length, 0);
	});

	// When the order names an object the client never cached, the delta
	// cannot be rebuilt, so the part is dropped and re-fetched in full.
	it('drops the part when a delta order names unknown content', function () {
		const manager = new VectorManager();
		manager.handleVectorPrimitivesResponse({
			part: 0,
			version: 1,
			slideWidth: 1000,
			slideHeight: 800,
			objects: [{ id: 11, primitives: [] }],
		});

		const delta: any = { part: 0, version: 2, order: [99], objects: [] };
		manager.handleVectorPrimitivesDelta(delta);

		// The cache was dropped, so the next request starts a fresh full
		// fetch and has nothing to return yet.
		nodeassert.strictEqual(manager.requestPart(0), undefined);
	});

	// A delta computed against an older version can arrive after a newer
	// full response. It describes an older state, so it must not roll the
	// cache backwards.
	it('ignores a delta that is not newer than the cache', function () {
		const manager = new VectorManager();
		manager.handleVectorPrimitivesResponse({
			part: 0,
			version: 5,
			slideWidth: 1000,
			slideHeight: 800,
			objects: [
				{ id: 11, primitives: [] },
				{ id: 22, primitives: [] },
			],
		});

		// A stale delta from version 3 no longer lists object 22.
		const delta: any = { part: 0, version: 3, order: [11], objects: [] };
		manager.handleVectorPrimitivesDelta(delta);

		const data: any = manager.requestPart(0);
		nodeassert.strictEqual(data.version, 5);
		nodeassert.strictEqual(data.objects.length, 2);
	});

	// A delta carries the master page only when it changed, and then the
	// cached master page content is replaced.
	it('replaces the cached master page when a delta carries one', function () {
		const manager = new VectorManager();
		manager.handleVectorPrimitivesResponse({
			part: 0,
			version: 1,
			slideWidth: 1000,
			slideHeight: 800,
			masterPage: { primitives: [] },
			objects: [{ id: 11, primitives: [] }],
		});

		const delta: any = {
			part: 0,
			version: 2,
			order: [11],
			objects: [],
			masterPage: { primitives: [{ type: 'polygonHairline' }] },
		};
		manager.handleVectorPrimitivesDelta(delta);

		const data: any = manager.requestPart(0);
		nodeassert.strictEqual(data.version, 2);
		nodeassert.strictEqual(data.masterPage.length, 1);
	});

	// Each object carries where it sits in the group tree, which layer it
	// is on, whether it is an empty placeholder and where it paints. The
	// manager keeps all of that so the view can hit-test and frame objects.
	it('keeps the geometry and grouping of each object', function () {
		const manager = new VectorManager();
		manager.handleVectorPrimitivesResponse({
			part: 0,
			slideWidth: 1000,
			slideHeight: 800,
			objects: [
				{ id: 11, parent: 0, layer: 0, primitives: [] },
				{
					id: 22,
					parent: 11,
					layer: 2,
					emptyPlaceholder: true,
					x: 10,
					y: 20,
					width: 300,
					height: 200,
					transform: [300, 0, 0, 200, 10, 20],
					primitives: [],
				},
			],
		});

		const data = manager.requestPart(0);
		nodeassert.ok(data, 'part 0 is cached after its response');
		const member = data.objects[1];
		nodeassert.strictEqual(member.parent, 11);
		nodeassert.strictEqual(member.layer, 2);
		nodeassert.strictEqual(member.emptyPlaceholder, true);
		nodeassert.deepStrictEqual(
			[member.x, member.y, member.width, member.height],
			[10, 20, 300, 200],
		);
		nodeassert.deepStrictEqual(member.transform, [300, 0, 0, 200, 10, 20]);
	});

	// Hiding a layer leaves the objects on it out of the drawing while the
	// rest of the slide still paints, and showing it again brings them back.
	it('skips the objects on a hidden layer when drawing', function () {
		const manager = new VectorManager();
		const hairline = { type: 'polygonHairline', path: 'M0 0 L1 1' };
		manager.handleVectorPrimitivesResponse({
			part: 0,
			slideWidth: 1000,
			slideHeight: 800,
			objects: [
				{ id: 11, layer: 0, primitives: [hairline] },
				{ id: 22, layer: 5, primitives: [hairline] },
			],
		});
		const data: any = manager.requestPart(0);

		manager.setLayerVisible(5, false);
		let recorder = new CanvasRecorder();
		manager.renderInto(recorder as any, data);
		nodeassert.strictEqual(countCalls(recorder, 'stroke'), 1);

		manager.setLayerVisible(5, true);
		recorder = new CanvasRecorder();
		manager.renderInto(recorder as any, data);
		nodeassert.strictEqual(countCalls(recorder, 'stroke'), 2);
	});

	// Hiding a layer changes what the slide shows, so the views draw again.
	it('redraws the views when a layer is hidden', function () {
		const manager = new VectorManager();
		let notified = 0;
		manager.onVectorChanged(() => notified++);

		manager.setLayerVisible(3, false);
		nodeassert.strictEqual(notified, 1);

		// Hiding a layer that is already hidden changes nothing.
		manager.setLayerVisible(3, false);
		nodeassert.strictEqual(notified, 1);
	});

	// An empty placeholder shows a dashed frame in the edit view only. A
	// thumbnail or a slideshow renders the same data without the frame.
	it('frames an empty placeholder in the edit view only', function () {
		const manager = new VectorManager();
		manager.handleVectorPrimitivesResponse({
			part: 0,
			slideWidth: 1000,
			slideHeight: 800,
			objects: [
				{
					id: 11,
					emptyPlaceholder: true,
					transform: [300, 0, 0, 200, 10, 20],
					primitives: [],
				},
			],
		});
		const data: any = manager.requestPart(0);

		let recorder = new CanvasRecorder();
		manager.renderInto(recorder as any, data);
		nodeassert.strictEqual(countCalls(recorder, 'setLineDash'), 0);

		recorder = new CanvasRecorder();
		manager.renderInto(recorder as any, data, { editView: true });
		nodeassert.strictEqual(countCalls(recorder, 'setLineDash'), 1);
		nodeassert.ok(recorder.findCall('stroke'), 'the frame is stroked');
	});

	// A text portion names its font face by id. The manager asks the
	// engine for that font so it can be loaded and used for an exact match.
	it('requests the font a text portion names', function () {
		const sent: string[] = [];
		(app as any).socket.sendMessage = (message: string) => sent.push(message);

		const manager = new VectorManager();
		manager.handleVectorPrimitivesResponse({
			part: 0,
			slideWidth: 1000,
			slideHeight: 800,
			objects: [
				{
					id: 1,
					primitives: [
						{ type: 'textSimplePortion', text: 'Hi', fontId: '1092a' } as any,
					],
				},
			],
		});

		(app as any).socket.sendMessage = function () {};
		nodeassert.ok(
			sent.some(
				(message) => message.indexOf('.uno:VectorRenderingFont?id=1092a') >= 0,
			),
			'a font request carries the portion font id',
		);
	});

	// Discarding the whole cache forgets every part, so the next use
	// starts a fresh fetch.
	it('drops all cached parts when the cache is discarded', function () {
		const sent: string[] = [];
		(app as any).socket.sendMessage = (message: string) => sent.push(message);

		const manager = new VectorManager();
		manager.handleVectorPrimitivesResponse({
			part: 0,
			version: 1,
			slideWidth: 1000,
			slideHeight: 800,
			objects: [{ id: 11, primitives: [] }],
		});
		manager.discardAllCache();

		nodeassert.strictEqual(manager.requestPart(0), undefined);
		nodeassert.ok(
			sent.some(
				(message) => message.indexOf('.uno:VectorPrimitives?part=0') >= 0,
			),
			'the dropped part is fetched afresh',
		);
		(app as any).socket.sendMessage = function () {};
	});

	// A graphics-memory reclaim drops the cached parts and tells the
	// listeners, so views re-fetch on their next paint.
	it('drops cached parts and notifies on graphics memory reclaim', function () {
		const manager = new VectorManager();
		manager.handleVectorPrimitivesResponse({
			part: 0,
			version: 1,
			slideWidth: 1000,
			slideHeight: 800,
			objects: [{ id: 11, primitives: [] }],
		});

		let notified = 0;
		manager.onVectorChanged(() => notified++);
		manager.reclaimGraphicsMemory();

		nodeassert.ok(notified > 0, 'listeners hear about the drop');
		nodeassert.strictEqual(manager.requestPart(0), undefined);
	});
});
