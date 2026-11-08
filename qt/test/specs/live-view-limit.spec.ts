/*
 * Copyright the Collabora Online contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

import { statSync } from 'fs';
import { join } from 'path';
import { copyFixture, openFixture } from '../lib/file-dialog.js';

// A window keeps a web view only for the documents the user was on most recently, as many as
// CODA_LIVE_VIEWS says, five unless it is set. A document further back loses its web view and
// stays loaded in the engine, and its page disappears from the window handles. When the user goes
// back to it, a new page joins the document that is still loaded, and the user can edit it as
// before. A document with an unsaved change is saved before its web view goes.
//
// The app reads CODA_LIVE_VIEWS from the environment that the test service starts it with, which is
// the environment of this test run too. Going back to a document is done by opening its file
// again, which brings its tab to the front.

const we = (): typeof browser.webEngine => browser.webEngine;

// The same reading of CODA_LIVE_VIEWS as the app's: a whole number, zero for no limit, and the
// default for anything else.
function liveViewLimit(): number {
	const text = process.env.CODA_LIVE_VIEWS ?? '';
	return /^[0-9]+$/.test(text) ? Number(text) : 5;
}

// A web view is kept for five seconds after it is built and the limit runs a second after the
// last tab switch, and a save comes first for a modified document.
const DROP_TIMEOUT = 45000;

async function waitUntilDropped(handle: string, what: string): Promise<void> {
	await we().waitUntil(
		async () => !(await we().getWindowHandles()).includes(handle),
		{
			timeout: DROP_TIMEOUT,
			interval: 500,
			timeoutMsg: `the page of ${what} is still there, ${liveViewLimit()} documents later`,
		},
	);
}

async function openCopies(count: number): Promise<void> {
	for (let i = 0; i < count; i++)
		await openFixture(we(), browser.native, copyFixture('simple.odt'));
}

function modificationTime(fileName: string): number {
	return statSync(join(process.env.CODA_TEST_DOCUMENTS_DIR!, fileName)).mtimeMs;
}

describe('Live-view limit', () => {
	// The document that loses its web view in both tests, and its page while it has one.
	let first = '';
	let firstHandle = '';

	before(function () {
		// CODA-M counts the documents of all windows together and keeps every document that is
		// on the screen, and each of its documents has a window of its own.
		if (
			!['qt', 'windows'].includes(process.env.CODA_PLATFORM ?? '') ||
			liveViewLimit() === 0
		)
			this.skip();
	});

	it('gives a document past the limit a new page that can edit it', async function () {
		first = copyFixture('simple.odt');
		firstHandle = await openFixture(we(), browser.native, first);
		await openCopies(liveViewLimit());
		await waitUntilDropped(firstHandle, first);

		firstHandle = await openFixture(we(), browser.native, first);
		expect(
			await we().execute(() => decodeURIComponent(window.location.href)),
		).toContain(first);

		// The document opens in the read-only view mode, and goes to edit mode only when its
		// file can be written.
		await we().execute(() => {
			if (!app.map.isEditMode()) app.map._enterEditMode('edit');
		});
		await we().waitUntil(async () => we().execute(() => app.map.isEditMode()), {
			timeout: 15000,
			interval: 200,
			timeoutMsg: 'the document did not enter edit mode in its new page',
		});

		// The edit goes to the engine, and the modified state comes back to the new page.
		await we().execute(() => {
			app.map.sendUnoCommand('.uno:InsertText', {
				Text: { type: 'string', value: 'x' },
			});
		});
		await we().waitUntil(
			async () => we().execute(() => app.file.modified === true),
			{
				timeout: 15000,
				interval: 250,
				timeoutMsg:
					'the new page did not hear back from the engine after an edit',
			},
		);
	});

	it('saves a modified document before its page goes', async function () {
		if (!firstHandle) this.skip();

		const before = modificationTime(first);
		await openCopies(liveViewLimit());
		await waitUntilDropped(firstHandle, `the modified ${first}`);

		expect(modificationTime(first)).toBeGreaterThan(before);
	});
});
