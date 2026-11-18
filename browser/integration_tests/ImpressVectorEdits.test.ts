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
import {
	Key,
	TWIPS_PER_CM,
	centerOf,
	entriesOf,
	entryOfKind,
	entryWithFill,
	findTextPortion,
} from './helper/VectorJson';

/// How long an edit may take to reach the client as a pushed delta.
const PUSH_TIMEOUT_MS = 10000;

describe('Integration: vector deltas after slide edits', function () {
	this.timeout(60000);

	let client: SocketClient;
	/// The page ids of the slides, in the order of the sample: Order, Move and Background. Each
	/// test edits its own slide, so the edits of one test do not reach another.
	let slideIds: string[];

	before(async function () {
		client = await SocketClient.connectAndLoad(
			BASE_URL,
			sampleUrl('impress-vector-edits.fodp'),
		);
		const status = await client.status();
		slideIds = status.parts.map((part: any) => String(part.part));
		nodeassert.strictEqual(slideIds.length, 3);
	});

	after(function () {
		if (client) client.close();
	});

	/// Bringing the lower of two objects to the front changes the paint order. The delta carries
	/// the new order, with the page entry first and the two objects swapped.
	it('sends the new order when an object comes to the front', async function () {
		const partId = slideIds[0];
		client.setClientPart(partId);
		const full = await client.vectorPrimitives(partId);
		const lower = entryWithFill(full, '#4472c4');
		const upper = entryWithFill(full, '#c00000');
		nodeassert.ok(lower && upper, 'both rectangles should name their fill');

		// The point lies in the lower rectangle, outside the one that covers its corner.
		const mark = client.mark();
		client.click(1.5 * TWIPS_PER_CM, 1.5 * TWIPS_PER_CM);
		client.uno('.uno:BringToFront');

		const assertNewOrder = (delta: any) => {
			nodeassert.strictEqual(delta.type, 'vectorprimitivesdelta');
			nodeassert.ok(delta.version > full.version);
			nodeassert.strictEqual(delta.order.length, 3);
			nodeassert.strictEqual(delta.order[1], upper.id);
			nodeassert.strictEqual(delta.order[2], lower.id);
		};
		assertNewOrder(await client.vectorDelta(partId, PUSH_TIMEOUT_MS, mark));
		assertNewOrder(
			await client.vectorPrimitives(partId, {
				since: full.version,
				epoch: full.epoch,
			}),
		);
	});

	/// Moving one of two objects changes that object alone. The set of objects stays, so the
	/// delta carries no order, and only the moved object.
	it('sends only the object that moved', async function () {
		const partId = slideIds[1];
		client.setClientPart(partId);
		const full = await client.vectorPrimitives(partId);
		nodeassert.strictEqual(entriesOf(full).length, 3);
		const moved = entryWithFill(full, '#4472c4');
		nodeassert.ok(moved, 'the blue rectangle should name its fill');

		const mark = client.mark();
		const center = centerOf(moved);
		client.click(center.x, center.y);
		client.pressKey(Key.RIGHT);

		const assertOnlyMoved = (delta: any) => {
			nodeassert.strictEqual(delta.type, 'vectorprimitivesdelta');
			nodeassert.strictEqual(delta.order, undefined);
			nodeassert.strictEqual(entriesOf(delta).length, 1);
			nodeassert.strictEqual(delta.objects[0].id, moved.id);
		};
		assertOnlyMoved(await client.vectorDelta(partId, PUSH_TIMEOUT_MS, mark));
		assertOnlyMoved(
			await client.vectorPrimitives(partId, {
				since: full.version,
				epoch: full.epoch,
			}),
		);
	});

	/// Text in the automatic color keeps that color through a change of the slide background.
	/// The page entry carries the new backdrop, so the delta has the page and leaves the
	/// unchanged text out.
	it('sends the page and not the text when the background changes', async function () {
		const partId = slideIds[2];
		client.setClientPart(partId);
		const full = await client.vectorPrimitives(partId);
		const text = entriesOf(full).find((entry) =>
			findTextPortion(entry, 'Hello'),
		);
		nodeassert.ok(text, 'no entry holds the text');

		const mark = client.mark();
		client.uno('.uno:FillPageColor', {
			'FillColor.Color': { type: 'long', value: 0 },
		});

		const assertPageOnly = (delta: any) => {
			nodeassert.strictEqual(delta.type, 'vectorprimitivesdelta');
			nodeassert.ok(
				!entriesOf(delta).some((entry) => entry.id === text.id),
				'the text travelled with the background',
			);
			const page = entryOfKind(delta, 'page');
			nodeassert.ok(page, 'the page did not travel');
			nodeassert.strictEqual(page.backdrop, '#000000');
		};
		assertPageOnly(await client.vectorDelta(partId, PUSH_TIMEOUT_MS, mark));
		assertPageOnly(
			await client.vectorPrimitives(partId, {
				since: full.version,
				epoch: full.epoch,
			}),
		);
	});
});
