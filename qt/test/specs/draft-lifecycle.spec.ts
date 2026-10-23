/*
 * Copyright the Collabora Online contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

import { existsSync, readdirSync, readFileSync, statSync } from 'fs';
import { join } from 'path';

import * as dialog from '../lib/file-dialog.js';
import * as draft from '../lib/draft.js';
import * as webview from '../lib/webview.js';

function recentsContain(needle: string): boolean {
	const path = process.env.CODA_TEST_RECENTS_FILE!;
	if (!existsSync(path)) return false;
	return readFileSync(path, 'utf8').includes(needle);
}

/** Switch to the tab strip page that shows a tab with this title. */
async function switchToStripWith(title: string): Promise<void> {
	for (const h of await browser.webEngine.getWindowHandles()) {
		try {
			await browser.webEngine.switchToWindow(h);
			if (await webview.isDocumentPage(browser.webEngine)) continue;
			const has = await browser.webEngine.execute(
				(t: string) =>
					Array.from(document.querySelectorAll('.tab .title')).some(
						(e) => e.textContent === t,
					),
				title,
			);
			if (has) return;
		} catch {
			// Page closed between enumeration and the switch.
		}
	}
	throw new Error(`no tab strip shows ${title}`);
}

/** Run a strip action on the tab with this title, the way the mouse would. */
async function onStripTab(
	title: string,
	action: 'activate' | 'close' | 'moveToNewWindow',
): Promise<void> {
	await switchToStripWith(title);
	await browser.webEngine.execute(
		(t: string, a: string) => {
			const tab = Array.from(document.querySelectorAll('.tab')).find(
				(e) => e.querySelector('.title')?.textContent === t,
			) as HTMLElement;
			setTimeout(() => {
				if (a === 'activate') tab.click();
				else if (a === 'close')
					(tab.querySelector('.close') as HTMLElement).click();
				else {
					// A drag that ends outside every strip moves the tab to a new window.
					tab.dispatchEvent(
						new DragEvent('dragstart', { dataTransfer: new DataTransfer() }),
					);
					tab.dispatchEvent(new DragEvent('dragend'));
				}
			}, 100);
		},
		title,
		action,
	);
}

