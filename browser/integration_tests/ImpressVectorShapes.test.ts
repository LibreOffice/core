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

import { strict as nodeassert } from 'assert';
import { SocketClient } from './helper/SocketClient';
import { BASE_URL, sampleUrl } from './helper/Environment';
import { entriesOf } from './helper/VectorJson';

/// A page id that no page of the sample has.
const MISSING_PART_ID = '{6F1D3C9A-2B4E-4F8A-9C1D-0E7B5A3F2D61}';

describe('Integration: vector primitives of slide shapes', function () {
	this.timeout(60000);

	let client: SocketClient;
	/// The page ids of the slides, in the order of the sample: Geometry, Colors, Rotated and
	/// Versions.
	let slideIds: string[];

	before(async function () {
		client = await SocketClient.connectAndLoad(
			BASE_URL,
			sampleUrl('impress-vector-shapes.fodp'),
		);
		const status = await client.status();
		slideIds = status.parts.map((part: any) => String(part.part));
		nodeassert.strictEqual(slideIds.length, 4);
	});

	after(function () {
		if (client) client.close();
	});

	/// An entry says where its object paints, and how the unit square maps onto that box, both
	/// in twips. The slide holds the page entry and the rectangle alone.
	it('places an object entry by its box and transform', async function () {
		const answer = await client.vectorPrimitives(slideIds[0]);
		nodeassert.strictEqual(entriesOf(answer).length, 2);

		// The box is rounded to whole twips, edge by edge, and its width and height count both
		// edges in, so each can be a twip off.
		const entry = answer.objects[1];
		for (const [value, expected] of [
			[entry.x, 567],
			[entry.y, 1134],
			[entry.width, 2268],
			[entry.height, 1701],
		])
			nodeassert.ok(
				Math.abs(value - expected) <= 1,
				value + ' is not ' + expected,
			);

		// An unrotated rectangle scales the unit square to its size and moves it to its top
		// left corner, within a twip of the geometry in the sample.
		nodeassert.strictEqual(entry.transform.length, 6);
		const expected = [2267.7, 0, 0, 1700.8, 566.9, 1133.9];
		for (let i = 0; i < 6; ++i)
			nodeassert.ok(
				Math.abs(entry.transform[i] - expected[i]) <= 1,
				'transform[' + i + '] is ' + entry.transform[i],
			);
	});

	/// The properties of an object entry name the fill and the line color where they are solid.
	/// A text box with neither carries no properties.
	it('names the solid colors of an object', async function () {
		const answer = await client.vectorPrimitives(slideIds[1]);
		nodeassert.strictEqual(entriesOf(answer).length, 3);

		nodeassert.strictEqual(answer.objects[1].properties.fillColor, '#4472c4');
		nodeassert.strictEqual(answer.objects[1].properties.lineColor, '#203864');
		nodeassert.strictEqual(answer.objects[2].properties, undefined);
	});

	/// A rotation shows in the transform, while the box stays the axis-aligned box around the
	/// rotated object.
	it('turns the transform of a rotated object and keeps its box upright', async function () {
		const answer = await client.vectorPrimitives(slideIds[2]);
		nodeassert.strictEqual(entriesOf(answer).length, 2);

		// A quarter turn puts the whole scale into the entries off the diagonal, within a twip
		// of the size in the sample.
		const transform = answer.objects[1].transform;
		nodeassert.ok(
			Math.abs(transform[0]) <= 0.5,
			'transform[0] is ' + transform[0],
		);
		nodeassert.ok(
			Math.abs(Math.abs(transform[1]) - 2267.7) <= 1,
			'transform[1] is ' + transform[1],
		);
		nodeassert.ok(
			Math.abs(Math.abs(transform[2]) - 2267.7) <= 1,
			'transform[2] is ' + transform[2],
		);
		nodeassert.ok(
			Math.abs(transform[3]) <= 0.5,
			'transform[3] is ' + transform[3],
		);

		// A square turned by a quarter covers the same box. The box is rounded to whole twips,
		// so it can be a twip off either way.
		const entry = answer.objects[1];
		for (const [value, expected] of [
			[entry.x, 2835],
			[entry.y, 2835],
			[entry.width, 2268],
			[entry.height, 2268],
		])
			nodeassert.ok(
				Math.abs(value - expected) <= 1,
				value + ' is not ' + expected,
			);
	});

	/// A client that says which version it holds gets the change since then: the header alone
	/// when nothing changed, and the whole slide when the version counts in another epoch,
	/// names no epoch, or was never served.
	it('answers a request with what the client lacks', async function () {
		const partId = slideIds[3];
		const full = await client.vectorPrimitives(partId);
		nodeassert.strictEqual(full.type, 'vectorprimitives');
		nodeassert.strictEqual(entriesOf(full).length, 2);
		const version = full.version;
		const epoch = full.epoch;

		const quiet = await client.vectorPrimitives(partId, {
			since: version,
			epoch,
		});
		nodeassert.strictEqual(quiet.type, 'vectorprimitivesdelta');
		nodeassert.strictEqual(quiet.from, version);
		nodeassert.deepStrictEqual(quiet.objects, []);

		const otherEpoch = await client.vectorPrimitives(partId, {
			since: version,
			epoch: epoch + 1,
		});
		nodeassert.strictEqual(otherEpoch.type, 'vectorprimitives');
		nodeassert.strictEqual(entriesOf(otherEpoch).length, 2);

		const noEpoch = await client.vectorPrimitives(partId, { since: version });
		nodeassert.strictEqual(noEpoch.type, 'vectorprimitives');
		nodeassert.strictEqual(entriesOf(noEpoch).length, 2);

		const ahead = await client.vectorPrimitives(partId, {
			since: version + 100,
			epoch,
		});
		nodeassert.strictEqual(ahead.type, 'vectorprimitives');

		// A request for a page that is not there gets the header alone.
		const gone = await client.vectorPrimitives(MISSING_PART_ID, {
			since: version,
		});
		nodeassert.strictEqual(gone.type, 'vectorprimitives');
		nodeassert.strictEqual(gone.objects, undefined);
	});

	/// A client whose page list is behind a mode switch can ask for a master that is not there.
	/// It gets the header, so it can tell the request was seen.
	it('answers a missing master with the header alone', async function () {
		const answer = await client.vectorPrimitives(MISSING_PART_ID, { mode: 1 });
		nodeassert.strictEqual(answer.type, 'vectorprimitives');
		nodeassert.strictEqual(answer.partId, MISSING_PART_ID);
		nodeassert.strictEqual(answer.mode, 1);
		nodeassert.strictEqual(answer.order, undefined);
		nodeassert.strictEqual(answer.objects, undefined);
	});

	/// A page id names one page, and the mode names the list it is asked from. The id of a slide
	/// names no master page, so the answer is the header alone, with the id it answers.
	it('answers a slide id asked as a master with the header alone', async function () {
		const answer = await client.vectorPrimitives(slideIds[3], { mode: 1 });
		nodeassert.strictEqual(answer.type, 'vectorprimitives');
		nodeassert.strictEqual(answer.partId, slideIds[3]);
		nodeassert.strictEqual(answer.mode, 1);
		nodeassert.strictEqual(answer.objects, undefined);
	});

	/// A mode outside the page lists names no page, so the answer has no content, not the slide
	/// with that id.
	it('answers a mode that names no page list with the header alone', async function () {
		const answer = await client.vectorPrimitives(slideIds[3], { mode: 3 });
		nodeassert.strictEqual(answer.mode, 3);
		nodeassert.strictEqual(answer.order, undefined);
		nodeassert.strictEqual(answer.objects, undefined);
	});

	/// A request for an image the document does not know still gets a typed answer that names
	/// the checksum, only with no image data.
	it('answers an unknown image checksum with a typed answer', async function () {
		const answer = await client.vectorGraphics('12345');
		nodeassert.strictEqual(answer.type, 'vectorrenderinggraphics');
		nodeassert.strictEqual(Number(answer.checksum), 12345);
		nodeassert.strictEqual(answer.data, undefined);
		nodeassert.strictEqual(answer.bytes, undefined);
	});
});
