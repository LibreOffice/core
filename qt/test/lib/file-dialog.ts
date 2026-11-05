/*
 * Copyright the Collabora Online contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

import { join } from 'path';
import * as webview from './webview.js';

/**
 * Open a file from the test fixtures directory via the native file dialog.
 *
 * Triggers .uno:Open, locates the dialog with platform-appropriate
 * selectors, fills the path, clicks Open, switches to the new
 * WebView, and waits for the document to finish loading.
 *
 * On Qt the selectors target QFileDialog's accessibility IDs.  On
 * macOS they target NSOpenPanel by Obj-C class plus an "Open" button
 * by title.  On Windows they target the control IDs of the common
 * file dialog: 1148 is the file name box and 1 the Open button.  The
 * post-conditions (switch to new webview + doc loaded) are the same
 * on all platforms.
 */
const FILE_DIALOG_SELECTORS: { [platform: string]: { file: string; open: string } } = {
	qt: {
		file: '//*[@accessibility-id="QApplication.QFileDialog.fileNameEdit"]',
		open: '//*[@accessibility-id="QApplication.QFileDialog.buttonBox.QPushButton"]',
	},
	macos: {
		file: '//*[@accessibility-id="open-panel"]',
		open: '//*[@accessibility-id="OKButton"]',
	},
	windows: {
		file: '//*[@accessibility-id="1148"]',
		open: '//*[@accessibility-id="1"]',
	},
};

export async function openFixture(
	webEngine: WebdriverIO.Browser,
	native: WebdriverIO.Browser,
	fileName: string,
): Promise<string> {
	const { file: fileSel, open: openSel } =
		FILE_DIALOG_SELECTORS[process.env.CODA_PLATFORM || 'qt'];

	// Snapshot the current WebView handles BEFORE dispatching .uno:Open.
	// switchToNewWebView() below uses this snapshot to identify the
	// WebView created by this action; the native side may register it
	// at any point between now and the document load completing, so the
	// snapshot has to be taken up front.
	const beforeHandles = await webEngine.getWindowHandles();

	// Use setTimeout so execute() returns before the modal dialog blocks.
	await webEngine.execute(() => {
		setTimeout(() => {
			window.postMobileMessage('uno .uno:Open');
		}, 100);
	});

	const filePath = join(process.env.CODA_TEST_DOCUMENTS_DIR!, fileName);

	const fileTarget = await native.$(fileSel);
	await fileTarget.waitForExist({ timeout: 10000 });
	await fileTarget.setValue(filePath);

	const openBtn = await native.$(openSel);
	await openBtn.waitForExist({ timeout: 5000 });
	await openBtn.click();

	const newHandle = await webview.switchToNewWebView(webEngine, beforeHandles);

	await (webEngine as any).waitForCondition(
		() =>
			typeof app !== 'undefined' &&
			app.map &&
			app.map._docLoaded === true,
		{
			timeout: 45000,
			timeoutMsg: `Document did not load after opening ${fileName}`,
		},
	);

	return newHandle;
}
