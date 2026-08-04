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

// Two documents share one kit. Closing one of them must leave the other still
// talking to the engine. This targets the coda-m close-notification wiring,
// where a single shared notification pipe once made closing any document tear
// down every other open document's engine-message pump, so the other
// documents could still send to the engine but never hear anything back.
//
// app.file.modified is set from the engine's .uno:ModifiedStatus notification,
// which travels the engine-to-client receive path. So a surviving document
// that still turns "modified" after an edit proves its receive path is intact;
// if the receive path were gone, the edit would reach the engine but the
// modified notification would never come back.

const we = (): WebdriverIO.Browser & {
	waitForCondition(
		predicate: () => boolean,
		opts?: { timeout?: number; interval?: number; timeoutMsg?: string },
	): Promise<boolean>;
} => browser.webEngine;

async function switchTo(handle: string): Promise<void> {
	await we().switchToWindow(handle);
}

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

describe('Closing one document keeps another responsive', () => {
	before(function () {
		// The shared-pipe defect is coda-m specific, and closing a document by
		// posting BYE is the coda-m close path, so run this only there.
		if (process.env.CODA_PLATFORM !== 'macos') {
			this.skip();
		}
	});

	it('still marks the surviving document modified after an edit', async function () {
		const handleToClose = await openFixture(
			browser.webEngine,
			browser.native,
			'simple.odt',
		);
		const handleToKeep = await openFixture(
			browser.webEngine,
			browser.native,
			'scrolling.odt',
		);

		// Put the surviving document into edit mode while both are still open,
		// so the edit later only has to exercise the receive path, not also a
		// permission change.
		await switchTo(handleToKeep);
		await enterEditMode();
		expect(await we().execute(() => app.file.modified)).toBe(false);

		// Close the other document. BYE makes the app close that document's
		// window, which is what releases its engine connection. The call goes in
		// a statement of its own: on coda-m postMobileMessage answers with a
		// promise, and returning one from an executed script is not a result the
		// driver can hand back.
		await switchTo(handleToClose);
		await we().execute(() => {
			window.postMobileMessage('BYE');
		});

		// Wait for the close to actually take effect before testing the survivor,
		// so the survivor's receive path has already had its chance to break. The
		// app stops offering a document's web view as the document closes, so the
		// handle going away is the signal that the close happened.
		try {
			await we().waitUntil(
				async () => !(await we().getWindowHandles()).includes(handleToClose),
				{ timeout: 15000, interval: 250 },
			);
		} catch {
			const handles = await we().getWindowHandles();
			throw new Error(
				`the closed document is still on offer; asked to close ` +
					`${handleToClose}, still listed: ${handles.join(', ')}`,
			);
		}

		// Editing the survivor must still round-trip through the engine and come
		// back as a modified notification.
		await switchTo(handleToKeep);
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
					'the surviving document stopped receiving engine updates after the other document closed',
			},
		);
	});
});
