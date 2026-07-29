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
import { openFixture } from '../lib/file-dialog.js';

let documentHandle: string;

// Documents opened through the native file dialog start in read-only view,
// so editing commands such as .uno:HyperlinkDialog need edit mode entered
// first.
async function enterEditMode() {
	await browser.webEngine.execute(() => {
		if (!app.map.isEditMode()) app.map._enterEditMode('edit');
	});
	await browser.webEngine.waitForCondition(() => app.map.isEditMode(), {
		timeout: 15000,
		interval: 200,
		timeoutMsg: 'document did not enter edit mode',
	});
}

// A dialog's registry entry outlives its native window: a deferred task
// removes it after the close round-trip. While any entry remains,
// dialog-opening commands are dropped, so wait for the drain.
async function closeOpenDialogs() {
	await browser.webEngine.switchToWindow(documentHandle);
	await browser.webEngine.execute(() => {
		const jsdialog = app.map.jsdialog as any;
		jsdialog.closeAllDropdowns();
		// A closed dropdown's registry entry is removed by a deferred task, so
		// the keys read here can still name one; closing an entry that is
		// already gone does nothing.
		Object.keys(jsdialog.dialogs)
			.filter((key) => key !== 'snackbar')
			.forEach((key) => jsdialog.close(key, true));
	});
	await browser.webEngine.waitForCondition(
		() =>
			Object.keys((app.map.jsdialog as any).dialogs).filter(
				(key) => key !== 'snackbar',
			).length === 0,
		{ timeout: 10000, timeoutMsg: 'dialog registry did not drain' },
	);
}

