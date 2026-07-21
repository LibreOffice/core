/*
 * Copyright the Collabora Online contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

import { openFixture } from '../lib/file-dialog.js';

// Saving a coda-m document writes the file and must then refresh the app's
// record of the file's date on disk. If that record is left stale, the next
// save reports the file as changed by another application and refuses. This
// walks the reported reproduction - edit, save, edit, save, all from inside
// the WebView (the Save button path) - and checks the second save raises no
// such error.
//
// The error is a document-modal alert. The native driver reports an alert in
// its page source as a <dialog> element, so the check is that no dialog
// appears after the second save.

const we = (): WebdriverIO.Browser & {
	waitForCondition(
		predicate: () => boolean,
		opts?: { timeout?: number; interval?: number; timeoutMsg?: string },
	): Promise<boolean>;
} => browser.webEngine;

// Existing documents open in the read-only view mode; switch to edit mode so
// editing commands are enabled.
async function enterEditMode(): Promise<void> {
	await we().execute(() => {
		if (!app.map.isEditMode()) app.map._enterEditMode('edit');
	});
	await we().waitUntil(async () => we().execute(() => app.map.isEditMode()), {
		timeout: 15000,
		interval: 200,
		timeoutMsg: 'document did not enter edit mode',
	});
}

// Insert a character so the document has unsaved edits, then wait for the
// engine to report the modified state back.
async function makeModified(): Promise<void> {
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
			timeoutMsg: 'the edit did not mark the document modified',
		},
	);
}

// Save through the WebView Save button path, then wait for the save to
// round-trip and the document to become unmodified.
async function save(): Promise<void> {
	await we().execute(() => {
		app.map.save(false, false);
	});
	await we().waitUntil(
		async () => we().execute(() => app.file.modified === false),
		{
			timeout: 20000,
			interval: 250,
			timeoutMsg: 'the save did not complete (the document stayed modified)',
		},
	);
}

describe('Saving a document more than once', () => {
	before(function () {
		// The stale-date defect is in the coda-m save path, and the alert it
		// raised is read from the coda-m native driver's page source, so run
		// this only there.
		if (process.env.CODA_PLATFORM !== 'macos') {
			this.skip();
		}
	});

	it('does not report the file as changed by another application', async function () {
		await openFixture(browser.webEngine, browser.native, 'simple.odt');
		await enterEditMode();

		await makeModified();
		await save();

		await makeModified();
		await save();

		// A save error would appear as a document-modal alert, which the native
		// driver reports as a <dialog> element. Give one a moment to appear and
		// fail if it does.
		let dialogAppeared = false;
		try {
			await browser.native.waitUntil(
				async () => /<dialog\b/.test(await browser.native.getPageSource()),
				{ timeout: 4000, interval: 300 },
			);
			dialogAppeared = true;
		} catch {
			dialogAppeared = false;
		}
		expect(dialogAppeared).toBe(false);
	});
});
