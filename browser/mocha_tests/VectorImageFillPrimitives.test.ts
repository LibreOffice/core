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

describe('VectorPrimitiveRenderer image fills', function () {
	const assert = require('assert');

	function withImage(primitive: any): CanvasRecorder {
		const context = new CanvasRecorder(200, 200);
		const image = new ImageRecorder();
		(image as any).complete = true;
		const renderer = new cool.VectorPrimitiveRenderer(() => image as any);
		renderer.renderPrimitive(context as any, primitive);
		return context;
	}

	it('draws an untiled image once over its graphic range', function () {
		const context = withImage({
			type: 'fillGraphic',
			matrix: [100, 0, 0, 100, 0, 0],
			fillGraphic: { checksum: 1, graphicRange: [0, 0, 1, 1] },
		});

		const draws = context.callsOf('drawImage');
		assert.strictEqual(draws.length, 1);
		assert.deepStrictEqual(draws[0].args.slice(1), [0, 0, 1, 1]);
	});

	it('repeats a tiled image on a grid the size of its range', function () {
		const context = withImage({
			type: 'fillGraphic',
			matrix: [400, 0, 0, 400, 0, 0],
			fillGraphic: {
				checksum: 1,
				graphicRange: [0, 0, 0.5, 0.5],
				tiling: true,
			},
		});

		const draws = context.callsOf('drawImage');
		assert.ok(draws.length >= 4, 'the unit square is covered');
		const places = draws.map((call) => call.args.slice(1).join(','));
		for (const corner of ['0,0', '0.5,0', '0,0.5', '0.5,0.5'])
			assert.ok(
				places.includes(corner + ',0.5,0.5'),
				'a tile sits at ' + corner,
			);
	});

	it('leaves an image fill undrawn until its image arrives', function () {
		const context = new CanvasRecorder(200, 200);
		const renderer = new cool.VectorPrimitiveRenderer(() => undefined);
		renderer.renderPrimitive(
			context as any,
			{
				type: 'fillGraphic',
				matrix: [100, 0, 0, 100, 0, 0],
				fillGraphic: { checksum: 1, graphicRange: [0, 0, 1, 1] },
			} as any,
		);

		assert.strictEqual(context.countOf('drawImage'), 0);
	});
});
