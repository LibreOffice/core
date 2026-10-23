/*
 * Copyright the Collabora Online contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

// Runs as its own suite (run-tests.sh --suite recovery) with
// CODA_TEST_SEED_DRAFT=1, so the service plants a draft before
// coda-qt starts.

import { existsSync, readFileSync } from 'fs';
import { join } from 'path';

import * as draft from '../lib/draft.js';

describe('Draft recovery', () => {
	let draftHandle: string;

	it('reopens the leftover draft as an editable draft', async () => {
		await browser.webEngine.waitUntil(
			async () => {
				for (const h of await browser.webEngine.getWindowHandles()) {
					try {
						await browser.webEngine.switchToWindow(h);
						const url = await browser.webEngine.execute(
							() => window.location.href,
						);
						if (
							decodeURIComponent(decodeURIComponent(url)).includes(
								'/drafts/Recovered Draft.odt',
							)
						) {
							draftHandle = h;
							return true;
						}
					} catch {
						// Page closed between enumeration and the switch.
					}
				}
				return false;
			},
			{
				timeout: 45000,
				timeoutMsg: 'the seeded draft was not reopened on startup',
			},
		);

		await (browser.webEngine as any).waitForCondition(
			() =>
				typeof app !== 'undefined' && app.map && app.map._docLoaded === true,
			{ timeout: 45000, timeoutMsg: 'recovered draft did not load' },
		);

		// Recovered drafts open editable, not read-only like existing files.
		const url = await browser.webEngine.execute(() => window.location.href);
		expect(url.includes('startreadonly')).toBe(false);

		// Recovery must not touch the recent files list.
		const recents = process.env.CODA_TEST_RECENTS_FILE!;
		if (existsSync(recents)) {
			expect(readFileSync(recents, 'utf8').includes('Recovered')).toBe(false);
		}
	});

	it('prompts before closing the recovered draft even when untouched', async () => {
		// A recovered draft carries content from the previous run, so the
		// close prompt must appear although nothing changed in this session.
		// Answer Cancel: discarding the only tab would close the last window
		// and quit the app (Discard itself is covered by draft-lifecycle).
		await draft.requestClose(browser.webEngine);
		await draft.answerClosePrompt(browser.webEngine, 'cancel');

		await browser.webEngine.switchToWindow(draftHandle);
		const loaded = await browser.webEngine.execute(
			() => app.map._docLoaded === true,
		);
		expect(loaded).toBe(true);
		expect(existsSync(join(draft.draftsDir(), 'Recovered Draft.odt'))).toBe(
			true,
		);
	});
});
