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
	// manager keys the cached objects by that id and keeps the paint order
	// beside them, so a later update can reach one object by its id.
	it('keeps each object id from a primitive tree response', function () {
		const manager = new VectorManager();

		// Two objects with empty primitive lists.
		manager.handleVectorPrimitivesResponse({
			part: 0,
			objects: [
				{ id: 11, primitives: [] },
				{ id: 22, primitives: [] },
			],
		});

		const data = manager.requestPart(0, cool.VectorMode.Slides);
		nodeassert.ok(data, 'part 0 is cached after its response');
		nodeassert.deepStrictEqual(data.order, [11, 22]);
		nodeassert.strictEqual(data.objects.get(11)?.id, 11);
		nodeassert.strictEqual(data.objects.get(22)?.id, 22);
	});

	// The engine stamps each part with a content version. The manager
	// keeps it on the cached tree so a later update can tell whether the
	// client is still in step with the engine.
	it('keeps the part version from a primitive tree response', function () {
		const manager = new VectorManager();

		manager.handleVectorPrimitivesResponse({
			part: 0,
			version: 7,
			objects: [],
		});

		const data = manager.requestPart(0, cool.VectorMode.Slides);
		nodeassert.ok(data, 'part 0 is cached after its response');
		nodeassert.strictEqual(data.version, 7);
	});

	// A delta replaces the objects it carries and leaves the rest alone.
	// An order that comes with it sets the paint order, here reversing the
	// two objects.
	it('applies a delta, reusing cached content for unchanged objects', function () {
		const manager = new VectorManager();
		manager.handleVectorPrimitivesResponse({
			part: 0,
			version: 1,
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

		const data: any = manager.requestPart(0, cool.VectorMode.Slides);
		nodeassert.strictEqual(data.version, 2);
		nodeassert.deepStrictEqual(data.order, [22, 11]);
		nodeassert.strictEqual(data.objects.get(22).primitives.length, 1);
		nodeassert.strictEqual(data.objects.get(11).primitives.length, 0);
	});

	// The order travels only when the object set or its order moved, so a
	// delta without one leaves the order alone and touches nothing but the
	// object it names.
	it('applies a delta that carries no order', function () {
		const manager = new VectorManager();
		manager.handleVectorPrimitivesResponse({
			part: 0,
			version: 1,
			objects: [
				{ id: 11, primitives: [] },
				{ id: 22, primitives: [] },
			],
		});

		const delta: any = {
			part: 0,
			version: 2,
			objects: [{ id: 22, primitives: [{ type: 'polygonHairline' }] }],
		};
		manager.handleVectorPrimitivesDelta(delta);

		const data: any = manager.requestPart(0, cool.VectorMode.Slides);
		nodeassert.deepStrictEqual(data.order, [11, 22]);
		nodeassert.strictEqual(data.objects.get(22).primitives.length, 1);
		nodeassert.strictEqual(data.objects.get(11).primitives.length, 0);
	});

	// An object the order no longer names is gone, so it is dropped from
	// the cache rather than left behind holding its primitives.
	it('drops an object the delta order no longer names', function () {
		const manager = new VectorManager();
		manager.handleVectorPrimitivesResponse({
			part: 0,
			version: 1,
			objects: [
				{ id: 11, primitives: [] },
				{ id: 22, primitives: [] },
			],
		});

		const delta: any = { part: 0, version: 2, order: [11], objects: [] };
		manager.handleVectorPrimitivesDelta(delta);

		const data: any = manager.requestPart(0, cool.VectorMode.Slides);
		nodeassert.strictEqual(data.objects.size, 1);
		nodeassert.ok(!data.objects.has(22), 'the removed object is gone');
		nodeassert.deepStrictEqual(data.order, [11]);
	});

	// When the order names an object the client never cached, the delta
	// cannot be rebuilt, so the part is dropped and re-fetched in full.
	it('drops the part when a delta order names unknown content', function () {
		const manager = new VectorManager();
		manager.handleVectorPrimitivesResponse({
			part: 0,
			version: 1,
			objects: [{ id: 11, primitives: [] }],
		});

		const delta: any = { part: 0, version: 2, order: [99], objects: [] };
		manager.handleVectorPrimitivesDelta(delta);

		// The cache was dropped, so the next request starts a fresh full
		// fetch and has nothing to return yet.
		nodeassert.strictEqual(
			manager.requestPart(0, cool.VectorMode.Slides),
			undefined,
		);
	});

	// A delta computed against an older version can arrive after a newer
	// full response. It describes an older state, so it must not roll the
	// cache backwards.
	it('ignores a delta that is not newer than the cache', function () {
		const manager = new VectorManager();
		manager.handleVectorPrimitivesResponse({
			part: 0,
			version: 5,
			objects: [
				{ id: 11, primitives: [] },
				{ id: 22, primitives: [] },
			],
		});

		// A stale delta from version 3 no longer lists object 22.
		const delta: any = { part: 0, version: 3, order: [11], objects: [] };
		manager.handleVectorPrimitivesDelta(delta);

		const data: any = manager.requestPart(0, cool.VectorMode.Slides);
		nodeassert.strictEqual(data.version, 5);
		nodeassert.strictEqual(data.objects.size, 2);
	});

	// The page rectangle rides on the page entry rather than on a field of
	// its own, so it arrives with a full response and a delta that carries
	// that entry updates it, which is how a resized page reaches the client.
	it('takes the page rectangle from the page entry', function () {
		const manager = new VectorManager();
		manager.handleVectorPrimitivesResponse({
			part: 0,
			version: 1,
			objects: [
				{ id: 0, kind: 'page', width: 1000, height: 800, primitives: [] },
				{ id: 11, primitives: [] },
			],
		});

		let data: any = manager.requestPart(0, cool.VectorMode.Slides);
		nodeassert.strictEqual(data.slideWidth, 1000);
		nodeassert.strictEqual(data.slideHeight, 800);

		manager.handleVectorPrimitivesDelta({
			part: 0,
			version: 2,
			order: [0, 11],
			objects: [
				{ id: 0, kind: 'page', width: 2000, height: 1600, primitives: [] },
			],
		});

		data = manager.requestPart(0, cool.VectorMode.Slides);
		nodeassert.strictEqual(data.slideWidth, 2000);
		nodeassert.strictEqual(data.slideHeight, 1600);
	});

	// The page is an object like the others, first in the order. A delta
	// carries its entry only when the background or the master page
	// changed, and then it replaces the cached one.
	it('replaces the page entry when a delta carries it', function () {
		const manager = new VectorManager();
		manager.handleVectorPrimitivesResponse({
			part: 0,
			version: 1,
			objects: [
				{ id: 5, kind: 'page', primitives: [] },
				{ id: 11, primitives: [] },
			],
		});

		const delta: any = {
			part: 0,
			version: 2,
			order: [5, 11],
			objects: [
				{ id: 5, kind: 'page', primitives: [{ type: 'polygonHairline' }] },
			],
		};
		manager.handleVectorPrimitivesDelta(delta);

		const data: any = manager.requestPart(0, cool.VectorMode.Slides);
		nodeassert.strictEqual(data.version, 2);
		nodeassert.strictEqual(data.objects.get(5).kind, 'page');
		nodeassert.strictEqual(data.objects.get(5).primitives.length, 1);
		nodeassert.deepStrictEqual(data.order, [5, 11]);
	});

	// Each object carries where it sits in the group tree, which layer it
	// is on, whether it is an empty placeholder and where it paints. The
	// manager keeps all of that so the view can hit-test and frame objects.
	it('keeps the geometry and grouping of each object', function () {
		const manager = new VectorManager();
		manager.handleVectorPrimitivesResponse({
			part: 0,
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

		const data = manager.requestPart(0, cool.VectorMode.Slides);
		nodeassert.ok(data, 'part 0 is cached after its response');
		const member = data.objects.get(22);
		nodeassert.ok(member, 'the member is cached under its own id');
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
			objects: [
				{ id: 11, layer: 0, primitives: [hairline] },
				{ id: 22, layer: 5, primitives: [hairline] },
			],
		});
		const data: any = manager.requestPart(0, cool.VectorMode.Slides);

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

	// A placeholder that holds no content yet carries its prompt text inside
	// a wrapper only an editing view unfolds, so a thumbnail and a slideshow
	// leave the prompt out and the editing view shows it.
	it('draws the prompt of an empty placeholder in the edit view only', function () {
		const manager = new VectorManager();
		const prompt = {
			type: 'exclusiveEditView',
			children: [{ type: 'polygonHairline', path: 'M0 0 L1 1' }],
		};
		manager.handleVectorPrimitivesResponse({
			part: 0,
			objects: [
				{ id: 0, kind: 'page', width: 1000, height: 800, primitives: [] },
				{ id: 11, primitives: [prompt] },
			],
		});
		const data: any = manager.requestPart(0, cool.VectorMode.Slides);

		let recorder = new CanvasRecorder();
		manager.renderInto(recorder as any, data);
		nodeassert.strictEqual(countCalls(recorder, 'stroke'), 0);

		recorder = new CanvasRecorder();
		manager.renderInto(recorder as any, data, { editView: true });
		nodeassert.strictEqual(countCalls(recorder, 'stroke'), 1);
	});

	// A text portion names its font face by id. The manager asks the
	// engine for that font so it can be loaded and used for an exact match.
	it('requests the font a text portion names', function () {
		const sent: string[] = [];
		(app as any).socket.sendMessage = (message: string) => sent.push(message);

		const manager = new VectorManager();
		manager.handleVectorPrimitivesResponse({
			part: 0,
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
			objects: [{ id: 11, primitives: [] }],
		});
		manager.discardAllCache();

		nodeassert.strictEqual(
			manager.requestPart(0, cool.VectorMode.Slides),
			undefined,
		);
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
			objects: [{ id: 11, primitives: [] }],
		});

		let notified = 0;
		manager.onVectorChanged(() => notified++);
		manager.reclaimGraphicsMemory();

		nodeassert.ok(notified > 0, 'listeners hear about the drop');
		nodeassert.strictEqual(
			manager.requestPart(0, cool.VectorMode.Slides),
			undefined,
		);
	});

	// The same index names a different page in each mode, so slide 0 and
	// master page 0 are cached apart and each keeps its own content.
	it('caches the same index in two modes as separate pages', function () {
		const manager = new VectorManager();

		manager.handleVectorPrimitivesResponse({
			part: 0,
			mode: cool.VectorMode.Slides,
			version: 1,
			objects: [{ id: 11, primitives: [] }],
		});
		manager.handleVectorPrimitivesResponse({
			part: 0,
			mode: cool.VectorMode.MasterPages,
			version: 1,
			objects: [
				{ id: 22, primitives: [] },
				{ id: 33, primitives: [] },
			],
		});

		const slide = manager.requestPart(0, cool.VectorMode.Slides);
		const master = manager.requestPart(0, cool.VectorMode.MasterPages);
		nodeassert.deepStrictEqual(slide.order, [11]);
		nodeassert.deepStrictEqual(master.order, [22, 33]);
	});

	// A master page and the slide at the same index are cached apart, so
	// the slide is still requested after the master page arrives.
	it('does not let one mode satisfy a request for the other', function () {
		const sent: string[] = [];
		(app as any).socket.sendMessage = function (message: string) {
			sent.push(message);
		};

		const manager = new VectorManager();
		manager.handleVectorPrimitivesResponse({
			part: 0,
			mode: cool.VectorMode.MasterPages,
			version: 1,
			objects: [],
		});

		nodeassert.strictEqual(
			manager.requestPart(0, cool.VectorMode.Slides),
			undefined,
		);
		nodeassert.ok(
			sent.some(
				(message) =>
					message.indexOf('.uno:VectorPrimitives?part=0&mode=0') >= 0,
			),
			'the slide at the same index is fetched on its own',
		);

		(app as any).socket.sendMessage = function () {};
	});

	// A part the document does not hold is answered with the header alone.
	// The manager must drop the request rather than hold it open, so the next
	// draw asks again once the page list has caught up.
	it('retries a part the document did not have', function () {
		const sent: string[] = [];
		(app as any).socket.sendMessage = function (message: string) {
			sent.push(message);
		};

		const manager = new VectorManager();
		nodeassert.strictEqual(
			manager.requestPart(4, cool.VectorMode.MasterPages),
			undefined,
		);
		nodeassert.strictEqual(sent.length, 1, 'the part is asked for once');

		// The engine answers that it holds no such page.
		manager.handleVectorPrimitivesResponse({
			part: 4,
			mode: cool.VectorMode.MasterPages,
		});

		// Asking again sends a fresh request instead of waiting on the first.
		nodeassert.strictEqual(
			manager.requestPart(4, cool.VectorMode.MasterPages),
			undefined,
		);
		nodeassert.strictEqual(sent.length, 2, 'the part is asked for again');

		(app as any).socket.sendMessage = function () {};
	});

	// Master view marks out each placeholder with the name of its area. The
	// name travels with the aids, which the view that edits the page draws
	// in a pass of its own, so the page content stays free of it.
	it('paints the area name of a master placeholder', function () {
		const response = loadVectorRenderingReference('testMasterAreaName');
		const manager = new VectorManager();
		manager.handleVectorPrimitivesResponse(response);
		const master = manager.requestPart(
			response.part,
			cool.VectorMode.MasterPages,
		);
		nodeassert.ok(master, 'the master part is cached');

		const drawnText = (recorder: any): string =>
			recorder.calls
				.filter(
					(call: any) =>
						call.method === 'fillText' || call.method === 'strokeText',
				)
				.map((call: any) => call.args[0])
				.join(' ');

		const aids = new CanvasRecorder(400, 300);
		manager.renderPlaceholderAids(aids as any, master);
		nodeassert.ok(
			drawnText(aids).indexOf('Footer Area') >= 0,
			'the aids drew no area name',
		);
		nodeassert.ok(
			aids.calls.some((call: any) => call.method === 'setLineDash'),
			'the aids drew no dashed boundary',
		);

		const content = new CanvasRecorder(400, 300);
		manager.renderInto(content as any, master, { editView: true });
		nodeassert.strictEqual(
			drawnText(content).indexOf('Footer Area'),
			-1,
			'the page content drew the area name',
		);
	});

	// The aids that mark out a placeholder are an overlay of the view that
	// edits the page, so the page content is drawn without them.
	it('draws the placeholder aids apart from the page content', function () {
		const hairline = (path: string): any => ({ type: 'polygonHairline', path });
		const manager = new VectorManager();
		manager.handleVectorPrimitivesResponse({
			part: 0,
			mode: cool.VectorMode.MasterPages,
			version: 1,
			objects: [
				{
					id: 1,
					primitives: [hairline('M0 0 L1 1')],
					aids: [hairline('M0 0 L2 2'), hairline('M0 0 L3 3')],
				},
			],
		});
		const master: any = manager.requestPart(0, cool.VectorMode.MasterPages);
		nodeassert.ok(master, 'the master part is cached');

		const content = new CanvasRecorder();
		manager.renderInto(content as any, master, { editView: true });
		nodeassert.strictEqual(countCalls(content, 'stroke'), 1);

		const aids = new CanvasRecorder();
		manager.renderPlaceholderAids(aids as any, master);
		nodeassert.strictEqual(countCalls(aids, 'stroke'), 2);
	});

	// A response that names no mode is filed as the slide at that index.
	it('treats a response with no mode as a slide', function () {
		const manager = new VectorManager();
		manager.handleVectorPrimitivesResponse({
			part: 2,
			version: 1,
			objects: [{ id: 11, primitives: [] }],
		});

		nodeassert.ok(manager.requestPart(2, cool.VectorMode.Slides));
	});

	// Two views can edit the same text box at once, each with an entry of its
	// own carrying the same text. Only one of them is drawn: this view's own
	// where it is one of the two, and otherwise the first.
	it('draws one text edit entry per edited object', function () {
		const hairline = (path: string): any => ({ type: 'polygonHairline', path });
		const response: any = {
			part: 0,
			version: 1,
			objects: [
				{ id: 0, kind: 'page', width: 100, height: 100, primitives: [] },
				{ id: 11, textEdit: true, primitives: [] },
				{
					id: -2,
					kind: 'texteditoverlay',
					parent: 11,
					viewId: 1,
					primitives: [hairline('M0 0 L1 1')],
				},
				{
					id: -3,
					kind: 'texteditoverlay',
					parent: 11,
					viewId: 2,
					primitives: [hairline('M0 0 L2 2'), hairline('M0 0 L3 3')],
				},
			],
		};
		const originalMap = (app as any).map;
		try {
			// This client is view 2, so its own entry is the one drawn.
			(app as any).map = { _docLayer: { _viewId: 2 } };
			let manager = new VectorManager();
			manager.handleVectorPrimitivesResponse(response);
			let recorder = new CanvasRecorder();
			manager.renderInto(
				recorder as any,
				manager.requestPart(0, cool.VectorMode.Slides) as any,
				{
					editView: true,
				},
			);
			nodeassert.strictEqual(countCalls(recorder, 'stroke'), 2);

			// A view that is not editing the box draws the first entry.
			(app as any).map = { _docLayer: { _viewId: 7 } };
			manager = new VectorManager();
			manager.handleVectorPrimitivesResponse(response);
			recorder = new CanvasRecorder();
			manager.renderInto(
				recorder as any,
				manager.requestPart(0, cool.VectorMode.Slides) as any,
			);
			nodeassert.strictEqual(countCalls(recorder, 'stroke'), 1);
		} finally {
			(app as any).map = originalMap;
		}
	});
});
