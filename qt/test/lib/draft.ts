/*
 * Copyright the Collabora Online contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

import * as webview from './webview.js';

export function draftsDir(): string {
	return process.env.CODA_TEST_DRAFTS_DIR!;
}

export async function createDraft(
	webEngine: WebdriverIO.Browser,
	basename: string,
): Promise<string> {
	const beforeHandles = await webEngine.getWindowHandles();
	// setTimeout so execute() returns before the new WebView opens.
	await webEngine.execute((name: string) => {
		setTimeout(() => {
			window.postMobileMessage(
				'newdoc type=writer basename=' + encodeURIComponent(name),
			);
		}, 100);
	}, basename);
	const handle = await webview.switchToNewWebView(webEngine, beforeHandles);
	await (webEngine as any).waitForCondition(
		() =>
			typeof app !== 'undefined' && app.map && app.map._docLoaded === true,
		{ timeout: 45000, timeoutMsg: 'draft document did not load' },
	);
	return handle;
}

/**
 * Waiting for the page matters: the page decides whether the close
 * prompt appears.
 */
export async function modifyDoc(webEngine: WebdriverIO.Browser): Promise<void> {
	await webEngine.execute(() => {
		app.map.sendUnoCommand('.uno:InsertPagebreak');
	});
	await (webEngine as any).waitForCondition(
		() => app.map._everModified === true,
		{ timeout: 10000, timeoutMsg: 'document did not report a modification' },
	);
}

/** Same close path as the tab close button. */
export async function requestClose(
	webEngine: WebdriverIO.Browser,
): Promise<void> {
	// setTimeout so execute() returns before a modal prompt blocks.
	await webEngine.execute(() => {
		setTimeout(() => {
			window.postMobileMessage('uno .uno:CloseWin');
		}, 100);
	});
}

export type ClosePromptAnswer = 'save' | 'discard' | 'cancel';

/** Wait for the page's close prompt in the current page. */
export async function waitForClosePrompt(
	webEngine: WebdriverIO.Browser,
): Promise<void> {
	await (webEngine as any).waitForCondition(
		() => !!document.getElementById('draft-close-save-button'),
		{ timeout: 15000, timeoutMsg: 'the close prompt did not show' },
	);
}

/** Answer the page's close prompt in the current page. */
export async function answerClosePrompt(
	webEngine: WebdriverIO.Browser,
	answer: ClosePromptAnswer,
): Promise<void> {
	await waitForClosePrompt(webEngine);
	// setTimeout so execute() returns before the answer acts on the page.
	await webEngine.execute((id: string) => {
		const button = document.getElementById(id) as HTMLElement;
		setTimeout(() => button.click(), 100);
	}, `draft-close-${answer}-button`);
}

export async function findTabStripHandle(
	webEngine: WebdriverIO.Browser,
): Promise<string> {
	for (const h of await webEngine.getWindowHandles()) {
		try {
			await webEngine.switchToWindow(h);
			if (!(await webview.isDocumentPage(webEngine))) return h;
		} catch {
			// Page closed between enumeration and the switch.
		}
	}
	throw new Error('tab strip page not found');
}

export async function waitGone(
	webEngine: WebdriverIO.Browser,
	handle: string,
): Promise<void> {
	await webEngine.waitUntil(
		async () => !(await webEngine.getWindowHandles()).includes(handle),
		{ timeout: 30000, timeoutMsg: 'WebView did not close' },
	);
}

/** A tab that went away can leave the session on a closed window. */
export async function switchToAnyDocumentPage(
	webEngine: WebdriverIO.Browser,
): Promise<void> {
	for (const h of await webEngine.getWindowHandles()) {
		try {
			await webEngine.switchToWindow(h);
			if (await webview.isDocumentPage(webEngine)) return;
		} catch {
			// Page closed between enumeration and the switch.
		}
	}
}
