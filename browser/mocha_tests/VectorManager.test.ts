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
});
