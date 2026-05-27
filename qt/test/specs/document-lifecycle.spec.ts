/*
 * Copyright the Collabora Online contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

import * as webview from '../lib/webview.js';

describe('Document lifecycle', () => {
	it('should open a blank document from backstage', async function () {
		await browser.webEngine.waitForCondition(
			() =>
				document.readyState === 'complete' &&
				document.querySelectorAll('.backstage-template-card').length > 0,
			{
				timeout: 30000,
				interval: 200,
				timeoutMsg: 'No template cards found in backstage',
			},
		);

		// Snapshot WebView handles before triggering the click; the new
		// document WebView is identified by switchToNewWebView() as the
		// handle that wasn't in this set.
		const beforeHandles = await browser.webEngine.getWindowHandles();

		// Click the first template card (Blank Document). Use setTimeout so execute() returns
		// before coda-qt tears down the backstage WebView and creates a new
		// document WebView - otherwise webenginedriver never receives the
		// response because its target disappears mid-command.
		await browser.webEngine.execute(() => {
			setTimeout(() => {
				const card = document.querySelector('.backstage-template-card') as HTMLElement | null;
				if (card) card.click();
			}, 100);
		});

		// Wait for a new WebView to replace the backstage, then switch to it.
		await webview.switchToNewWebView(browser.webEngine, beforeHandles);

		// Wait for the document editor to finish loading
		await browser.webEngine.waitForCondition(
			() =>
				document.querySelector('.leaflet-cursor.blinking-cursor') !== null,
			{
				timeout: 45000,
				timeoutMsg:
					'Document editor did not load (no blinking cursor appeared)',
			},
		);

		const state = await browser.webEngine.execute(() => ({
			docLoaded: app.map._docLoaded,
			docType: app.map.getDocType(),
		}));

		expect(state.docLoaded).toBe(true);
		expect(state.docType).toBe('text');
	});
});
