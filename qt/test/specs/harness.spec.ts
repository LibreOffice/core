/*
 * Copyright the Collabora Online contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

describe('Test harness', () => {
	describe('webEngine driver', () => {
		it('should connect to coda-qt via WebDriver', async function () {
			await browser.webEngine.waitForCondition(
				() => document.readyState === 'complete',
				{ timeout: 10000, timeoutMsg: 'Page did not reach ready state' },
			);
		});

		it('should report a known engine user agent', async function () {
			const userAgent = await browser.webEngine.execute(
				() => navigator.userAgent,
			);
			const known = userAgent.includes('QtWebEngine') || userAgent.includes('AppleWebKit');
			expect(known).toBe(true);
		});

		it('should report non-zero page dimensions', async function () {
			const dims = await browser.webEngine.execute(() => ({
				width: window.innerWidth,
				height: window.innerHeight,
			}));
			expect(dims.width).toBeGreaterThan(0);
			expect(dims.height).toBeGreaterThan(0);
		});
	});

	describe('native (AT-SPI) driver', () => {
		it('should connect and retrieve the accessibility tree', async function () {
			const source = await browser.native.getPageSource();
			expect(source).toBeTruthy();
		});
	});
});
