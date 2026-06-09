/*
 * Copyright the Collabora Online contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

// The Qt tab strip shell (qtapp-tabstrip.html) registers as its own
// DevTools target alongside document pages, so anything that picks a
// document webview out of the handle list must exclude it.
const TAB_STRIP_PAGE = 'qtapp-tabstrip';

/**
 * Whether the currently focused page is a document, not the tab strip
 * shell.  Reads the URL via execute() because the macOS in-app
 * WebDriverServer does not implement WebDriver's GET /url.
 */
export async function isDocumentPage(
	webEngine: WebdriverIO.Browser,
): Promise<boolean> {
	const url = await webEngine.execute(() => window.location.href);
	return !url.includes(TAB_STRIP_PAGE);
}

/**
 * Wait for a new document WebView to appear (relative to the snapshot
 * in `beforeHandles`) and switch the WebDriver session to it.
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
 * The action can create more than one new page: on Qt, a fresh
 * TabbedWindow also creates its tab strip page, which is a DevTools
 * target like any document page.  Candidates are therefore probed
 * with isDocumentPage() and the tab strip skipped.
 */
export async function switchToNewWebView(
	webEngine: WebdriverIO.Browser,
	beforeHandles: string[],
	timeoutMs = 30000,
	intervalMs = 300,
): Promise<string> {
	const before = new Set(beforeHandles);
	let newHandle: string | null = null;

	await webEngine.waitUntil(
		async () => {
			const handles = await webEngine.getWindowHandles();
			for (let i = handles.length - 1; i >= 0; i--) {
				if (before.has(handles[i])) continue;
				try {
					await webEngine.switchToWindow(handles[i]);
					if (await isDocumentPage(webEngine)) {
						newHandle = handles[i];
						return true;
					}
				} catch {
					// Page closed between enumeration and the switch.
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

	return newHandle!;
}
