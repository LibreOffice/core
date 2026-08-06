/*
 * Copyright the Collabora Online contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

// Runs as its own suite (run-tests.sh --suite versionhistory), outside
// the default specs/ glob: the store-count assertions need a fresh app
// instance where no other spec has opened documents yet. The service
// sets a zero merge window, so every save snapshots.

import { readFileSync, readdirSync } from 'fs';
import { join } from 'path';

import { openFixture } from '../lib/file-dialog.js';
import * as draft from '../lib/draft.js';
import * as versions from '../lib/versions.js';
import * as webview from '../lib/webview.js';

describe('Local version history', () => {
	it('keeps the file as opened just before the first save', async () => {
		await openFixture(browser.webEngine, browser.native, 'simple.odt');
		expect(versions.allVersionFiles().length).toBe(0);
		const asOpened = readFileSync(
			join(process.env.CODA_TEST_DOCUMENTS_DIR!, 'simple.odt'),
		);

		// Existing documents open read-only; edits need edit mode.
		await browser.webEngine.execute(() => {
			if (!app.map.isEditMode()) app.map._enterEditMode('edit');
		});
		// No edit: a save of an unmodified document writes the file too.
		await browser.webEngine.execute(() => {
			app.socket.sendMessage('save dontTerminateEdit=1 dontSaveIfUnmodified=0');
		});

		await browser.webEngine.waitUntil(
			async () =>
				versions
					.allVersionFiles()
					.some((file) => readFileSync(file).equals(asOpened)),
			{
				timeout: 20000,
				timeoutMsg: 'no version holds the file as opened after the first save',
			},
		);
	});

	it('keeps a version on save', async () => {
		await draft.modifyDoc(browser.webEngine);

		const before = versions.allVersionFiles().length;
		await browser.webEngine.execute(() => {
			app.socket.sendMessage('save dontTerminateEdit=1 dontSaveIfUnmodified=0');
		});

		await browser.webEngine.waitUntil(
			async () => versions.allVersionFiles().length > before,
			{
				timeout: 20000,
				timeoutMsg: 'no version appeared after saving',
			},
		);
	});

	it('lists versions in the dialog and opens a copy as a draft', async () => {
		const beforeHandles = await browser.webEngine.getWindowHandles();
		const draftsBefore = readdirSync(draft.draftsDir());

		// Editing mode from the previous case keeps the notebookbar - and
		// thus the File tab - around.
		await browser.webEngine.execute(() => {
			(document.getElementById('File-tab-label') as HTMLElement).click();
		});
		await browser.webEngine.waitUntil(
			async () =>
				await browser.webEngine.execute(
					() => !!document.getElementById('backstage-history'),
				),
			{
				timeout: 10000,
				timeoutMsg: 'the backstage did not show a Version History entry',
			},
		);
		await browser.webEngine.execute(() => {
			(document.getElementById('backstage-history') as HTMLElement).click();
		});
		await browser.webEngine.waitUntil(
			async () =>
				await browser.webEngine.execute(
					() => !!document.getElementById('VersionHistoryDialog'),
				),
			{
				timeout: 10000,
				timeoutMsg: 'the version history dialog did not open',
			},
		);

		// No selection defaults to the newest version. JSDialog renders
		// the pushbutton as a container div holding the actual button.
		await browser.webEngine.execute(() => {
			(
				document.getElementById('version-open-copy-button') as HTMLElement
			).click();
		});

		// The copy is named after the document and the version timestamp.
		const newHandle = await webview.switchToNewWebView(
			browser.webEngine,
			beforeHandles,
		);
		await (browser.webEngine as any).waitForCondition(
			() =>
				typeof app !== 'undefined' && app.map && app.map._docLoaded === true,
			{ timeout: 45000, timeoutMsg: 'the version copy did not load' },
		);

		const url = await browser.webEngine.execute(() => window.location.href);
		expect(decodeURIComponent(decodeURIComponent(url))).toContain('/drafts/');
		expect(newHandle).toBeDefined();

		const draftsAfter = readdirSync(draft.draftsDir()).filter(
			(name) => !draftsBefore.includes(name),
		);
		expect(draftsAfter.length).toBe(1);
		expect(draftsAfter[0]).toMatch(/^simple \(.+\)\.odt$/);

		await draft.findTabStripHandle(browser.webEngine);
		const pill = await browser.webEngine.execute((title: string) => {
			const tab = Array.from(document.querySelectorAll('.tab')).find(
				(candidate) => candidate.querySelector('.title')?.textContent === title,
			);
			return tab?.querySelector('.draft')?.textContent ?? null;
		}, draftsAfter[0]);
		expect(pill).toBe('Draft');
		await browser.webEngine.switchToWindow(newHandle);
	});
});