describe('Popped-out jsdialogs', () => {
	before(async function () {
		this.timeout(60000);

		// Opening a file that is already open activates the existing
		// window instead of creating a new one, so this spec uses its own
		// fixture to guarantee a fresh WebView.
		documentHandle = await openFixture(
			browser.webEngine,
			browser.native,
			'popout-dialog.odt',
		);
		await enterEditMode();
	});

	// A dialog command issued while a dialog is already open opens no window, so
	// a dialog left behind by one test fails every test after it. Clear them
	// whatever the outcome of the test that opened them.
	afterEach(async function () {
		try {
			await closeOpenDialogs();
		} catch {
			// The failure the test itself reported is the one worth reading.
		}
	});

	it('opens a dialog in its own window', async function () {
		const beforeHandles = await browser.webEngine.getWindowHandles();

		await browser.webEngine.execute(() => {
			app.map.sendUnoCommand('.uno:HyperlinkDialog');
		});

		await webview.switchToNewWebView(browser.webEngine, beforeHandles);

		await browser.webEngine.waitForCondition(
			() => document.querySelector('.jsdialog-container') !== null,
			{ timeout: 10000, timeoutMsg: 'No dialog container in child window' },
		);

		// The main document window holds no dialog DOM.
		await browser.webEngine.switchToWindow(documentHandle);
		const inPage = await browser.webEngine.execute(
			() => document.querySelectorAll('.jsdialog-container').length,
		);
		expect(inPage).toBe(0);
	});

	it('closes the child window when the dialog is closed', async function () {
		const handlesBefore = await browser.webEngine.getWindowHandles();

		await browser.webEngine.switchToWindow(documentHandle);
		await browser.webEngine.execute(() => {
			app.map.sendUnoCommand('.uno:HyperlinkDialog');
		});
		await webview.switchToNewWebView(browser.webEngine, handlesBefore);

		await browser.webEngine.switchToWindow(documentHandle);
		await browser.webEngine.execute(() => {
			// Close the open dialog the same way the Escape key does.
			const jsdialog = app.map.jsdialog;
			const keys = Object.keys(jsdialog.dialogs);
			jsdialog.close(keys[keys.length - 1], true);
		});

		// waitForCondition runs its predicate inside the page, so it cannot
		// see the driver-side handle count computed here; poll from outside
		// instead.
		let closed = false;
		for (let i = 0; i < 50; i++) {
			if (
				(await browser.webEngine.getWindowHandles()).length ===
				handlesBefore.length
			) {
				closed = true;
				break;
			}
			await new Promise((resolve) => setTimeout(resolve, 200));
		}
		expect(closed).toBe(true);
	});

	it('shows the dialog as a native dialog window with a title', async function () {
		const beforeHandles = await browser.webEngine.getWindowHandles();

		await browser.webEngine.switchToWindow(documentHandle);
		await browser.webEngine.execute(() => {
			app.map.sendUnoCommand('.uno:HyperlinkDialog');
		});
		await webview.switchToNewWebView(browser.webEngine, beforeHandles);
		await browser.webEngine.waitForCondition(
			() => document.querySelector('.jsdialog-container') !== null,
			{ timeout: 10000, timeoutMsg: 'No dialog container in child window' },
		);

		// The window is sized to the dialog, not to the 800x600 document
		// window minimum.
		const dims = await browser.webEngine.execute(() => ({
			width: window.innerWidth,
			height: window.innerHeight,
			formWidth: (
				document.querySelector('.jsdialog-container') as HTMLElement
			).getBoundingClientRect().width,
		}));
		expect(Math.abs(dims.width - dims.formWidth)).toBeLessThan(50);

		// The native tree shows a window whose title is the dialog title.
		const source = await browser.native.getPageSource();
		expect(source).toContain('Hyperlink');
	});

	it('draws the dialog with the styles the page uses', async function () {
		const beforeHandles = await browser.webEngine.getWindowHandles();

		await browser.webEngine.switchToWindow(documentHandle);
		await browser.webEngine.execute(() => {
			app.map.sendUnoCommand('.uno:HyperlinkDialog');
		});
		await webview.switchToNewWebView(browser.webEngine, beforeHandles);
		await browser.webEngine.waitForCondition(
			() => document.querySelector('.jsdialog-container') !== null,
			{ timeout: 10000, timeoutMsg: 'No dialog container in child window' },
		);

		// An unstyled dialog falls back to the browser's own serif font and
		// leaves its buttons at the platform default look, so the font the
		// application asks for tells us the styles arrived.
		const fonts = await browser.webEngine.execute(() => {
			const content = document.querySelector(
				'.jsdialog-container .ui-dialog-content',
			) as HTMLElement;
			return {
				content: getComputedStyle(content).fontFamily,
				expected: getComputedStyle(document.documentElement).getPropertyValue(
					'--cool-font',
				),
			};
		});
		// The two lists name the same fonts but spell the list differently: a
		// computed list puts one space after each comma and quotes the names it
		// has to quote, while the stylesheet the release build ships has been
		// through a minifier that takes those spaces out. Compare the names.
		const fontNames = (fontList: string) =>
			fontList.split(',').map((name) => name.trim().replace(/['"]/g, ''));
		expect(fonts.expected).not.toBe('');
		expect(fontNames(fonts.content)).toEqual(fontNames(fonts.expected));
	});

	it('leaves room in the dialog window for the whole dialog', async function () {
		const beforeHandles = await browser.webEngine.getWindowHandles();

		await browser.webEngine.switchToWindow(documentHandle);
		await browser.webEngine.execute(() => {
			app.map.sendUnoCommand('.uno:HyperlinkDialog');
		});
		await webview.switchToNewWebView(browser.webEngine, beforeHandles);
		await browser.webEngine.waitForCondition(
			() => document.querySelector('.jsdialog-container') !== null,
			{ timeout: 10000, timeoutMsg: 'No dialog container in child window' },
		);

		const fit = await browser.webEngine.execute(() => {
			const content = document.querySelector(
				'.jsdialog-container .ui-dialog-content',
			) as HTMLElement;
			return {
				windowWidth: window.innerWidth,
				windowHeight: window.innerHeight,
				width: content.clientWidth,
				height: content.clientHeight,
				wantedWidth: content.scrollWidth,
				wantedHeight: content.scrollHeight,
			};
		});

		// Every part of the dialog is on screen: nothing is cut off and no
		// scrollbar stands in for the missing room.
		expect(fit.wantedWidth).toBeLessThanOrEqual(fit.width + 1);
		expect(fit.wantedHeight).toBeLessThanOrEqual(fit.height + 1);
		expect(fit.windowWidth).toBeGreaterThanOrEqual(fit.width);
		expect(fit.windowHeight).toBeGreaterThanOrEqual(fit.height);
		// A dialog squeezed into a window that was never given a size comes
		// out a hundred pixels tall, far short of this dialog's own height.
		expect(fit.width).toBeGreaterThan(300);
		expect(fit.height).toBeGreaterThan(200);
	});

	it('opens a dropdown from a popped-out Find and Replace dialog inside the dialog window', async function () {
		const beforeHandles = await browser.webEngine.getWindowHandles();

		await browser.webEngine.switchToWindow(documentHandle);
		await browser.webEngine.execute(() => {
			app.map.sendUnoCommand('.uno:SearchDialog');
		});
		await webview.switchToNewWebView(browser.webEngine, beforeHandles);
		await browser.webEngine.waitForCondition(
			() => document.querySelector('.jsdialog-container') !== null,
			{ timeout: 10000, timeoutMsg: 'No dialog container in child window' },
		);

		await browser.webEngine.execute(() => {
			const arrow = document.querySelector(
				'.jsdialog-container .ui-combobox-button',
			) as HTMLElement | null;
			if (arrow) arrow.click();
		});

		await browser.webEngine.waitForCondition(
			() => document.querySelector('.modalpopup') !== null,
			{ timeout: 10000, timeoutMsg: 'No dropdown in dialog window' },
		);

		// The dropdown is not in the main document.
		await browser.webEngine.switchToWindow(documentHandle);
		const inPage = await browser.webEngine.execute(
			() => document.querySelectorAll('.modalpopup').length,
		);
		expect(inPage).toBe(0);
	});

	it('closes only the dropdown, not the dialog, when Escape is pressed inside it', async function () {
		const beforeHandles = await browser.webEngine.getWindowHandles();

		await browser.webEngine.switchToWindow(documentHandle);
		await browser.webEngine.execute(() => {
			app.map.sendUnoCommand('.uno:SearchDialog');
		});
		await webview.switchToNewWebView(browser.webEngine, beforeHandles);
		await browser.webEngine.waitForCondition(
			() => document.querySelector('.jsdialog-container') !== null,
			{ timeout: 10000, timeoutMsg: 'No dialog container in child window' },
		);

		// Search once so the term lands in the combobox history: the dropdown
		// only carries its filter field when it has entries to list, and the
		// dialog remembers the term whether or not the document contains it.
		await browser.webEngine.execute(() => {
			const input = document.querySelector(
				'.jsdialog-container #searchterm .ui-combobox-content',
			) as HTMLInputElement | null;
			if (input) {
				input.value = 'hello';
				input.dispatchEvent(
					new KeyboardEvent('keyup', { key: 'Enter', bubbles: true }),
				);
			}
		});
		// Find Next enables once the dialog hears about the non-empty term;
		// a click on a still-disabled button would do nothing.
		await browser.webEngine.waitForCondition(
			() => {
				const findNext = document.querySelector(
					'.jsdialog-container #search-button',
				) as HTMLButtonElement | null;
				return findNext !== null && !findNext.disabled;
			},
			{ timeout: 10000, timeoutMsg: 'Find Next stayed disabled' },
		);
		await browser.webEngine.execute(() => {
			const findNext = document.querySelector(
				'.jsdialog-container #search-button',
			) as HTMLElement | null;
			if (findNext) findNext.click();
		});

		// The history entry arrives as a combobox update a moment after the
		// search runs, so reopen the dropdown until it shows up with its
		// filter field.
		let filterInputFound = false;
		for (let i = 0; i < 20 && !filterInputFound; i++) {
			await browser.webEngine.execute(() => {
				const arrow = document.querySelector(
					'.jsdialog-container #searchterm .ui-combobox-button',
				) as HTMLElement | null;
				if (arrow && document.querySelector('.modalpopup') === null)
					arrow.click();
			});
			try {
				await browser.webEngine.waitForCondition(
					() => document.querySelector('.ui-combobox-search-input') !== null,
					{ timeout: 1000 },
				);
				filterInputFound = true;
			} catch {
				// An empty dropdown means the update has not arrived yet; close
				// it and try again.
				await browser.webEngine.execute(() => {
					const arrow = document.querySelector(
						'.jsdialog-container #searchterm .ui-combobox-button',
					) as HTMLElement | null;
					if (arrow) arrow.click();
				});
				await new Promise((resolve) => setTimeout(resolve, 300));
			}
		}
		expect(filterInputFound).toBe(true);

		// The dropdown's own keydown handler sits on the filter field, so the
		// event is dispatched there, the same path a real keypress on the
		// focused field takes: the handler calls preventDefault before the
		// event reaches the dialog's document-level Escape listener.
		await browser.webEngine.execute(() => {
			const searchInput = document.querySelector(
				'.ui-combobox-search-input',
			) as HTMLElement | null;
			if (searchInput) {
				searchInput.dispatchEvent(
					new KeyboardEvent('keydown', {
						key: 'Escape',
						bubbles: true,
						cancelable: true,
					}),
				);
			}
		});

		// The dropdown closed...
		await browser.webEngine.waitForCondition(
			() => document.querySelector('.modalpopup') === null,
			{ timeout: 10000, timeoutMsg: 'Dropdown did not close on Escape' },
		);
		// ...and the dialog window's DOM is still there.
		const dialogStillOpen = await browser.webEngine.execute(
			() => document.querySelector('.jsdialog-container') !== null,
		);
		expect(dialogStillOpen).toBe(true);

		// The dialog's own registry entry survived too.
		await browser.webEngine.switchToWindow(documentHandle);
		const dialogKeys = await browser.webEngine.execute(() =>
			Object.keys((app.map.jsdialog as any).dialogs).filter(
				(key) =>
					key !== 'snackbar' &&
					(app.map.jsdialog as any).dialogs[key].isDropdown !== true,
			),
		);
		expect(dialogKeys.length).toBe(1);
	});

	it('tells the server when the user closes the dialog window', async function () {
		const beforeHandles = await browser.webEngine.getWindowHandles();

		await browser.webEngine.switchToWindow(documentHandle);
		await browser.webEngine.execute(() => {
			app.map.sendUnoCommand('.uno:HyperlinkDialog');
		});
		await webview.switchToNewWebView(browser.webEngine, beforeHandles);
		await browser.webEngine.waitForCondition(
			() => document.querySelector('.jsdialog-container') !== null,
			{ timeout: 10000, timeoutMsg: 'No dialog container in child window' },
		);

		// Closing the child window over the dialog's own head takes the same
		// teardown path as the native title-bar close button: the page goes
		// away first and the opener reacts to pagehide. The close call runs
		// in the document window, so the driver is never attached to the
		// window being destroyed (attaching there can crash webenginedriver).
		await browser.webEngine.switchToWindow(documentHandle);
		await browser.webEngine.execute(() => {
			(window as any).JSDialog.ForEachPopoutDocument((doc: Document) => {
				if (doc.defaultView) doc.defaultView.close();
			});
		});

		// The dialog registry empties: the opener noticed and closed the
		// dialog server-side.
		await browser.webEngine.waitForCondition(
			() =>
				Object.keys(app.map.jsdialog.dialogs).filter(
					(key) => key !== 'snackbar',
				).length === 0,
			{ timeout: 10000, timeoutMsg: 'Dialog registry not cleaned up' },
		);

		// The same dialog opens again cleanly, so the server-side dialog
		// was really torn down rather than left in a half-closed state.
		const handlesNow = await browser.webEngine.getWindowHandles();
		await browser.webEngine.execute(() => {
			app.map.sendUnoCommand('.uno:HyperlinkDialog');
		});
		await webview.switchToNewWebView(browser.webEngine, handlesNow);
		await browser.webEngine.waitForCondition(
			() => document.querySelector('.jsdialog-container') !== null,
			{ timeout: 10000, timeoutMsg: 'Dialog did not reopen' },
		);
	});

	it('pops out a modal message box and its button closes it', async function () {
		const beforeHandles = await browser.webEngine.getWindowHandles();

		await browser.webEngine.switchToWindow(documentHandle);
		await browser.webEngine.execute(() => {
			// A page-generated modal message box takes the same popout path
			// as a server-driven dialog: showInfoModal builds a modalpopup,
			// and JSDialog.canPopout admits modal popups as well as message
			// boxes.
			(app.map as any).uiManager.showInfoModal(
				'coda-popout-test',
				'Popout Test',
				'A modal message box in its own window.',
				'',
				'OK',
			);
		});

		await webview.switchToNewWebView(browser.webEngine, beforeHandles);
		await browser.webEngine.waitForCondition(
			() => document.querySelector('.jsdialog-container') !== null,
			{ timeout: 10000, timeoutMsg: 'No modal container in child window' },
		);

		// The response button's id is the button id plus a '-button' suffix
		// that the pushbutton widget appends to its wrapper element. The click
		// runs from the document window, so the driver is never attached to the
		// window the click is about to destroy.
		await browser.webEngine.switchToWindow(documentHandle);
		await browser.webEngine.execute(() => {
			(window as any).JSDialog.ForEachPopoutDocument((doc: Document) => {
				const button = doc.querySelector(
					'.jsdialog-container #coda-popout-test-response-button',
				) as HTMLElement | null;
				if (button) button.click();
			});
		});

		await browser.webEngine.waitForCondition(
			() =>
				Object.keys(app.map.jsdialog.dialogs).filter(
					(key) => key !== 'snackbar',
				).length === 0,
			{ timeout: 10000, timeoutMsg: 'Modal did not close' },
		);
	});

	it('closes dialog windows when all dialogs are closed', async function () {
		const beforeHandles = await browser.webEngine.getWindowHandles();

		await browser.webEngine.switchToWindow(documentHandle);
		await browser.webEngine.execute(() => {
			app.map.sendUnoCommand('.uno:HyperlinkDialog');
		});
		await webview.switchToNewWebView(browser.webEngine, beforeHandles);
		await browser.webEngine.waitForCondition(
			() => document.querySelector('.jsdialog-container') !== null,
			{ timeout: 10000, timeoutMsg: 'No dialog container in child window' },
		);

		await browser.webEngine.switchToWindow(documentHandle);
		await browser.webEngine.execute(() => {
			(app.map as any).fire('closealldialogs');
		});

		await browser.webEngine.waitForCondition(
			() =>
				Object.keys(app.map.jsdialog.dialogs).filter(
					(key) => key !== 'snackbar',
				).length === 0,
			{ timeout: 10000, timeoutMsg: 'closealldialogs left dialogs behind' },
		);

		// waitForCondition runs its predicate inside the page, so it cannot
		// see the driver-side handle count computed here; poll from outside
		// instead. The child WebView is gone once this returns.
		for (let i = 0; i < 50; i++) {
			if (
				(await browser.webEngine.getWindowHandles()).length ===
				beforeHandles.length
			)
				break;
			await new Promise((resolve) => setTimeout(resolve, 200));
		}
		expect((await browser.webEngine.getWindowHandles()).length).toBe(
			beforeHandles.length,
		);
	});
});
