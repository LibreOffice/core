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

describe('VectorResourceWalker', function () {
	const assert = require('assert');

	function walk(primitives: any[]): {
		checksums: Set<number>;
		fontIds: Set<string>;
	} {
		const checksums = new Set<number>();
		const fontIds = new Set<string>();
		new cool.VectorResourceWalker(checksums, fontIds).walkPrimitives(
			primitives,
		);
		return { checksums: checksums, fontIds: fontIds };
	}

	it('finds the resources inside a transparency mask', function () {
		const found = walk([
			{
				type: 'transparence',
				children: [{ type: 'textSimplePortion', fontId: 'content' }],
				transparence: [{ type: 'textSimplePortion', fontId: 'mask' }],
			},
		]);
		assert.deepStrictEqual([...found.fontIds].sort(), ['content', 'mask']);
	});

	it('finds the resources under a uniform transparency', function () {
		const found = walk([
			{
				type: 'unifiedTransparence',
				transparence: 0.5,
				children: [{ type: 'textSimplePortion', fontId: 'content' }],
			},
		]);
		assert.deepStrictEqual([...found.fontIds], ['content']);
	});
});
