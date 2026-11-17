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
import { findValueOf } from './helper/VectorJson';

describe('Integration: vector primitives of a slide', function () {
	this.timeout(60000);

	let client: SocketClient;

	before(async function () {
		client = await SocketClient.connectAndLoad(
			BASE_URL,
			sampleUrl('impress.fodp'),
		);
	});

	after(function () {
		if (client) client.close();
	});

	/// The first slide, named by the id the status gives it, is answered in full: the whole
	/// slide at its current version, with the entry of the slide itself first. A slide that no
	/// client asked for before keeps the version it started at, which is 0 in a fresh document.
	it('answers the first slide in full', async function () {
		const status = await client.status();
		nodeassert.ok(
			Array.isArray(status.parts) && status.parts.length > 0,
			'the status should list the slides',
		);
		const partId = String(status.parts[0].part);

		const answer = await client.vectorPrimitives(partId);

		nodeassert.strictEqual(answer.type, 'vectorprimitives');
		nodeassert.strictEqual(answer.partId, partId);
		nodeassert.strictEqual(answer.from, undefined);
		nodeassert.ok(
			Number.isInteger(answer.version) && answer.version >= 0,
			'the version should be a count from 0',
		);
		nodeassert.ok(
			Array.isArray(answer.objects) && answer.objects.length > 0,
			'the slide should have entries',
		);
		nodeassert.strictEqual(answer.objects[0].id, 0);
	});

	/// The third slide shows a PNG image. The answer for it names the type, and the file itself
	/// follows the JSON.
	it('sends an image as the bytes of its file', async function () {
		const status = await client.status();
		const answer = await client.vectorPrimitives(String(status.parts[2].part));
		const checksum = findValueOf(answer, 'checksum');
		nodeassert.notStrictEqual(
			checksum,
			undefined,
			'the slide should name an image',
		);

		const image = await client.vectorGraphics(String(checksum));

		nodeassert.strictEqual(image.type, 'vectorrenderinggraphics');
		nodeassert.strictEqual(image.mime, 'image/png');
		nodeassert.strictEqual(image.data, undefined);
		nodeassert.deepStrictEqual(
			Array.from(image.bytes.subarray(0, 4)),
			[0x89, 0x50, 0x4e, 0x47],
		);
	});
});
