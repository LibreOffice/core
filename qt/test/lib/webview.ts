/*
 * Copyright the Collabora Online contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

/**
 * Wait for a new WebView to appear (relative to the snapshot in
 * `beforeHandles`) and switch the WebDriver session to it.
 *
 * The native side registers WebViews asynchronously and may also
 * auto-activate the new one when register() runs.  That means we
 * cannot reliably remember "which handles we have already seen"
 * across switchToNewWebView() calls: a new WebView can register
 * before, during, or after the polling loop, and if it lands after
 * the loop has returned we would silently leave it out of our
 * remembered set and then mistake it for "new" on the next call.
 *
 * Instead, every caller takes a snapshot of the handle set right
 * before it triggers the action that creates the new WebView and
 * passes it in here.  The diff is then unambiguous.
 *
 * If the action creates more than one WebView (it normally creates
 * exactly one) we pick the most recently registered, which is at
 * the end of getWindowHandles() because the native manager preserves
 * insertion order.
 */
export async function switchToNewWebView(
	webEngine: WebdriverIO.Browser,
	beforeHandles: string[],
	timeoutMs = 30000,
	intervalMs = 300,
): Promise<void> {
	const before = new Set(beforeHandles);
	let newHandle: string | null = null;

	await webEngine.waitUntil(
		async () => {
			const handles = await webEngine.getWindowHandles();
			for (let i = handles.length - 1; i >= 0; i--) {
				if (!before.has(handles[i])) {
					newHandle = handles[i];
					return true;
				}
			}
			return false;
		},
		{
			timeout: timeoutMs,
			interval: intervalMs,
			timeoutMsg: `New WebView did not appear within ${timeoutMs}ms`,
		},
	);

	await webEngine.switchToWindow(newHandle!);
}
