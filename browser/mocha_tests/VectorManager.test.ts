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

	// The page list on screen, as the status message names it: one entry per
	// page of the list the mode names, each with the id of its page.
	function pageList(mode: number, count: number): any[] {
		const prefix =
			mode === cool.VectorMode.MasterPages
				? 'M'
				: mode === cool.VectorMode.NotesPages
					? 'N'
					: 'S';
		const list: any[] = [];
		for (let i = 0; i < count; i++)
			list.push({ part: prefix + String(i), mode: mode });
		return list;
	}

	// A page is asked for by the id the page list on screen names it by, so
	// every test starts with ten slides on screen.
	let originalImpressForTest: any;
	beforeEach(function () {
		originalImpressForTest = (app as any).impress;
		(app as any).impress = { partList: pageList(cool.VectorMode.Slides, 10) };
	});
	afterEach(function () {
		(app as any).impress = originalImpressForTest;
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
			partId: 'S0',
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
			partId: 'S0',
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
			partId: 'S0',
			version: 1,
			objects: [
				{ id: 11, primitives: [] },
				{ id: 22, primitives: [] },
			],
		});

		// Object 22 changed and now carries one primitive. The order is
		// reversed. Object 11 is unchanged.
		const delta: any = {
			partId: 'S0',
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
			partId: 'S0',
			version: 1,
			objects: [
				{ id: 11, primitives: [] },
				{ id: 22, primitives: [] },
			],
		});

		const delta: any = {
			partId: 'S0',
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
			partId: 'S0',
			version: 1,
			objects: [
				{ id: 11, primitives: [] },
				{ id: 22, primitives: [] },
			],
		});

		const delta: any = {
			partId: 'S0',
			version: 2,
			order: [11],
			objects: [],
		};
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
			partId: 'S0',
			version: 1,
			objects: [{ id: 11, primitives: [] }],
		});

		const delta: any = {
			partId: 'S0',
			version: 2,
			order: [99],
			objects: [],
		};
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
			partId: 'S0',
			version: 5,
			objects: [
				{ id: 11, primitives: [] },
				{ id: 22, primitives: [] },
			],
		});

		// A stale delta from version 3 no longer lists object 22.
		const delta: any = {
			partId: 'S0',
			version: 3,
			order: [11],
			objects: [],
		};
		manager.handleVectorPrimitivesDelta(delta);

		const data: any = manager.requestPart(0, cool.VectorMode.Slides);
		nodeassert.strictEqual(data.version, 5);
		nodeassert.strictEqual(data.objects.size, 2);
	});

	// A client that comes back to a new model keeps drawing what it holds until each page
	// arrives again.
	it('keeps drawing the other cached parts when a full response has a new epoch', function () {
		const manager = new VectorManager();
		manager.handleVectorPrimitivesResponse({
			partId: 'S0',
			epoch: 100,
			version: 5,
			objects: [{ id: 11, primitives: [] }],
		});

		manager.handleVectorPrimitivesResponse({
			partId: 'S1',
			epoch: 200,
			version: 1,
			objects: [{ id: 22, primitives: [] }],
		});

		const held: any = manager.requestPart(0, cool.VectorMode.Slides);
		nodeassert.strictEqual(held.version, 5);
		const data: any = manager.requestPart(1, cool.VectorMode.Slides);
		nodeassert.strictEqual(data.version, 1);
	});

	// A delta from a new epoch counts from another start, so it is not applied on top of the
	// content cached for its own part.
	it('drops a cached part instead of applying a delta from a new epoch', function () {
		const manager = new VectorManager();
		manager.handleVectorPrimitivesResponse({
			partId: 'S0',
			epoch: 100,
			version: 5,
			objects: [{ id: 11, primitives: [] }],
		});

		manager.handleVectorPrimitivesDelta({
			partId: 'S0',
			epoch: 200,
			from: 5,
			version: 6,
			objects: [{ id: 22, primitives: [] }],
		});

		nodeassert.strictEqual(
			manager.requestPart(0, cool.VectorMode.Slides),
			undefined,
		);
	});

	// A delta that starts below the version held carries more objects than are needed. Each of
	// them replaces a whole object, so the delta is applied as it is.
	it('applies a delta that starts below the version held', function () {
		const manager = new VectorManager();
		manager.handleVectorPrimitivesResponse({
			partId: 'S0',
			version: 5,
			objects: [
				{ id: 11, primitives: [] },
				{ id: 22, primitives: [] },
			],
		});

		const delta: any = {
			partId: 'S0',
			from: 2,
			version: 6,
			objects: [{ id: 22, primitives: [], width: 400 }],
		};
		manager.handleVectorPrimitivesDelta(delta);

		const data: any = manager.requestPart(0, cool.VectorMode.Slides);
		nodeassert.strictEqual(data.version, 6);
		nodeassert.strictEqual(data.objects.get(22).width, 400);
	});

	// A delta that starts above the version held leaves out the changes in between, and no
	// later delta carries them, so the part is dropped and fetched whole.
	it('drops the part when a delta starts above the version held', function () {
		const manager = new VectorManager();
		manager.handleVectorPrimitivesResponse({
			partId: 'S0',
			version: 5,
			objects: [{ id: 11, primitives: [] }],
		});

		const delta: any = {
			partId: 'S0',
			from: 7,
			version: 8,
			objects: [],
		};
		manager.handleVectorPrimitivesDelta(delta);

		nodeassert.strictEqual(
			manager.requestPart(0, cool.VectorMode.Slides),
			undefined,
		);
	});

	// Versions count in a space the engine names. A model that started counting again can
	// answer with a lower version, and its page still replaces what is cached.
	it('takes a full response from a new epoch whatever its version', function () {
		const manager = new VectorManager();
		manager.handleVectorPrimitivesResponse({
			partId: 'S0',
			epoch: 100,
			version: 5,
			objects: [{ id: 11, primitives: [] }],
		});

		manager.handleVectorPrimitivesResponse({
			partId: 'S0',
			epoch: 200,
			version: 1,
			objects: [{ id: 22, primitives: [] }],
		});

		const data: any = manager.requestPart(0, cool.VectorMode.Slides);
		nodeassert.strictEqual(data.version, 1);
		nodeassert.ok(data.objects.has(22), 'the new page is cached');
	});

	// Runs the steps with the socket and the map recording what the manager
	// sends to the server and tells the rest of the client.
	function recording(steps: () => void): { sent: string[]; fired: string[] } {
		const sent: string[] = [];
		const fired: string[] = [];
		const socket: any = (app as any).socket;
		const originalSendMessage = socket.sendMessage;
		const originalMap = (app as any).map;
		socket.sendMessage = function (message: string) {
			sent.push(message);
		};
		(app as any).map = {
			fire: function (name: string) {
				fired.push(name);
			},
		};
		try {
			steps();
		} finally {
			socket.sendMessage = originalSendMessage;
			(app as any).map = originalMap;
		}
		return { sent: sent, fired: fired };
	}

	// A response for slide i, version 5 of epoch 100.
	const cachedSlide = (i: number): cool.VectorPrimitivesResponse => ({
		partId: 'S' + String(i),
		epoch: 100,
		version: 5,
		objects: [{ id: 11, primitives: [] }],
	});

	// A client that was away was sent none of the changes made while it was, so
	// its cache may be behind. A page drawn after that asks what changed since
	// the version it holds, and is drawn from the cache until the answer comes.
	it('asks what changed on a cached page when it is drawn after being away', function () {
		const manager = new VectorManager();
		manager.handleVectorPrimitivesResponse(cachedSlide(0));

		const { sent, fired } = recording(() => {
			manager.revalidateCachedParts();
			nodeassert.ok(
				manager.requestPart(0, cool.VectorMode.Slides),
				'the part stopped being drawable while it was asked for again',
			);
			manager.requestPart(0, cool.VectorMode.Slides);
		});

		nodeassert.deepStrictEqual(fired, ['vectorcontentstale']);
		nodeassert.strictEqual(sent.length, 1);
		nodeassert.ok(
			sent[0].indexOf(
				'.uno:VectorPrimitives?partid=S0&mode=0&since=5&epoch=100',
			) >= 0,
			'the request does not say what the client holds: ' + sent[0],
		);

		// The answer is a delta, or a header alone when nothing changed. Either
		// way it closes the request, so the next time away asks again.
		manager.handleVectorPrimitivesDelta({
			partId: 'S0',
			epoch: 100,
			from: 5,
			version: 5,
			objects: [],
		});
		const again = recording(() => {
			manager.requestPart(0, cool.VectorMode.Slides);
			manager.revalidateCachedParts();
			manager.requestPart(0, cool.VectorMode.Slides);
		});
		nodeassert.strictEqual(again.sent.length, 1);
	});

	// Coming back with many pages cached asks only for the ones drawn.
	it('leaves a cached page that is not drawn after being away unasked', function () {
		const manager = new VectorManager();
		for (let i = 0; i < 10; i++)
			manager.handleVectorPrimitivesResponse(cachedSlide(i));

		const { sent } = recording(() => {
			manager.revalidateCachedParts();
			manager.requestPart(3, cool.VectorMode.Slides);
		});

		nodeassert.strictEqual(sent.length, 1);
		nodeassert.ok(
			sent[0].indexOf('partid=S3&') >= 0,
			'another page was asked for: ' + sent[0],
		);
	});

	// A page that arrives whole after being away is current, so drawing it asks
	// for nothing.
	it('asks nothing for a page that arrived whole after being away', function () {
		const manager = new VectorManager();
		manager.handleVectorPrimitivesResponse(cachedSlide(0));

		const { sent } = recording(() => {
			manager.revalidateCachedParts();
			const fresh = cachedSlide(0);
			fresh.version = 6;
			manager.handleVectorPrimitivesResponse(fresh);
			manager.requestPart(0, cool.VectorMode.Slides);
		});

		nodeassert.strictEqual(sent.length, 0);
	});

	// A pushed delta for a page that is not cached yet answers no request, so
	// the full request for the page stays the one in flight.
	it('keeps the request for a page open when a delta for it arrives first', function () {
		const manager = new VectorManager();
		const sent: string[] = [];
		const socket: any = (app as any).socket;
		const originalSendMessage = socket.sendMessage;
		socket.sendMessage = function (message: string) {
			sent.push(message);
		};
		try {
			manager.requestPartById('M0', cool.VectorMode.MasterPages);
			manager.handleVectorPrimitivesDelta({
				mode: cool.VectorMode.MasterPages,
				partId: 'M0',
				epoch: 100,
				from: 1,
				version: 2,
				objects: [],
			});
			manager.requestPartById('M0', cool.VectorMode.MasterPages);
		} finally {
			socket.sendMessage = originalSendMessage;
		}

		nodeassert.strictEqual(sent.length, 1);
	});

	// A request sent before the connection dropped gets no answer, so a reconnect forgets it
	// and the next draw asks for the part again. A request still in flight is not sent twice.
	it('asks again for a cached part once its request is forgotten', function () {
		const manager = new VectorManager();
		manager.handleVectorPrimitivesResponse(cachedSlide(0));

		const whileInFlight = recording(() => {
			manager.revalidateCachedParts();
			manager.requestPart(0, cool.VectorMode.Slides);
			manager.revalidateCachedParts();
			manager.requestPart(0, cool.VectorMode.Slides);
		});
		nodeassert.strictEqual(whileInFlight.sent.length, 1);

		const afterReconnect = recording(() => {
			manager.forgetRequestsInFlight();
			manager.revalidateCachedParts();
			manager.requestPart(0, cool.VectorMode.Slides);
		});
		nodeassert.strictEqual(afterReconnect.sent.length, 1);
	});

	// A page removed while the client was away is answered with the header
	// alone, and its cache entry is dropped.
	it('drops a cached part when the answer says the page is gone', function () {
		const manager = new VectorManager();
		manager.handleVectorPrimitivesResponse({
			partId: 'S3',
			version: 5,
			objects: [{ id: 11, primitives: [] }],
		});

		manager.handleVectorPrimitivesResponse({
			mode: 0,
			partId: 'S3',
		});

		nodeassert.strictEqual(
			manager.requestPart(3, cool.VectorMode.Slides),
			undefined,
		);
	});

	// A delta without a version cannot be checked against later ones, so the
	// cache keeps what it held.
	it('leaves a delta without a version unapplied', function () {
		const manager = new VectorManager();
		manager.handleVectorPrimitivesResponse({
			partId: 'S0',
			version: 5,
			objects: [{ id: 11, primitives: [] }],
		});
		manager.handleVectorPrimitivesDelta({
			partId: 'S0',
			objects: [{ id: 11, primitives: [{ type: 'polygonHairline' }] }],
		});
		const data: any = manager.requestPart(0, cool.VectorMode.Slides);
		nodeassert.strictEqual(data.version, 5);
		nodeassert.strictEqual(data.objects.get(11).primitives.length, 0);
	});

	// A full response older than a delta already applied does not roll the
	// cache back.
	it('ignores a full response older than the cache', function () {
		const manager = new VectorManager();
		manager.handleVectorPrimitivesResponse({
			partId: 'S0',
			version: 6,
			objects: [{ id: 11, primitives: [{ type: 'polygonHairline' }] }],
		});
		manager.handleVectorPrimitivesResponse({
			partId: 'S0',
			version: 5,
			objects: [{ id: 11, primitives: [] }],
		});
		const data: any = manager.requestPart(0, cool.VectorMode.Slides);
		nodeassert.strictEqual(data.version, 6);
		nodeassert.strictEqual(data.objects.get(11).primitives.length, 1);
	});

	// A delta that brings a new object without an order has no place to draw
	// it, so the part is fetched whole.
	it('drops the part when a delta brings a new object without an order', function () {
		const manager = new VectorManager();
		manager.handleVectorPrimitivesResponse({
			partId: 'S0',
			version: 1,
			objects: [{ id: 11, primitives: [] }],
		});
		manager.handleVectorPrimitivesDelta({
			partId: 'S0',
			version: 2,
			objects: [{ id: 22, primitives: [] }],
		});
		nodeassert.strictEqual(
			manager.requestPart(0, cool.VectorMode.Slides),
			undefined,
		);
	});

	// A listener that is no longer wanted is not called again.
	it('forgets a listener that was removed', function () {
		const manager = new VectorManager();
		let notified = 0;
		const listener = () => notified++;
		manager.onVectorChanged(listener);
		manager.setHiddenLayers([1]);
		nodeassert.strictEqual(notified, 1);
		manager.offVectorChanged(listener);
		manager.setHiddenLayers([2]);
		nodeassert.strictEqual(notified, 1);
	});

	// The page rectangle rides on the page entry rather than on a field of
	// its own, so it arrives with a full response and a delta that carries
	// that entry updates it, which is how a resized page reaches the client.
	it('takes the page rectangle from the page entry', function () {
		const manager = new VectorManager();
		manager.handleVectorPrimitivesResponse({
			partId: 'S0',
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
			partId: 'S0',
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
			partId: 'S0',
			version: 1,
			objects: [
				{ id: 5, kind: 'page', primitives: [] },
				{ id: 11, primitives: [] },
			],
		});

		const delta: any = {
			partId: 'S0',
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
			partId: 'S0',
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
			partId: 'S0',
			objects: [
				{ id: 11, layer: 0, primitives: [hairline] },
				{ id: 22, layer: 5, primitives: [hairline] },
			],
		});
		const data: any = manager.requestPart(0, cool.VectorMode.Slides);

		manager.setHiddenLayers([5]);
		let recorder = new CanvasRecorder();
		manager.renderInto(recorder as any, data);
		nodeassert.strictEqual(countCalls(recorder, 'stroke'), 1);

		manager.setHiddenLayers([]);
		recorder = new CanvasRecorder();
		manager.renderInto(recorder as any, data);
		nodeassert.strictEqual(countCalls(recorder, 'stroke'), 2);
	});

	// A slide names the master it draws under itself. The master is painted
	// between the slide's own background and its objects, in the master's
	// order. A shared object is drawn as the master carries it. An object
	// that differs per slide is drawn from the slide's own copy, and is left
	// out when the slide has no copy. An object the master hides behind
	// slides is never drawn.
	it('paints the master a page names under the page', function () {
		const hairline = (path: string): any => ({ type: 'polygonHairline', path });
		const manager = new VectorManager();
		manager.handleVectorPrimitivesResponse({
			partId: 'M0',
			mode: cool.VectorMode.MasterPages,
			version: 1,
			objects: [
				{
					id: 0,
					kind: 'page',
					width: 100,
					height: 100,
					primitives: [hairline('M0 0 L9 9')],
				},
				{ id: 1, hiddenBehindSlide: true, primitives: [hairline('M0 0 L1 1')] },
				{ id: 2, primitives: [hairline('M0 0 L2 2')] },
				{ id: 3, slideDependent: true, primitives: [hairline('M0 0 L3 3')] },
				{ id: 4, slideDependent: true, primitives: [hairline('M0 0 L4 4')] },
			],
		});
		manager.handleVectorPrimitivesResponse({
			partId: 'S0',
			mode: cool.VectorMode.Slides,
			version: 1,
			objects: [
				{
					id: 0,
					kind: 'page',
					width: 100,
					height: 100,
					masterPartId: 'M0',
					primitives: [hairline('M0 0 L6 6')],
				},
				{ id: 3, masterContent: true, primitives: [hairline('M0 0 L5 5')] },
				{ id: 9, primitives: [hairline('M0 0 L7 7')] },
			],
		});

		const slide: any = manager.requestPart(0, cool.VectorMode.Slides);
		nodeassert.strictEqual(slide.masterPartId, 'M0');
		nodeassert.strictEqual(
			manager.isPartDrawable(0, cool.VectorMode.Slides),
			true,
		);

		const recorder = new CanvasRecorder();
		manager.renderInto(recorder as any, slide);
		// The slide's background, the shared master object, the slide's copy
		// of the per-slide one, then the slide's own object. The hidden master
		// object and the copy-less one are left out.
		nodeassert.deepStrictEqual(
			recorder.calls
				.filter((call: any) => call.method === 'stroke')
				.map((call: any) => (call.args[0] as any).path),
			['M0 0 L6 6', 'M0 0 L2 2', 'M0 0 L5 5', 'M0 0 L7 7'],
		);
	});

	// A slide can turn the master's objects off. The page entry names the
	// master layers it hides, and the objects on them are not drawn under
	// that slide while another slide still shows them.
	it('leaves out the master layers a page hides', function () {
		const hairline = (path: string): any => ({ type: 'polygonHairline', path });
		const manager = new VectorManager();
		manager.handleVectorPrimitivesResponse({
			partId: 'M0',
			mode: cool.VectorMode.MasterPages,
			version: 1,
			objects: [
				{ id: 0, kind: 'page', width: 100, height: 100, primitives: [] },
				{ id: 2, layer: 2, primitives: [hairline('M0 0 L2 2')] },
				{ id: 3, layer: 1, primitives: [hairline('M0 0 L3 3')] },
			],
		});
		manager.handleVectorPrimitivesResponse({
			partId: 'S0',
			mode: cool.VectorMode.Slides,
			version: 1,
			objects: [
				{
					id: 0,
					kind: 'page',
					width: 100,
					height: 100,
					masterPartId: 'M0',
					masterHiddenLayers: [2],
					primitives: [],
				},
			],
		});

		const slide: any = manager.requestPart(0, cool.VectorMode.Slides);
		const recorder = new CanvasRecorder();
		manager.renderInto(recorder as any, slide);
		nodeassert.deepStrictEqual(
			recorder.calls
				.filter((call: any) => call.method === 'stroke')
				.map((call: any) => (call.args[0] as any).path),
			['M0 0 L3 3'],
		);
	});

	// An edit running on a master object hides the object's own text, and the
	// entry that carries what is typed is on the master part. A slide drawing
	// under that master draws the entry, so the typed text shows on the slide.
	it('draws a master edit on the slides under the master', function () {
		const hairline = (path: string): any => ({ type: 'polygonHairline', path });
		const manager = new VectorManager();
		manager.handleVectorPrimitivesResponse({
			partId: 'M0',
			mode: cool.VectorMode.MasterPages,
			version: 1,
			objects: [
				{ id: 0, kind: 'page', width: 100, height: 100, primitives: [] },
				{ id: 2, textEdit: true, primitives: [] },
				{
					id: -2,
					kind: 'texteditoverlay',
					parent: 2,
					viewId: 1,
					primitives: [hairline('M0 0 L4 4')],
				},
			],
		});
		manager.handleVectorPrimitivesResponse({
			partId: 'S0',
			mode: cool.VectorMode.Slides,
			version: 1,
			objects: [
				{
					id: 0,
					kind: 'page',
					width: 100,
					height: 100,
					masterPartId: 'M0',
					primitives: [],
				},
			],
		});

		const slide: any = manager.requestPart(0, cool.VectorMode.Slides);
		const recorder = new CanvasRecorder();
		manager.renderInto(recorder as any, slide);
		nodeassert.deepStrictEqual(
			recorder.calls
				.filter((call: any) => call.method === 'stroke')
				.map((call: any) => (call.args[0] as any).path),
			['M0 0 L4 4'],
		);
	});

	// A slide that names a master it does not have yet is not drawable, and
	// the master is asked for. It becomes drawable once the master arrives.
	it('waits for the master a page names before drawing it', function () {
		const sent: string[] = [];
		(app as any).socket.sendMessage = function (message: string) {
			sent.push(message);
		};
		const manager = new VectorManager();
		manager.handleVectorPrimitivesResponse({
			partId: 'S2',
			mode: cool.VectorMode.Slides,
			version: 1,
			objects: [
				{
					id: 0,
					kind: 'page',
					width: 100,
					height: 100,
					masterPartId: 'M1',
					primitives: [],
				},
			],
		});
		nodeassert.strictEqual(
			manager.isPartDrawable(2, cool.VectorMode.Slides),
			false,
		);
		nodeassert.ok(
			sent.some(
				(message) =>
					message.indexOf('.uno:VectorPrimitives?partid=M1&mode=1') >= 0,
			),
			'the master the page names is fetched by its id',
		);

		manager.handleVectorPrimitivesResponse({
			partId: 'M1',
			mode: cool.VectorMode.MasterPages,
			version: 1,
			objects: [
				{ id: 0, kind: 'page', width: 100, height: 100, primitives: [] },
			],
		});
		nodeassert.strictEqual(
			manager.isPartDrawable(2, cool.VectorMode.Slides),
			true,
		);
		(app as any).socket.sendMessage = function () {};
	});

	// The engine reports the layers a view hides as a state change. The
	// manager takes the list as it is, and a list that is not one hides
	// nothing. A new list changes what the slide shows, so the views
	// listening for vector changes redraw.
	it('takes the hidden layers from the layer visibility state', function () {
		const manager = new VectorManager();
		let notified = 0;
		manager.onVectorChanged(() => notified++);

		manager.setHiddenLayers([3, 7]);
		nodeassert.strictEqual(manager.isLayerVisible(3), false);
		nodeassert.strictEqual(manager.isLayerVisible(7), false);
		nodeassert.strictEqual(manager.isLayerVisible(4), true);
		nodeassert.strictEqual(notified, 1);

		// The same list again changes nothing.
		manager.setHiddenLayers([7, 3]);
		nodeassert.strictEqual(notified, 1);

		manager.setHiddenLayers('nonsense');
		nodeassert.strictEqual(manager.isLayerVisible(3), true);
		nodeassert.strictEqual(notified, 2);
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
			partId: 'S0',
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
			partId: 'S0',
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
			partId: 'S0',
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
				(message) =>
					message.indexOf('.uno:VectorPrimitives?partid=S0&mode=0') >= 0,
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
			partId: 'S0',
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
			partId: 'S0',
			mode: cool.VectorMode.Slides,
			version: 1,
			objects: [{ id: 11, primitives: [] }],
		});
		manager.handleVectorPrimitivesResponse({
			partId: 'M0',
			mode: cool.VectorMode.MasterPages,
			version: 1,
			objects: [
				{ id: 22, primitives: [] },
				{ id: 33, primitives: [] },
			],
		});

		const slide = manager.requestPart(0, cool.VectorMode.Slides);
		const master = manager.requestPartById('M0', cool.VectorMode.MasterPages);
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
			partId: 'M0',
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
					message.indexOf('.uno:VectorPrimitives?partid=S0&mode=0') >= 0,
			),
			'the slide at the same index is fetched on its own',
		);

		(app as any).socket.sendMessage = function () {};
	});

	// A part the document does not hold is answered with the header alone.
	// The request is closed, and the page is not asked for again until the
	// page list is replaced, which is when the answer could differ.
	it('asks again for a missing part once the page list is replaced', function () {
		const sent: string[] = [];
		(app as any).socket.sendMessage = function (message: string) {
			sent.push(message);
		};

		const manager = new VectorManager();
		nodeassert.strictEqual(
			manager.requestPartById('M4', cool.VectorMode.MasterPages),
			undefined,
		);
		nodeassert.strictEqual(sent.length, 1, 'the part is asked for once');

		// The engine answers that it holds no such page.
		manager.handleVectorPrimitivesResponse({
			mode: cool.VectorMode.MasterPages,
			partId: 'M4',
		});

		// Asking again sends nothing while the page list is the one that did
		// not hold the page, and asks again once the list is replaced.
		nodeassert.strictEqual(
			manager.requestPartById('M4', cool.VectorMode.MasterPages),
			undefined,
		);
		nodeassert.strictEqual(
			sent.length,
			1,
			'a page the document lacks was asked again',
		);
		manager.partListChanged();
		nodeassert.strictEqual(
			manager.requestPartById('M4', cool.VectorMode.MasterPages),
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
		const master = manager.requestPartById(
			response.partId,
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
			partId: 'M0',
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
		const master: any = manager.requestPartById(
			'M0',
			cool.VectorMode.MasterPages,
		);
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
			partId: 'S2',
			version: 1,
			objects: [{ id: 11, primitives: [] }],
		});

		nodeassert.ok(manager.requestPart(2, cool.VectorMode.Slides));
	});

	// A page is cached under its id, so a slide inserted in front of it moves
	// its index and nothing else: the page list names the page at the new
	// index and the cache answers for it without asking the engine again.
	it('keeps a cached page when its index moves', function () {
		const sent: string[] = [];
		(app as any).socket.sendMessage = function (message: string) {
			sent.push(message);
		};
		const originalImpress = (app as any).impress;
		try {
			const manager = new VectorManager();
			manager.handleVectorPrimitivesResponse({
				partId: 'S0',
				mode: cool.VectorMode.Slides,
				version: 3,
				objects: [{ id: 11, primitives: [] }],
			});

			// A new slide is inserted before it, and the status carries the new list.
			(app as any).impress = {
				partList: [
					{ part: 'S9', mode: cool.VectorMode.Slides },
					{ part: 'S0', mode: cool.VectorMode.Slides },
				],
			};
			manager.partListChanged();

			const moved: any = manager.requestPart(1, cool.VectorMode.Slides);
			nodeassert.ok(moved, 'the page is still cached at its new index');
			nodeassert.strictEqual(moved.version, 3);
			nodeassert.deepStrictEqual(moved.order, [11]);
			nodeassert.strictEqual(sent.length, 0, 'nothing was asked for again');

			// The new slide at the old index is another page, asked for by its id.
			nodeassert.strictEqual(
				manager.requestPart(0, cool.VectorMode.Slides),
				undefined,
			);
			nodeassert.ok(
				sent.some(
					(message) =>
						message.indexOf('.uno:VectorPrimitives?partid=S9&mode=0') >= 0,
				),
				'the inserted page is fetched by its id',
			);
		} finally {
			(app as any).impress = originalImpress;
			(app as any).socket.sendMessage = function () {};
		}
	});

	// A delta names the page by its id, so it reaches the page wherever the
	// page now sits in its list.
	it('applies a delta to a page whose index moved', function () {
		const manager = new VectorManager();
		manager.handleVectorPrimitivesResponse({
			partId: 'S0',
			version: 1,
			objects: [{ id: 11, primitives: [] }],
		});

		// Two slides are inserted before it, and the status carries the new list.
		(app as any).impress = {
			partList: [
				{ part: 'S7', mode: cool.VectorMode.Slides },
				{ part: 'S8', mode: cool.VectorMode.Slides },
				{ part: 'S0', mode: cool.VectorMode.Slides },
			],
		};
		manager.partListChanged();

		manager.handleVectorPrimitivesDelta({
			partId: 'S0',
			version: 2,
			objects: [{ id: 11, primitives: [{ type: 'polygonHairline' }] }],
		});

		const data: any = manager.requestPart(2, cool.VectorMode.Slides);
		nodeassert.ok(data, 'the page is found at its new index');
		nodeassert.strictEqual(data.version, 2);
		nodeassert.strictEqual(data.objects.get(11).primitives.length, 1);
	});
	// Two views can edit the same text box at once, each with an entry of its
	// own carrying the same text. Only one of them is drawn: this view's own
	// where it is one of the two, and otherwise the first.
	it('draws one text edit entry per edited object', function () {
		const hairline = (path: string): any => ({ type: 'polygonHairline', path });
		const response: any = {
			partId: 'S0',
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

	describe('Thumbnails', function () {
		let originalDocument: any;
		let originalImage: any;
		let originalMap: any;
		let originalImpress: any;
		let originalDpiScale: number;
		let drawn: any[];
		// One recorder per thumbnail drawn, in the order they were drawn.
		let recorders: any[];

		// The text a thumbnail drew, read back from its recorder.
		const drawnText = (event: any): string =>
			recorders[drawn.indexOf(event)].calls
				.filter(
					(call: any) =>
						call.method === 'fillText' || call.method === 'strokeText',
				)
				.map((call: any) => call.args[0])
				.join(' ');

		beforeEach(function () {
			drawn = [];
			recorders = [];
			originalDocument = (globalThis as any).document;
			originalImage = (globalThis as any).Image;
			originalMap = (app as any).map;
			originalImpress = (app as any).impress;
			originalDpiScale = app.roundedDpiScale;
			app.roundedDpiScale = 1;
			// A thumbnail draws to an offscreen canvas and reports itself
			// through the map, so a recorder stands in for both.
			(globalThis as any).document = {
				createElement: function () {
					return {
						width: 0,
						height: 0,
						getContext: function () {
							recorders.push(new CanvasRecorder(100, 100));
							return recorders[recorders.length - 1];
						},
						toDataURL: function () {
							return 'data:image/png;base64,';
						},
					};
				},
			};
			// The finished thumbnail is handed over as an image, which node
			// does not have.
			(globalThis as any).Image = function () {};
			(app as any).map = {
				_docLayer: { _docType: 'presentation', _selectedPart: 0 },
				fire: function (name: string, event: any) {
					if (name === 'tilepreview') drawn.push(event);
				},
			};
			(app as any).impress = {
				partList: pageList(cool.VectorMode.Slides, 10),
			};
		});

		afterEach(function () {
			(globalThis as any).document = originalDocument;
			(globalThis as any).Image = originalImage;
			(app as any).map = originalMap;
			(app as any).impress = originalImpress;
			app.roundedDpiScale = originalDpiScale;
		});

		// A response for a page with a page entry of the given size and the
		// given objects on it.
		const page = (part: number, mode: number, objects: any[]): any => ({
			mode: mode,
			partId: (mode === cool.VectorMode.MasterPages ? 'M' : 'S') + String(part),
			version: 1,
			objects: [
				{ id: 0, kind: 'page', width: 100, height: 100, primitives: [] },
			].concat(objects),
		});

		const prompt: any = {
			type: 'exclusiveEditView',
			children: [{ type: 'textSimplePortion', text: 'Click to edit' }],
		};

		// A master page is the layout the slides are built on, and its
		// placeholders are what there is to see on it. A thumbnail of one
		// draws them, prompt text and all.
		it('draws the prompt text of a master page thumbnail', function () {
			(app as any).impress = {
				partList: pageList(cool.VectorMode.MasterPages, 5),
			};
			const manager = new VectorManager();
			manager.requestThumbnail(0, 0, cool.VectorMode.MasterPages, 100, 100);
			manager.handleVectorPrimitivesResponse(
				page(0, cool.VectorMode.MasterPages, [
					{ id: 1, emptyPlaceholder: true, primitives: [prompt] },
				]),
			);

			nodeassert.strictEqual(drawn.length, 1);
			nodeassert.strictEqual(drawn[0].mode, cool.VectorMode.MasterPages);
			// The preview names the page it shows by its id and by its index at
			// render time.
			nodeassert.strictEqual(drawn[0].part, 'M0');
			nodeassert.strictEqual(drawn[0].partIndex, 0);
			nodeassert.ok(
				drawnText(drawn[0]).indexOf('Click to edit') >= 0,
				'the master thumbnail drew no prompt text',
			);
		});

		// A request for a page the document does not hold is answered with
		// the header alone. The thumbnail is dropped, so a page that turns
		// up at that index later is not drawn into a preview that has since
		// stopped asking for it.
		it('drops a thumbnail of a part the document does not hold', function () {
			(app as any).impress = {
				partList: pageList(cool.VectorMode.MasterPages, 5),
			};
			const manager = new VectorManager();
			manager.requestThumbnail(0, 4, cool.VectorMode.MasterPages, 100, 100);

			manager.handleVectorPrimitivesResponse({
				mode: cool.VectorMode.MasterPages,
				partId: 'M4',
			});
			nodeassert.strictEqual(drawn.length, 0);

			manager.handleVectorPrimitivesResponse(
				page(4, cool.VectorMode.MasterPages, []),
			);
			nodeassert.strictEqual(drawn.length, 0);
		});

		// A page asked for again after being away answers with a full response,
		// and a preview already shown for it is drawn again from that.
		it('draws a shown preview again when its page arrives anew', function () {
			const manager = new VectorManager();
			manager.requestThumbnail(0, 0, cool.VectorMode.Slides, 100, 100);
			manager.handleVectorPrimitivesResponse(
				page(0, cool.VectorMode.Slides, [{ id: 1, primitives: [] }]),
			);
			nodeassert.strictEqual(drawn.length, 1);

			const again = page(0, cool.VectorMode.Slides, [
				{ id: 1, primitives: [] },
			]);
			again.version = 2;
			manager.handleVectorPrimitivesResponse(again);
			nodeassert.strictEqual(drawn.length, 2);
		});

		// A preview of a page that may be behind the document is drawn from the
		// cache at once, and asks what changed since the version held.
		it('draws a stale preview from the cache and asks what changed', function () {
			const manager = new VectorManager();
			const held = page(0, cool.VectorMode.Slides, [{ id: 1, primitives: [] }]);
			held.epoch = 100;
			manager.handleVectorPrimitivesResponse(held);
			manager.revalidateCachedParts();

			const sent: string[] = [];
			const socket: any = (app as any).socket;
			const originalSendMessage = socket.sendMessage;
			socket.sendMessage = function (message: string) {
				sent.push(message);
			};
			try {
				manager.requestThumbnail(0, 0, cool.VectorMode.Slides, 100, 100);
			} finally {
				socket.sendMessage = originalSendMessage;
			}

			nodeassert.strictEqual(drawn.length, 1);
			nodeassert.strictEqual(sent.length, 1);
			nodeassert.ok(
				sent[0].indexOf('partid=S0&mode=0&since=1&epoch=100') >= 0,
				'the request does not say what the client holds: ' + sent[0],
			);
		});

		// A preview asked for twice while its page is pending is drawn once.
		it('draws a waiting preview once however often it asked', function () {
			const manager = new VectorManager();
			manager.requestThumbnail(0, 0, cool.VectorMode.Slides, 100, 100);
			manager.requestThumbnail(0, 0, cool.VectorMode.Slides, 120, 120);
			manager.handleVectorPrimitivesResponse(
				page(0, cool.VectorMode.Slides, [{ id: 1, primitives: [] }]),
			);
			nodeassert.strictEqual(drawn.length, 1);
			nodeassert.strictEqual(drawn[0].width, 120);
		});

		// A slide stands on its own, so the prompt that invites an edit is
		// not part of the picture.
		it('leaves the prompt text out of a slide thumbnail', function () {
			const manager = new VectorManager();
			manager.requestThumbnail(0, 0, cool.VectorMode.Slides, 100, 100);
			manager.handleVectorPrimitivesResponse(
				page(0, cool.VectorMode.Slides, [
					{ id: 1, emptyPlaceholder: true, primitives: [prompt] },
				]),
			);

			nodeassert.strictEqual(drawn.length, 1);
			nodeassert.strictEqual(
				drawnText(drawn[0]).indexOf('Click to edit'),
				-1,
				'the slide thumbnail drew prompt text',
			);
		});
	});

	// The manager measures text with a stub, so widths do not depend on a
	// canvas. The document state is set up as during a text edit.
	describe('predicted text edits', function () {
		const measure: cool.TextRunMeasure = (_run, text) => text.length * 100;
		class TestManager extends VectorManager {
			protected _textMeasure(): cool.TextRunMeasure {
				return measure;
			}
		}

		// One text box with id 5 at (1000, 2000) holding one run of text.
		function textBox(text: string): any {
			return {
				id: 5,
				primitives: [
					{
						type: 'transform',
						matrix: [1, 0, 0, 1, 1000, 2000],
						children: [
							{
								type: 'textSimplePortion',
								text,
								textPosition: 0,
								textLength: text.length,
								matrix: [1, 0, 0, 1, 0, 0],
								fontSize: 360,
								familyname: 'Arial',
								paragraph: 0,
							},
						],
					},
				],
			};
		}

		function textOf(manager: VectorManager): string {
			const data: any = manager.requestPart(0, cool.VectorMode.Slides);
			return data.objects.get(5).primitives[0].children[0].text;
		}

		let savedMap: any;
		let savedSelections: any;
		let savedCursor: any;
		let savedSequence: number;
		beforeEach(function () {
			const anyApp = app as any;
			savedMap = anyApp.map;
			savedSelections = (window as any).TextSelections;
			savedCursor = app.file.textCursor;
			savedSequence = app.file.inputSequence;
			anyApp.map = {
				_docLayer: { _selectedPart: 0, _onUpdateCursor: function () {} },
			};
			(window as any).TextSelections = { isActive: () => false };
			// The caret sits at the end of "Hi", which spans 1000..1200.
			app.file.textCursor = {
				visible: true,
				rectangle: new cool.SimpleRectangle(1200, 1800, 0, 250),
				logical: { paragraph: 0, index: 2 },
			};
			app.file.inputSequence = 10;
		});
		afterEach(function () {
			(app as any).map = savedMap;
			(window as any).TextSelections = savedSelections;
			app.file.textCursor = savedCursor;
			app.file.inputSequence = savedSequence;
		});

		function managerWith(text: string): VectorManager {
			const manager = new TestManager();
			manager.handleVectorPrimitivesResponse({
				partId: 'S0',
				version: 1,
				objects: [textBox(text)],
			});
			return manager;
		}

		// A manager that holds "Hi" and predicted the 'a' sent as input 11.
		function afterTypingA(): VectorManager {
			const manager = managerWith('Hi');
			manager.predictTextInsert('a');
			app.file.inputSequence = 11;
			return manager;
		}

		// Sends a delta that holds the text box with that text and reflects that input.
		function sendDelta(
			manager: VectorManager,
			inputSequence: number,
			text: string,
			version = 2,
		): void {
			manager.handleVectorPrimitivesDelta({
				partId: 'S0',
				version,
				inputSequence,
				order: [5],
				objects: [textBox(text)],
			});
		}

		// A text box with id 5 holding "line1" and "line2" on two lines of one paragraph,
		// split by a line break, so index 5 is the end of the first run and the start of the
		// second.
		function twoLineBox(first = 'line1'): any {
			const text = first + 'line2';
			const run = (start: number, length: number, f: number): any => ({
				type: 'textSimplePortion',
				text,
				textPosition: start,
				textLength: length,
				matrix: [1, 0, 0, 1, 0, f],
				fontSize: 360,
				familyname: 'Arial',
				paragraph: 0,
			});
			return {
				id: 5,
				primitives: [
					{
						type: 'transform',
						matrix: [1, 0, 0, 1, 1000, 2000],
						children: [run(0, first.length, 0), run(first.length, 5, 400)],
					},
				],
			};
		}

		// A caret update for that input that puts the caret in paragraph 0 at that index, in
		// the object with that id when one is given.
		function caretUpdate(
			inputSequence: number,
			index: number,
			objectId?: number,
		): cool.CaretUpdate {
			const values: cool.CaretUpdate = {
				inputSequence,
				paragraph: '0',
				index: String(index),
			};
			if (objectId !== undefined) values.object = String(objectId);
			return values;
		}

		// The typed character is input 11. A delta for input 10 does not
		// include it, so the edit is replayed and the caret stays after it.
		it('keeps a predicted character when a delta predates it', function () {
			const manager = managerWith('Hi');
			nodeassert.ok(manager.predictTextInsert('a'));
			nodeassert.strictEqual(textOf(manager), 'Hia');
			app.file.inputSequence = 11;

			sendDelta(manager, 10, 'Hi');
			nodeassert.strictEqual(textOf(manager), 'Hia');
			nodeassert.deepStrictEqual(app.file.textCursor.logical, {
				paragraph: 0,
				index: 3,
			});
			nodeassert.strictEqual(app.file.textCursor.rectangle.x1, 1300);
		});

		// Content for the edit's own input includes it, so the edit is dropped
		// and later content stands as it is.
		it('shows later content unchanged once the server includes the edit', function () {
			const manager = afterTypingA();

			sendDelta(manager, 11, 'Hia');
			nodeassert.strictEqual(textOf(manager), 'Hia');
			// Another view appended text. Nothing is replayed on top.
			sendDelta(manager, 11, 'Hia!', 3);
			nodeassert.strictEqual(textOf(manager), 'Hia!');
		});

		// Content for the first of several keystrokes keeps the later ones, in
		// order.
		it('keeps the keystrokes a delta does not include yet, in order', function () {
			const manager = afterTypingA();
			manager.predictTextInsert('b');
			app.file.inputSequence = 12;
			manager.predictTextDelete(1, 0);
			app.file.inputSequence = 13;
			nodeassert.strictEqual(textOf(manager), 'Hia');

			sendDelta(manager, 11, 'Hia');
			nodeassert.strictEqual(textOf(manager), 'Hia');
			nodeassert.deepStrictEqual(app.file.textCursor.logical, {
				paragraph: 0,
				index: 3,
			});
		});

		// A part dropped from the cache comes back in full, and the keystroke that the
		// full response does not include yet shows on it again.
		it('keeps a predicted character on a part fetched again in full', function () {
			const manager = afterTypingA();
			manager.clearCachedPart('S0');
			manager.handleVectorPrimitivesResponse({
				partId: 'S0',
				version: 2,
				inputSequence: 10,
				objects: [textBox('Hi')],
			});
			nodeassert.strictEqual(textOf(manager), 'Hia');
		});

		// A full response without an input sequence stands as it is, and the
		// edits are dropped.
		it('takes content without an input sequence as it is', function () {
			const manager = afterTypingA();
			manager.handleVectorPrimitivesResponse({
				partId: 'S0',
				version: 2,
				objects: [textBox('Hi')],
			});
			nodeassert.strictEqual(textOf(manager), 'Hi');
		});

		// A caret update for an input older than a predicted edit leaves the
		// local caret after the typed text. The update that reflects the edit
		// applies.
		it('keeps the local caret while a caret update lags behind a predicted edit', function () {
			const manager = afterTypingA();
			nodeassert.strictEqual(
				manager.keepPredictedCaret(caretUpdate(10, 2)),
				true,
			);
			nodeassert.strictEqual(
				manager.keepPredictedCaret(caretUpdate(11, 3)),
				false,
			);
			// The edit is still pending, so a stale delta still replays it.
			sendDelta(manager, 10, 'Hi');
			nodeassert.strictEqual(textOf(manager), 'Hia');
		});

		// A caret elsewhere for the same input means the server changed the
		// text differently, so the predicted edits are dropped and the server's
		// content stands.
		it('shows the server text when the server put the caret elsewhere', function () {
			const manager = afterTypingA();
			nodeassert.strictEqual(
				manager.keepPredictedCaret(caretUpdate(11, 5)),
				false,
			);
			sendDelta(manager, 10, 'Hi');
			nodeassert.strictEqual(textOf(manager), 'Hi');
		});

		// A server that ignored the keystroke sends no delta for it, so the client
		// asks for the whole part, and its answer replaces the predicted text.
		it('replaces the predicted text with the whole part when the server put the caret elsewhere', function () {
			const manager = afterTypingA();
			const sent: string[] = [];
			(app as any).socket.sendMessage = (message: string) => sent.push(message);
			manager.keepPredictedCaret(caretUpdate(11, 2));
			(app as any).socket.sendMessage = function () {};
			nodeassert.deepStrictEqual(sent, [
				'commandvalues command=.uno:VectorPrimitives?partid=S0&mode=' +
					String(cool.VectorMode.Slides),
			]);

			manager.handleVectorPrimitivesResponse({
				partId: 'S0',
				version: 1,
				inputSequence: 11,
				objects: [textBox('Hi')],
			});
			nodeassert.strictEqual(textOf(manager), 'Hi');
		});

		// When a predicted edit no longer fits the server's text, the client asks
		// for the whole part again.
		it('asks for the whole part when a replayed edit no longer fits', function () {
			const manager = afterTypingA();
			const sent: string[] = [];
			(app as any).socket.sendMessage = (message: string) => sent.push(message);
			// The server's text box has no paragraph 0, where the edit was typed.
			const moved = textBox('Hi');
			moved.primitives[0].children[0].paragraph = 1;
			manager.handleVectorPrimitivesDelta({
				partId: 'S0',
				version: 2,
				inputSequence: 10,
				order: [5],
				objects: [moved],
			});
			(app as any).socket.sendMessage = function () {};
			nodeassert.deepStrictEqual(sent, [
				'commandvalues command=.uno:VectorPrimitives?partid=S0&mode=' +
					String(cool.VectorMode.Slides),
			]);
		});

		// A caret in another text box for the same input drops the predicted
		// edits, even though the position inside that box matches.
		it('shows the server text when the caret is in another object', function () {
			const manager = afterTypingA();
			nodeassert.strictEqual(
				manager.keepPredictedCaret(caretUpdate(11, 3, 9)),
				false,
			);
			sendDelta(manager, 10, 'Hi');
			nodeassert.strictEqual(textOf(manager), 'Hi');
		});

		// The same input in the edited object confirms the caret.
		it('keeps the predicted text when the caret update names the edited object', function () {
			const manager = afterTypingA();
			nodeassert.strictEqual(
				manager.keepPredictedCaret(caretUpdate(11, 3, 5)),
				false,
			);
			sendDelta(manager, 10, 'Hi');
			nodeassert.strictEqual(textOf(manager), 'Hia');
		});

		// During a running text edit the typed text sits in an entry whose
		// parent is the edited object. A caret update naming that object keeps
		// the edits, and one naming another object drops them.
		it('keeps text typed in a running edit for a caret in the edited object, and shows the server text for a caret in another object', function () {
			const editEntry = (text: string): any => ({
				...textBox(text),
				id: -2,
				kind: 'texteditoverlay',
				viewId: 0,
				parent: 5,
			});
			const editText = (manager: VectorManager): string => {
				const data: any = manager.requestPart(0, cool.VectorMode.Slides);
				return data.objects.get(-2).primitives[0].children[0].text;
			};
			const managerEditing = (): VectorManager => {
				const manager = new TestManager();
				manager.handleVectorPrimitivesResponse({
					partId: 'S0',
					version: 1,
					objects: [{ id: 5, primitives: [], textEdit: true }, editEntry('Hi')],
				});
				return manager;
			};
			const delta = (): any => ({
				partId: 'S0',
				version: 2,
				inputSequence: 10,
				order: [5, -2],
				objects: [editEntry('Hi')],
			});

			const confirmed = managerEditing();
			confirmed.predictTextInsert('a');
			app.file.inputSequence = 11;
			confirmed.keepPredictedCaret(caretUpdate(11, 3, 5));
			confirmed.handleVectorPrimitivesDelta(delta());
			nodeassert.strictEqual(editText(confirmed), 'Hia');

			app.file.inputSequence = 10;
			app.file.textCursor.rectangle = new cool.SimpleRectangle(
				1200,
				1800,
				0,
				250,
			);
			app.file.textCursor.logical = { paragraph: 0, index: 2 };
			const moved = managerEditing();
			moved.predictTextInsert('a');
			app.file.inputSequence = 11;
			moved.keepPredictedCaret(caretUpdate(11, 3, 9));
			moved.handleVectorPrimitivesDelta(delta());
			nodeassert.strictEqual(editText(moved), 'Hi');
		});

		// The caret ends "line1", where index 5 also starts "line2". After the replay the
		// caret stays at the end of the first line, at 1000 plus the six characters of
		// "line1x", with its top 288 above the first baseline.
		it('keeps the caret on its line after replaying an edit at a line break', function () {
			const manager = new TestManager();
			manager.handleVectorPrimitivesResponse({
				partId: 'S0',
				version: 1,
				objects: [twoLineBox()],
			});
			app.file.textCursor.rectangle = new cool.SimpleRectangle(
				1500,
				1800,
				0,
				250,
			);
			app.file.textCursor.logical = { paragraph: 0, index: 5 };
			nodeassert.ok(manager.predictTextInsert('x'));
			app.file.inputSequence = 11;

			manager.handleVectorPrimitivesDelta({
				partId: 'S0',
				version: 2,
				inputSequence: 10,
				order: [5],
				objects: [twoLineBox()],
			});
			const data: any = manager.requestPart(0, cool.VectorMode.Slides);
			nodeassert.strictEqual(
				data.objects.get(5).primitives[0].children[0].text,
				'line1xline2',
			);
			nodeassert.strictEqual(app.file.textCursor.rectangle.x1, 1600);
			nodeassert.strictEqual(app.file.textCursor.rectangle.y1, 1712);
		});

		// An empty index says nothing about where the server put the caret, so the edit
		// stays pending and a later delta that predates it replays it.
		it('keeps the predicted text when a caret update has an empty index', function () {
			const manager = afterTypingA();
			nodeassert.strictEqual(
				manager.keepPredictedCaret({
					inputSequence: 11,
					paragraph: '0',
					index: '',
				}),
				false,
			);
			sendDelta(manager, 10, 'Hi');
			nodeassert.strictEqual(textOf(manager), 'Hia');
		});

		// A caret update without a sequence cannot be compared, so it applies.
		it('moves the caret for a caret update without an input sequence', function () {
			const manager = managerWith('Hi');
			manager.predictTextInsert('a');
			nodeassert.strictEqual(
				manager.keepPredictedCaret({ paragraph: '0', index: '2' }),
				false,
			);
		});
	});
});
