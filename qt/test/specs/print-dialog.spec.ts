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

// Printing exports the document to a PDF and then shows the print dialog. The
// export runs on a worker thread, so the web view keeps answering and the
// progress snackbar animates while it runs, and the native dialog appears when
// the export finishes. Before that change the export ran on the GUI thread and
// froze the window for its whole duration.
//
// The fixture is tiny, so the export is quick; the snackbar may open and close
// too fast to observe, so its appearance is checked best effort while the
// close (the completion callback ran) and the web view staying responsive are
// the firm assertions.
describe('Print', () => {
	const we = (): WebdriverIO.Browser & {
		waitForCondition(
			predicate: () => boolean,
			opts?: { timeout?: number; interval?: number; timeoutMsg?: string },
		): Promise<boolean>;
	} => browser.webEngine;

	const snackbarPresent = () =>
		!!document.querySelector(
			'#snackbar-container-progress, #snackbar-container, .snackbar',
		);

	// One coda-qt instance serves the whole spec, so the document is opened
	// once here and both tests print the same already-open presentation.
	before(async function () {
		this.timeout(120000);
		await openFixture(browser.webEngine, browser.native, 'print-sample.odp');
	});

	it('keeps the web view responsive and opens the print dialog', async function () {
		this.timeout(90000);

		await we().execute(() => {
			window.postMobileMessage('PRINT');
		});

		// The web view answers a round-trip while the export is pending. A
		// frozen GUI thread would stall message delivery instead.
		expect(await we().execute(() => 1 + 1)).toBe(2);

		// Best effort: catch the progress snackbar if the export is slow
		// enough for it to still be up.
		try {
			await we().waitUntil(async () => we().execute(snackbarPresent), {
				timeout: 3000,
				interval: 100,
			});
		} catch {
			// Export finished before the snackbar was observable.
		}

		// The completion callback closes the snackbar when the export ends.
		await we().waitUntil(async () => !(await we().execute(snackbarPresent)), {
			timeout: 60000,
			interval: 200,
			timeoutMsg: 'print progress snackbar never closed',
		});

		// The print dialog appears once the PDF is ready. The dialog is a
		// native Qt widget reached over AT-SPI; its window title is
		// "Print Document" and it carries a "Cancel" button.
		const dialog = await browser.native.$('//dialog[@name="Print Document"]');
		await dialog.waitForExist({ timeout: 30000 });

		const cancelButton = await browser.native.$(
			'//dialog[@name="Print Document"]//button[@name="Cancel"]',
		);
		await cancelButton.waitForExist({ timeout: 5000 });
		await cancelButton.click();

		await dialog.waitForExist({ timeout: 5000, reverse: true });

		// The document is still usable after the print flow.
		expect(await we().execute(() => app.map._docLoaded)).toBe(true);
	});

	// A second print request while one export is already running must not start
	// a second export or open a second dialog.
	it('ignores a second print while one is in progress', async function () {
		this.timeout(90000);

		await we().execute(() => {
			window.postMobileMessage('PRINT');
			window.postMobileMessage('PRINT');
		});

		const dialog = await browser.native.$('//dialog[@name="Print Document"]');
		await dialog.waitForExist({ timeout: 30000 });

		// A broken guard would run a second export alongside the first and its
		// dialog would appear a moment later, so let the count settle before
		// asserting rather than sampling it the instant the first dialog shows.
		let stableOnePolls = 0;
		await we().waitUntil(
			async () => {
				const dialogs = await browser.native.$$(
					'//dialog[@name="Print Document"]',
				);
				stableOnePolls = dialogs.length === 1 ? stableOnePolls + 1 : -1000;
				return stableOnePolls >= 10;
			},
			{
				timeout: 20000,
				interval: 300,
				timeoutMsg: 'a second print dialog appeared while one was open',
			},
		);

		const cancelButton = await browser.native.$(
			'//dialog[@name="Print Document"]//button[@name="Cancel"]',
		);
		await cancelButton.click();
		await dialog.waitForExist({ timeout: 5000, reverse: true });
	});

	// An export whose handling throws inside the kit is still answered: the
	// error dialog appears, and a later print works because the failed export
	// no longer counts as in progress.
	it('recovers from an export that fails inside the kit', async function () {
		this.timeout(90000);

		await we().execute(() => {
			// "%zz" is not a valid percent-escape, so decoding the requested
			// name throws inside the kit before any file is written.
			window.postMobileMessage(
				'downloadas name=fault%zz.pdf id=export format=pdf options=',
			);
		});

		// The error message box carries the accessibility role "alert", not
		// "dialog" like the print and file dialogs.
		const errorDialog = await browser.native.$('//alert[@name="Export Error"]');
		await errorDialog.waitForExist({ timeout: 30000 });

		const okButton = await browser.native.$(
			'//alert[@name="Export Error"]//button[@name="OK"]',
		);
		await okButton.waitForExist({ timeout: 5000 });
		await okButton.click();
		await errorDialog.waitForExist({ timeout: 5000, reverse: true });

		await we().execute(() => {
			window.postMobileMessage('PRINT');
		});

		const printDialog = await browser.native.$(
			'//dialog[@name="Print Document"]',
		);
		await printDialog.waitForExist({ timeout: 30000 });

		const cancelButton = await browser.native.$(
			'//dialog[@name="Print Document"]//button[@name="Cancel"]',
		);
		await cancelButton.click();
		await printDialog.waitForExist({ timeout: 5000, reverse: true });
	});
});
