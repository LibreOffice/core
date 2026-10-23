/*
 * Copyright the Collabora Online contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

// The Version History section of the Options dialog reads the limits from
// the app, saves changed ones to coda-prefs.json and deletes the store.
// The service seeds coda-prefs.json with a zero merge window.

import { copyFileSync, existsSync, readFileSync } from 'fs';
import { dirname, join } from 'path';

import { openFixture } from '../lib/file-dialog.js';
import * as draft from '../lib/draft.js';
import * as versions from '../lib/versions.js';

function prefsFile(): string {
	return join(dirname(process.env.CODA_TEST_RECENTS_FILE!), 'coda-prefs.json');
}

function readPrefs(): Record<string, string> {
	if (!existsSync(prefsFile())) return {};
	return JSON.parse(readFileSync(prefsFile(), 'utf8'));
}

async function openOptions(): Promise<void> {
	await browser.webEngine.execute(() => {
		app.map.settings.showSettingsDialog();
	});
	// The frame of a dialog that opens over an edited document can be
	// replaced once, so the switch is tried again.
	await browser.webEngine.waitUntil(
		async () => {
			const frame = await browser.webEngine.$('iframe.iframe-settings-modal');
			if (!(await frame.isExisting())) return false;
			try {
				await browser.webEngine.switchToFrame(frame);
				return true;
			} catch {
				return false;
			}
		},
		{
			timeout: 15000,
			timeoutMsg: 'the options dialog did not open',
		},
	);
	await browser.webEngine.waitUntil(
		async () =>
			await browser.webEngine.execute(
				() => !!document.getElementById('maxPerDoc'),
			),
		{
			timeout: 15000,
			timeoutMsg: 'the options dialog has no version limits',
		},
	);
}

async function readLimits(): Promise<Record<string, string>> {
	return browser.webEngine.execute(() => {
		const value = (id: string) =>
			(document.getElementById(id) as HTMLInputElement).value;
		return {
			maxPerDoc: value('maxPerDoc'),
			mergeWindowMins: value('mergeWindowMins'),
			maxFileSizeMB: value('maxFileSizeMB'),
			maxStoreSizeMB: value('maxStoreSizeMB'),
		};
	});
}

async function setLimit(id: string, value: string): Promise<void> {
	await browser.webEngine.execute(
		(inputId: string, inputValue: string) => {
			const input = document.getElementById(inputId) as HTMLInputElement;
			input.value = inputValue;
			input.dispatchEvent(new Event('change'));
		},
		id,
		value,
	);
}

async function closeOptions(buttonId: string): Promise<void> {
	await browser.webEngine.switchToParentFrame();
	await browser.webEngine.execute((id) => {
		(document.getElementById(id) as HTMLElement).click();
	}, buttonId);
}

describe('Options dialog version settings', () => {
	before(async function () {
		this.timeout(120000);
		// Under its own name: earlier specs in this app instance have
		// simple.odt open already, and opening it again opens no new view.
		const docs = process.env.CODA_TEST_DOCUMENTS_DIR!;
		copyFileSync(join(docs, 'simple.odt'), join(docs, 'version-settings.odt'));
		await openFixture(
			browser.webEngine,
			browser.native,
			'version-settings.odt',
		);
	});

	after(async () => {
		// Leave the default limits for the specs that follow.
		await browser.webEngine.switchToParentFrame();
		await browser.webEngine.execute(() => {
			window.postMobileMessage(
				'SETVERSIONSTORELIMITS {"maxPerDoc":120,"maxFileSizeMB":128,"maxStoreSizeMB":4096}',
			);
		});
	});

	it('shows the version limits from the prefs', async () => {
		await openOptions();
		expect(await readLimits()).toEqual({
			maxPerDoc: '120',
			mergeWindowMins: '0',
			maxFileSizeMB: '128',
			maxStoreSizeMB: '4096',
		});
		await closeOptions('iframe-settings-cancel');
	});

	it('saves a changed limit to the prefs and the running store', async () => {
		await openOptions();
		await browser.webEngine.execute(() => {
			const input = document.getElementById('maxPerDoc') as HTMLInputElement;
			input.value = '7';
			input.dispatchEvent(new Event('change'));
		});
		await closeOptions('iframe-settings-save');

		await browser.webEngine.waitUntil(
			async () => readPrefs()['versions.maxPerDoc'] === '7',
			{
				timeout: 10000,
				timeoutMsg: 'the changed limit did not reach coda-prefs.json',
			},
		);
		// Unchanged fields are not written back.
		expect(readPrefs()['versions.maxFileSizeMB']).toBeUndefined();
		expect(readPrefs()['versions.mergeWindowSecs']).toBe('0');

		// A new dialog asks the running store again.
		await openOptions();
		expect((await readLimits()).maxPerDoc).toBe('7');
		await closeOptions('iframe-settings-cancel');
	});

	it('asks before a storage limit below the storage in use', async () => {
		// The store of the test is small, so the dialog hears of a large one.
		await browser.webEngine.execute(() => {
			const call = (window as any).postMobileCall;
			(window as any)._realPostMobileCall = call;
			(window as any).postMobileCall = async (message: string) => {
				const reply = await call(message);
				if (message !== 'VERSIONSTORESTATUS') return reply;
				return JSON.stringify({
					...JSON.parse(reply),
					storeSizeBytes: 5 * 1024 * 1024 * 1024,
				});
			};
		});

		const saveWithAnswer = async (answer: boolean, maxPerDoc: string) => {
			await openOptions();
			await browser.webEngine.execute((confirmed: boolean) => {
				window.confirm = (text?: string) => {
					(window.parent as any)._capConfirmText = text;
					return confirmed;
				};
			}, answer);
			await setLimit('maxStoreSizeMB', '200');
			await setLimit('maxPerDoc', maxPerDoc);
			await closeOptions('iframe-settings-save');
			await browser.webEngine.waitUntil(
				async () => readPrefs()['versions.maxPerDoc'] === maxPerDoc,
				{
					timeout: 10000,
					timeoutMsg: 'the save did not reach coda-prefs.json',
				},
			);
		};

		// No: the other limit is saved, the storage limit is not.
		await saveWithAnswer(false, '8');
		expect(readPrefs()['versions.maxStoreSizeMB']).toBeUndefined();
		expect(
			await browser.webEngine.execute(() => (window as any)._capConfirmText),
		).toContain('4.8 GB');

		await saveWithAnswer(true, '9');
		await browser.webEngine.waitUntil(
			async () => readPrefs()['versions.maxStoreSizeMB'] === '200',
			{
				timeout: 10000,
				timeoutMsg: 'the confirmed storage limit did not reach coda-prefs.json',
			},
		);

		await browser.webEngine.execute(() => {
			(window as any).postMobileCall = (window as any)._realPostMobileCall;
		});
	});

	it('keeps the storage limit at or above the file size limit', async () => {
		await openOptions();
		await setLimit('maxStoreSizeMB', '0');
		await closeOptions('iframe-settings-save');
		await browser.webEngine.waitUntil(
			async () => readPrefs()['versions.maxStoreSizeMB'] === '128',
			{
				timeout: 10000,
				timeoutMsg: 'the storage limit was not raised to the file size limit',
			},
		);

		// A file size limit above the storage limit takes it along.
		await openOptions();
		await setLimit('maxFileSizeMB', '300');
		await closeOptions('iframe-settings-save');
		await browser.webEngine.waitUntil(
			async () => readPrefs()['versions.maxStoreSizeMB'] === '300',
			{
				timeout: 10000,
				timeoutMsg: 'the storage limit did not follow the file size limit',
			},
		);
		expect(readPrefs()['versions.maxFileSizeMB']).toBe('300');
	});

	it('deletes all versions', async () => {
		// Existing documents open read-only; edits need edit mode.
		await browser.webEngine.execute(() => {
			if (!app.map.isEditMode()) app.map._enterEditMode('edit');
		});
		await draft.modifyDoc(browser.webEngine);
		await browser.webEngine.execute(() => {
			app.socket.sendMessage('save dontTerminateEdit=1 dontSaveIfUnmodified=0');
		});
		await browser.webEngine.waitUntil(
			async () => versions.allVersionFiles().length > 0,
			{ timeout: 20000, timeoutMsg: 'no version appeared after saving' },
		);

		await openOptions();
		await browser.webEngine.execute(() => {
			window.confirm = () => true;
			(
				document.getElementById('version-store-delete-all') as HTMLElement
			).click();
		});
		await browser.webEngine.waitUntil(
			async () =>
				await browser.webEngine.execute(() =>
					document
						.getElementById('version-store-usage')!
						.textContent!.startsWith('0 B'),
				),
			{
				timeout: 10000,
				timeoutMsg: 'the dialog did not show an empty store',
			},
		);
		expect(versions.allVersionFiles().length).toBe(0);
		await closeOptions('iframe-settings-cancel');
	});
});
