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
});