describe('Draft lifecycle', () => {
	// A regular (non-draft) document that keeps the window alive while
	// draft tabs come and go, and makes the tab strip visible.
	let anchorHandle: string;
	let draftHandle: string;
	let firstSaveHandle: string;

	before(async () => {
		anchorHandle = await dialog.openFixture(
			browser.webEngine,
			browser.native,
			'simple.odt',
		);
	});

	after(async () => {
		// Close the anchor again: open-file.spec opens the same fixture and
		// would only get the existing tab activated instead of a new WebView.
		await browser.webEngine.switchToWindow(anchorHandle);
		await draft.requestClose(browser.webEngine);
		await draft.waitGone(browser.webEngine, anchorHandle);
		await draft.switchToAnyDocumentPage(browser.webEngine);
	});

	it('creates a new document as a draft outside Documents', async () => {
		const docsDir = process.env.CODA_TEST_DOCUMENTS_DIR!;
		const docsBefore = readdirSync(docsDir).sort();

		draftHandle = await draft.createDraft(browser.webEngine, 'PromptTest');

		expect(existsSync(join(draft.draftsDir(), 'PromptTest.odt'))).toBe(true);
		expect(readdirSync(docsDir).sort()).toEqual(docsBefore);
		expect(recentsContain('PromptTest')).toBe(false);
	});

	it('shows a Draft pill on the draft tab only', async () => {
		await draft.findTabStripHandle(browser.webEngine);

		const tabs = await browser.webEngine.execute(() =>
			Array.from(document.querySelectorAll('.tab')).map((tab) => ({
				title: tab.querySelector('.title')?.textContent,
				pill: tab.querySelector('.draft')?.textContent ?? null,
			})),
		);
		const draftTab = tabs.find((t) => t.title === 'PromptTest.odt');
		const anchorTab = tabs.find((t) => t.title === 'simple.odt');
		expect(draftTab?.pill).toBe('Draft');
		expect(anchorTab?.pill).toBeNull();

		await browser.webEngine.switchToWindow(draftHandle);
	});

	it('keeps a modified draft open on Cancel', async () => {
		await draft.modifyDoc(browser.webEngine);
		await draft.requestClose(browser.webEngine);
		await draft.waitForClosePrompt(browser.webEngine);
		const question = await browser.webEngine.execute(
			() => document.getElementById('info-modal-label1')?.textContent,
		);
		expect(question).toContain('"PromptTest.odt"');
		await draft.answerClosePrompt(browser.webEngine, 'cancel');

		// Give a close that went on anyway the time to take the tab away.
		await browser.pause(2000);
		expect(await browser.webEngine.getWindowHandles()).toContain(draftHandle);
		await browser.webEngine.switchToWindow(draftHandle);
		const loaded = await browser.webEngine.execute(
			() => app.map._docLoaded === true,
		);
		expect(loaded).toBe(true);
		expect(existsSync(join(draft.draftsDir(), 'PromptTest.odt'))).toBe(true);
	});

	it('deletes the draft on Discard', async () => {
		await draft.requestClose(browser.webEngine);
		await draft.answerClosePrompt(browser.webEngine, 'discard');

		await draft.waitGone(browser.webEngine, draftHandle);
		await browser.webEngine.switchToWindow(anchorHandle);
		await browser.webEngine.waitUntil(
			async () => !existsSync(join(draft.draftsDir(), 'PromptTest.odt')),
			{ timeout: 10000, timeoutMsg: 'draft file was not deleted' },
		);
	});

	it('saves a closing draft to the chosen location on Save', async () => {
		const handle = await draft.createDraft(browser.webEngine, 'SaveOnClose');
		await draft.modifyDoc(browser.webEngine);
		await draft.requestClose(browser.webEngine);
		await draft.answerClosePrompt(browser.webEngine, 'save');

		const target = join(
			process.env.CODA_TEST_DOCUMENTS_DIR!,
			'SavedOnClose.odt',
		);
		await dialog.fillAndAccept(browser.native, target);

		await draft.waitGone(browser.webEngine, handle);
		await browser.webEngine.switchToWindow(anchorHandle);
		expect(existsSync(target)).toBe(true);
		expect(recentsContain('SavedOnClose.odt')).toBe(true);
	});

	it('keeps the draft open when the Save As chooser is cancelled', async () => {
		const handle = await draft.createDraft(browser.webEngine, 'ChooserCancel');
		await browser.webEngine.execute(() => {
			app.map.uiManager.setDocTypePref('onboardingTourSeen', true);
		});
		await draft.modifyDoc(browser.webEngine);
		await draft.requestClose(browser.webEngine);
		await draft.answerClosePrompt(browser.webEngine, 'save');
		await dialog.cancelSave(browser.native, 'Save did not open Save As');

		await browser.pause(2000);
		expect(await browser.webEngine.getWindowHandles()).toContain(handle);
		expect(existsSync(join(draft.draftsDir(), 'ChooserCancel.odt'))).toBe(true);

		// The close is over, so a later Save As must not close the tab.
		await browser.webEngine.switchToWindow(handle);
		await browser.webEngine.keys(['Control', 's']);
		const target = join(
			process.env.CODA_TEST_DOCUMENTS_DIR!,
			'ChooserCancelLater.odt',
		);
		await dialog.fillAndAccept(browser.native, target);
		await browser.webEngine.switchToWindow(handle);
		await (browser.webEngine as any).waitForCondition(
			() =>
				app.map.options.doc.includes('ChooserCancelLater.odt') &&
				app.map._docLoaded === true,
			{
				timeout: 45000,
				timeoutMsg: 'document did not rebind to the saved file',
			},
		);
		expect(await browser.webEngine.getWindowHandles()).toContain(handle);

		await draft.requestClose(browser.webEngine);
		await draft.waitGone(browser.webEngine, handle);
		await browser.webEngine.switchToWindow(anchorHandle);
	});

	it('brings a background draft to the front before asking', async () => {
		const handle = await draft.createDraft(browser.webEngine, 'Background');
		await browser.webEngine.execute(() => {
			app.map.uiManager.setDocTypePref('onboardingTourSeen', true);
		});
		await draft.modifyDoc(browser.webEngine);

		await onStripTab('simple.odt', 'activate');
		await browser.webEngine.switchToWindow(handle);
		await browser.webEngine.waitUntil(
			async () =>
				(await browser.webEngine.execute(() => document.visibilityState)) ===
				'hidden',
			{ timeout: 10000, timeoutMsg: 'the draft tab did not go to the back' },
		);

		await onStripTab('Background.odt', 'close');
		await browser.webEngine.switchToWindow(handle);
		// The prompt is a page dialog, which a hidden page does not draw.
		await draft.waitForClosePrompt(browser.webEngine);
		expect(
			await browser.webEngine.execute(() => document.visibilityState),
		).toBe('visible');

		// A second close finds the prompt open, and brings it to the front again.
		await onStripTab('simple.odt', 'activate');
		await browser.webEngine.switchToWindow(handle);
		await browser.webEngine.waitUntil(
			async () =>
				(await browser.webEngine.execute(() => document.visibilityState)) ===
				'hidden',
			{ timeout: 10000, timeoutMsg: 'the draft tab did not go to the back' },
		);
		await onStripTab('Background.odt', 'close');
		await browser.webEngine.switchToWindow(handle);
		await browser.webEngine.waitUntil(
			async () =>
				(await browser.webEngine.execute(() => document.visibilityState)) ===
				'visible',
			{
				timeout: 10000,
				timeoutMsg: 'the second close did not bring the draft to the front',
			},
		);

		await draft.answerClosePrompt(browser.webEngine, 'discard');
		await draft.waitGone(browser.webEngine, handle);
		await browser.webEngine.switchToWindow(anchorHandle);
	});

	it('closes an untouched background draft at the back', async () => {
		const handle = await draft.createDraft(browser.webEngine, 'BackUntouched');
		await onStripTab('simple.odt', 'activate');
		await browser.webEngine.switchToWindow(anchorHandle);
		await browser.webEngine.waitUntil(
			async () =>
				(await browser.webEngine.execute(() => document.visibilityState)) ===
				'visible',
			{ timeout: 10000, timeoutMsg: 'simple.odt did not come to the front' },
		);
		// The page in front goes hidden when another tab takes its place.
		await browser.webEngine.execute(() => {
			(window as any).__wentHidden = false;
			document.addEventListener('visibilitychange', () => {
				if (document.visibilityState === 'hidden')
					(window as any).__wentHidden = true;
			});
		});

		await onStripTab('BackUntouched.odt', 'close');
		await draft.waitGone(browser.webEngine, handle);
		await browser.webEngine.switchToWindow(anchorHandle);
		expect(
			await browser.webEngine.execute(() => (window as any).__wentHidden),
		).toBe(false);
	});

	it('stops a quit, and leaves the other windows open, on Cancel', async () => {
		const handle = await draft.createDraft(browser.webEngine, 'QuitCancel');
		await browser.webEngine.execute(() => {
			app.map.uiManager.setDocTypePref('onboardingTourSeen', true);
		});
		await draft.modifyDoc(browser.webEngine);

		// The draft gets a window of its own, which the quit reaches first as
		// the newest window.
		const stripsBefore = (await browser.webEngine.getWindowHandles()).length;
		await onStripTab('QuitCancel.odt', 'moveToNewWindow');
		await browser.webEngine.waitUntil(
			async () =>
				(await browser.webEngine.getWindowHandles()).length > stripsBefore,
			{ timeout: 15000, timeoutMsg: 'the draft did not move to a new window' },
		);

		await browser.webEngine.switchToWindow(handle);
		await browser.webEngine.execute(() => {
			setTimeout(() => window.postMobileMessage('uno .uno:Quit'), 100);
		});
		await draft.answerClosePrompt(browser.webEngine, 'cancel');

		await browser.pause(3000);
		const handles = await browser.webEngine.getWindowHandles();
		expect(handles).toContain(handle);
		expect(handles).toContain(anchorHandle);

		await browser.webEngine.switchToWindow(handle);
		await draft.requestClose(browser.webEngine);
		await draft.answerClosePrompt(browser.webEngine, 'discard');
		await draft.waitGone(browser.webEngine, handle);
		await browser.webEngine.switchToWindow(anchorHandle);
	});

	it('closes an untouched draft silently', async () => {
		const handle = await draft.createDraft(browser.webEngine, 'Untouched');
		await draft.requestClose(browser.webEngine);

		// No explicit no-prompt probe: a prompt would block the close, so the
		// tab going away below already proves the close was silent.
		await draft.waitGone(browser.webEngine, handle);
		await browser.webEngine.switchToWindow(anchorHandle);
		await browser.webEngine.waitUntil(
			async () => !existsSync(join(draft.draftsDir(), 'Untouched.odt')),
			{ timeout: 10000, timeoutMsg: 'untouched draft file was not deleted' },
		);
	});

	it('opens the Save As chooser for every Save of a draft', async () => {
		firstSaveHandle = await draft.createDraft(browser.webEngine, 'FirstSave');
		await browser.webEngine.execute(() => {
			app.map.uiManager.setDocTypePref('onboardingTourSeen', true);
		});
		await draft.modifyDoc(browser.webEngine);

		const saves: Array<[string, () => Promise<unknown>]> = [
			['Ctrl+S', () => browser.webEngine.keys(['Control', 's'])],
			[
				'the Save button',
				() =>
					browser.webEngine.execute(() => {
						const save = document.querySelector(
							'[id^="save"].unotoolbutton',
						) as HTMLElement;
						save.click();
					}),
			],
			[
				'the backstage Save',
				() =>
					browser.webEngine.execute(() => {
						app.map.backstageView!.show();
						(document.getElementById('backstage-save') as HTMLElement).click();
					}),
			],
			[
				'.uno:Save',
				() =>
					browser.webEngine.execute(() => {
						app.map.sendUnoCommand('.uno:Save');
					}),
			],
		];
		for (const [name, save] of saves) {
			await save();
			await dialog.cancelSave(browser.native, `${name} did not open Save As`);

			// A Save turned into Save As must leave the Save button as it was.
			await browser.webEngine.switchToWindow(firstSaveHandle);
			const button = await browser.webEngine.execute(() => {
				const save = document.querySelector('[id^="save"].unotoolbutton');
				return {
					disabled: save?.hasAttribute('disabled') ?? true,
					saving: save?.classList.contains('saving') ?? true,
				};
			});
			expect(button).toEqual({ disabled: false, saving: false });
		}
		expect(existsSync(join(draft.draftsDir(), 'FirstSave.odt'))).toBe(true);
	});

	it('rebinds the document on Save As, and then saves in place', async () => {
		const handle = firstSaveHandle;
		await browser.webEngine.keys(['Control', 's']);

		const target = join(
			process.env.CODA_TEST_DOCUMENTS_DIR!,
			'FirstSaveTarget.odt',
		);
		await dialog.fillAndAccept(browser.native, target);

		// The saveas rebinds the SAME WebView: the page reconnects its socket
		// rather than reloading, so the page URL keeps the old file_path.
		await browser.webEngine.switchToWindow(handle);
		await (browser.webEngine as any).waitForCondition(
			() =>
				app.map.options.doc.includes('FirstSaveTarget.odt') &&
				app.map._docLoaded === true,
			{
				timeout: 45000,
				timeoutMsg: 'document did not rebind to the saved file',
			},
		);
		expect(existsSync(target)).toBe(true);
		await browser.webEngine.waitUntil(
			async () => recentsContain('FirstSaveTarget.odt'),
			{ timeout: 10000, timeoutMsg: 'saved file did not enter recents' },
		);

		await draft.findTabStripHandle(browser.webEngine);
		await browser.webEngine.waitUntil(
			async () => {
				const tabs = await browser.webEngine.execute(() =>
					Array.from(document.querySelectorAll('.tab')).map((tab) => ({
						title: tab.querySelector('.title')?.textContent,
						pill: !!tab.querySelector('.draft'),
					})),
				);
				const t = tabs.find((x) => x.title === 'FirstSaveTarget.odt');
				return !!t && !t.pill;
			},
			{
				timeout: 15000,
				timeoutMsg: 'tab did not drop its Draft pill after Save As',
			},
		);
		await browser.webEngine.switchToWindow(handle);

		const savedAt = statSync(target).mtimeMs;
		await draft.modifyDoc(browser.webEngine);
		await browser.webEngine.keys(['Control', 's']);
		await browser.webEngine.waitUntil(
			async () => statSync(target).mtimeMs !== savedAt,
			{ timeout: 20000, timeoutMsg: 'Save did not write the saved file' },
		);
		expect(await dialog.isOpen(browser.native)).toBe(false);

		// No longer a draft: closing must not prompt.
		await draft.requestClose(browser.webEngine);
		await draft.waitGone(browser.webEngine, handle);
		await browser.webEngine.switchToWindow(anchorHandle);
	});
});
