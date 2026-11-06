/*
 * Copyright the Collabora Online contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

import { execFileSync } from 'child_process';
import {
	chmodSync,
	copyFileSync,
	existsSync,
	mkdirSync,
	readFileSync,
	writeFileSync,
} from 'fs';
import { join } from 'path';
import { copyFixture, openFixture } from '../lib/file-dialog.js';
import * as webview from '../lib/webview.js';

// A document whose file cannot be written must never lose its edits. Saving it
// shows one prompt that offers Save As, and closing it stops at the same prompt
// until the user picks what to do with the edits.
//
// In the save cases the file is made read-only after it is open, so the
// document itself opens editable and the failure only shows when the engine
// writes the file. A file that is already read-only at the open gives a
// read-only document, with a notice that points to Save As.

const we = (): WebdriverIO.Browser & {
	waitForCondition(
		predicate: () => boolean,
		opts?: { timeout?: number; interval?: number; timeoutMsg?: string },
	): Promise<boolean>;
} => browser.webEngine;

const PROMPT_ID = 'modal-dialog-savefailed';
const EDIT_TEXT = 'lockededit';
const NOTICE_TEXT = 'This file cannot be written';
const FILE_DIALOG_NAME =
	'//*[@accessibility-id="QApplication.QFileDialog.fileNameEdit"]';
const FILE_DIALOG_BUTTON =
	'//*[@accessibility-id="QApplication.QFileDialog.buttonBox.QPushButton"]';

let copyCount = 0;

function documentPath(name: string): string {
	return join(process.env.CODA_TEST_DOCUMENTS_DIR!, name);
}

function uniqueName(extension: string): string {
	copyCount++;
	return `save-failed-${process.pid}-${copyCount}.${extension}`;
}

// A one-page PDF with one line of text; the cross-reference offsets are
// counted as the objects are written.
function writePdf(): string {
	const objects = [
		'<< /Type /Catalog /Pages 2 0 R >>',
		'<< /Type /Pages /Kids [3 0 R] /Count 1 >>',
		'<< /Type /Page /Parent 2 0 R /MediaBox [0 0 612 792] /Contents 4 0 R ' +
			'/Resources << /Font << /F1 5 0 R >> >> >>',
		'<< /Length 34 >>\nstream\nBT /F1 24 Tf 72 700 Td (pdf) Tj ET\nendstream',
		'<< /Type /Font /Subtype /Type1 /BaseFont /Helvetica >>',
	];
	let pdf = '%PDF-1.4\n';
	const offsets: number[] = [];
	objects.forEach((body, i) => {
		offsets.push(pdf.length);
		pdf += `${i + 1} 0 obj\n${body}\nendobj\n`;
	});
	const xref = pdf.length;
	pdf += `xref\n0 ${objects.length + 1}\n0000000000 65535 f \n`;
	for (const offset of offsets)
		pdf += `${String(offset).padStart(10, '0')} 00000 n \n`;
	pdf += `trailer\n<< /Size ${objects.length + 1} /Root 1 0 R >>\n`;
	pdf += `startxref\n${xref}\n%%EOF\n`;
	const name = uniqueName('pdf');
	writeFileSync(documentPath(name), pdf);
	return name;
}

function pause(ms: number): Promise<void> {
	return new Promise((resolve) => setTimeout(resolve, ms));
}

async function isModified(): Promise<boolean> {
	return we().execute(() => app.file.modified === true);
}

async function promptShown(): Promise<boolean> {
	return we().execute((id) => document.getElementById(id) !== null, PROMPT_ID);
}

async function noticeCount(): Promise<number> {
	return we().execute(
		(text) =>
			Array.from(
				document.querySelectorAll<HTMLElement>('.snackbar #label-no-action'),
			).filter(
				(e) => e.offsetParent !== null && e.textContent!.startsWith(text),
			).length,
		NOTICE_TEXT,
	);
}

// The notice shows soon after the load or not at all, so give it a moment.
async function expectNoNotice(): Promise<void> {
	await pause(4000);
	expect(await noticeCount()).toBe(0);
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

async function makeModified(): Promise<void> {
	await we().execute((text) => {
		app.map.sendUnoCommand('.uno:InsertText', {
			Text: { type: 'string', value: text },
		});
	}, EDIT_TEXT);
	await we().waitUntil(isModified, {
		timeout: 15000,
		interval: 250,
		timeoutMsg: 'the edit did not mark the document modified',
	});
}

// Open a fresh copy of simple.odt, edit it, then take away write access to
// its file. Answers the web view handle and the file path.
async function openLockedModified(): Promise<{ handle: string; path: string }> {
	const name = copyFixture('simple.odt');
	const path = documentPath(name);
	const handle = await openFixture(browser.webEngine, browser.native, name);
	await enterEditMode();
	await makeModified();
	chmodSync(path, 0o444);
	return { handle, path };
}

async function waitForPrompt(): Promise<void> {
	await we().waitUntil(promptShown, {
		timeout: 20000,
		interval: 250,
		timeoutMsg: 'the save-failed prompt did not show',
	});
}

async function waitForNoPrompt(): Promise<void> {
	await we().waitUntil(async () => !(await promptShown()), {
		timeout: 10000,
		interval: 250,
		timeoutMsg: 'the save-failed prompt did not go away',
	});
}

// The prompt has no reason to show, so give it a moment and fail if it does.
async function expectNoPromptFor(ms: number): Promise<void> {
	await pause(ms);
	expect(await promptShown()).toBe(false);
}

async function promptButtons(): Promise<string[]> {
	return we().execute((id) => {
		const prompt = document.getElementById(id);
		if (!prompt) return [];
		// The expander of the technical details is a button too, not a choice.
		return Array.from(prompt.querySelectorAll('button'))
			.filter((button) => button.offsetParent !== null)
			.filter((button) => !button.closest('#technical-details-expander'))
			.map((button) => (button.textContent || '').trim())
			.filter((text) => text !== '');
	}, PROMPT_ID);
}

async function clickPromptButton(label: string): Promise<void> {
	const clicked = await we().execute(
		(id, text) => {
			const prompt = document.getElementById(id);
			if (!prompt) return false;
			const button = Array.from(prompt.querySelectorAll('button')).find(
				(b) => (b.textContent || '').trim() === text,
			);
			if (!button) return false;
			// A button can close this page, so click after execute() returns.
			setTimeout(() => button.click(), 100);
			return true;
		},
		PROMPT_ID,
		label,
	);
	expect(clicked).toBe(true);
}

async function technicalDetails(): Promise<string> {
	return we().execute((id) => {
		const prompt = document.getElementById(id);
		const text =
			prompt &&
			prompt.querySelector<HTMLTextAreaElement>('#technical-details-text');
		return text ? (text.value || text.textContent || '').trim() : '';
	}, PROMPT_ID);
}

// The old save error dialogs; the prompt replaces them.
async function errorAlertShown(): Promise<boolean> {
	return we().execute(
		() => document.getElementById('modal-dialog-cool_alert') !== null,
	);
}

// A close goes through the native side and a Save As opens a modal dialog,
// so the command is posted after execute() returns.
async function postUno(command: string): Promise<void> {
	await we().execute((uno) => {
		setTimeout(() => {
			window.postMobileMessage('uno ' + uno);
		}, 100);
	}, command);
}

// Type a path into the open or save dialog and accept it.
async function acceptFileDialog(path: string): Promise<void> {
	const nameEdit = await browser.native.$(FILE_DIALOG_NAME);
	await nameEdit.waitForExist({ timeout: 10000 });
	await nameEdit.setValue(path);
	const acceptButton = await browser.native.$(FILE_DIALOG_BUTTON);
	await acceptButton.waitForExist({ timeout: 5000 });
	await acceptButton.click();
}

// The name is prefilled, so the path goes in with a leading slash; the
// dialog folds the doubled slash.
async function acceptSaveDialog(path: string): Promise<void> {
	await acceptFileDialog('/' + path);
}

// Give the file its write access back and save; the document is clean after.
async function saveWritableFile(path: string): Promise<void> {
	chmodSync(path, 0o644);
	await we().execute(() => {
		app.map.save(false, false);
	});
	await we().waitUntil(async () => !(await isModified()), {
		timeout: 20000,
		interval: 250,
		timeoutMsg: 'the save to the writable file did not complete',
	});
}

// Open a file whose load fails: the page shows up, the document never does.
async function openFailing(name: string): Promise<string> {
	const before = await we().getWindowHandles();
	await postUno('.uno:Open');
	await acceptFileDialog(documentPath(name));
	return webview.switchToNewWebView(browser.webEngine, before);
}

async function switchToOtherDocument(handle: string): Promise<void> {
	for (const other of await we().getWindowHandles()) {
		if (other === handle) continue;
		await we().switchToWindow(other);
		if (await webview.isDocumentPage(browser.webEngine)) return;
	}
	throw new Error('no other document page is open');
}

async function isOffered(handle: string): Promise<boolean> {
	return (await we().getWindowHandles()).includes(handle);
}

// The tab in front is the one whose page is visible.
async function isFrontTab(): Promise<boolean> {
	return we().execute(() => document.visibilityState === 'visible');
}

async function waitForFrontTab(): Promise<void> {
	await we().waitUntil(isFrontTab, {
		timeout: 15000,
		interval: 250,
		timeoutMsg: 'the tab did not come to the front',
	});
}

// Count the results of saves wsd starts on its own, from now on.
async function watchAutosaves(): Promise<void> {
	await we().execute(() => {
		(window as any).__autosaveResults = 0;
		app.map.on('commandresult', (ev: any) => {
			if (ev.commandName === '.uno:Save' && ev.isAutosave)
				(window as any).__autosaveResults++;
		});
	});
}

// wsd saves on its own 30 s after the last change, once nothing else is
// being saved; a failed save is no exception.
async function waitForAutosave(): Promise<void> {
	await we().waitUntil(
		async () => we().execute(() => (window as any).__autosaveResults > 0),
		{
			timeout: 90000,
			interval: 1000,
			timeoutMsg: 'wsd did not save on its own',
		},
	);
}

async function expandTechnicalDetails(): Promise<void> {
	await we().execute((id) => {
		document
			.getElementById(id)!
			.querySelector<HTMLElement>('#technical-details-label-button')!
			.click();
	}, PROMPT_ID);
	await we().waitUntil(technicalDetailsExpanded, {
		timeout: 5000,
		interval: 200,
		timeoutMsg: 'the technical details did not expand',
	});
}

async function technicalDetailsExpanded(): Promise<boolean> {
	return we().execute((id) => {
		const prompt = document.getElementById(id);
		const button =
			prompt &&
			prompt.querySelector<HTMLElement>('#technical-details-label-button');
		return !!button && button.getAttribute('aria-expanded') === 'true';
	}, PROMPT_ID);
}

// Remember the prompt's element, to tell a rebuilt prompt from the same one.
async function rememberPrompt(): Promise<void> {
	await we().execute((id) => {
		(window as any).__promptNode = document.getElementById(id);
	}, PROMPT_ID);
}

async function isSamePrompt(): Promise<boolean> {
	return we().execute(
		(id) => document.getElementById(id) === (window as any).__promptNode,
		PROMPT_ID,
	);
}

// The driver crashes when the page it is attached to goes away, so it moves
// to another document, the keeper unless given, before the close lands.
async function waitGone(handle: string, switchTo?: string): Promise<void> {
	if (switchTo) await we().switchToWindow(switchTo);
	else await we().switchToWindow(keeperHandle);
	await we().waitUntil(async () => !(await isOffered(handle)), {
		timeout: 20000,
		interval: 250,
		timeoutMsg: 'the document tab did not close',
	});
}

// The app quits when its last document closes; this one keeps it running
// while the cases close theirs.
let keeperHandle = '';

describe('Saving a document whose file cannot be written', () => {
	before(async function () {
		if (process.env.CODA_PLATFORM !== 'qt') {
			this.skip();
		}
		this.timeout(90000);
		keeperHandle = await openFixture(
			browser.webEngine,
			browser.native,
			copyFixture('scrolling.odt'),
		);
	});

	afterEach(async () => {
		if (keeperHandle && (await isOffered(keeperHandle)))
			await we().switchToWindow(keeperHandle);
	});

	it('offers Save As in one prompt, and Cancel keeps the edits', async function () {
		this.timeout(120000);
		const { handle } = await openLockedModified();

		await we().execute(() => {
			app.map.save(false, false);
		});
		await waitForPrompt();
		expect(await promptButtons()).toEqual(['Cancel', 'Save As']);
		expect(await errorAlertShown()).toBe(false);
		// The engine's reason for the failure is there to show.
		expect(await technicalDetails()).not.toBe('');
		// Enter picks Save As, not the expander of the details.
		await we().waitUntil(
			async () =>
				we().execute((id) => {
					const prompt = document.getElementById(id);
					const focused = document.activeElement;
					return (
						!!prompt &&
						!!focused &&
						prompt.contains(focused) &&
						(focused.textContent || '').trim() === 'Save As'
					);
				}, PROMPT_ID),
			{
				timeout: 5000,
				interval: 200,
				timeoutMsg: 'Save As does not have the first focus',
			},
		);

		await clickPromptButton('Cancel');
		await waitForNoPrompt();
		expect(await isOffered(handle)).toBe(true);
		expect(await isModified()).toBe(true);
	});

	it('keeps quiet when a save wsd started on its own fails', async function () {
		this.timeout(180000);
		const { handle, path } = await openLockedModified();
		await watchAutosaves();

		await waitForAutosave();
		await expectNoPromptFor(3000);
		expect(await errorAlertShown()).toBe(false);
		expect(await isOffered(handle)).toBe(true);
		expect(await isModified()).toBe(true);

		await saveWritableFile(path);
		await postUno('.uno:CloseWin');
		await waitGone(handle);
	});

	it('stops a close, and Close without saving leaves the file as it was', async function () {
		this.timeout(120000);
		const { handle, path } = await openLockedModified();

		await postUno('.uno:CloseWin');
		await waitForPrompt();
		expect(await promptButtons()).toEqual([
			'Cancel',
			'Close without saving',
			'Save As',
		]);
		expect(await errorAlertShown()).toBe(false);

		// Escape dismisses the prompt like Cancel: the tab and its edits stay.
		await we().keys('Escape');
		await waitForNoPrompt();
		await expectNoPromptFor(3000);
		expect(await isOffered(handle)).toBe(true);
		expect(await isModified()).toBe(true);

		// The dismissed close is over: a save that works now keeps the tab.
		await saveWritableFile(path);
		await pause(2000);
		expect(await isOffered(handle)).toBe(true);

		await makeModified();
		chmodSync(path, 0o444);
		const bytesBefore = readFileSync(path);
		await postUno('.uno:CloseWin');
		await waitForPrompt();
		await clickPromptButton('Close without saving');
		await waitGone(handle);
		expect(readFileSync(path).equals(bytesBefore)).toBe(true);
	});

	it('treats the X of the close prompt as Cancel', async function () {
		this.timeout(120000);
		const { handle, path } = await openLockedModified();

		await postUno('.uno:CloseWin');
		await waitForPrompt();
		await we().execute((id) => {
			const close = document.querySelector<HTMLElement>(
				'#' + id + ' .ui-dialog-titlebar-close',
			);
			close!.click();
		}, PROMPT_ID);
		await waitForNoPrompt();
		expect(await isOffered(handle)).toBe(true);
		expect(await isModified()).toBe(true);

		// The close is called off: a save that works now keeps the tab.
		await saveWritableFile(path);
		await pause(2000);
		expect(await isOffered(handle)).toBe(true);

		await postUno('.uno:CloseWin');
		await waitGone(handle);
	});

	it('names the chosen file when a Save As fails', async function () {
		this.timeout(120000);
		const { handle, path } = await openLockedModified();
		const dir = documentPath(`save-failed-${process.pid}-nowrite`);
		mkdirSync(dir, { recursive: true });
		chmodSync(dir, 0o555);
		const target = join(dir, 'chosen-name.odt');

		try {
			await postUno('.uno:SaveAs');
			await acceptSaveDialog(target);

			await waitForPrompt();
			const text = await we().execute(
				(id) => document.getElementById(id)!.textContent || '',
				PROMPT_ID,
			);
			expect(text).toContain('chosen-name.odt');
			expect(text).not.toContain(path.split('/').pop()!);
			expect(await promptButtons()).toEqual(['Cancel', 'Save As']);

			await clickPromptButton('Cancel');
			await waitForNoPrompt();
			expect(existsSync(target)).toBe(false);
			expect(await isOffered(handle)).toBe(true);
			expect(await isModified()).toBe(true);
		} finally {
			chmodSync(dir, 0o755);
		}
	});

	it('saves the edits to a new file from the close prompt and keeps the tab', async function () {
		this.timeout(120000);
		const { handle, path } = await openLockedModified();
		const newPath = path.replace(/\.odt$/, '-copy.odt');

		await postUno('.uno:CloseWin');
		await waitForPrompt();
		await clickPromptButton('Save As');
		await acceptSaveDialog(newPath);

		await we().waitUntil(async () => existsSync(newPath), {
			timeout: 20000,
			interval: 250,
			timeoutMsg: 'Save As did not write the new file',
		});
		await we().waitUntil(
			async () =>
				we().execute(
					(name) =>
						app.map._docLoaded === true && document.title.indexOf(name) !== -1,
					newPath.split('/').pop()!,
				),
			{
				timeout: 30000,
				interval: 250,
				timeoutMsg: 'the tab did not load the new file',
			},
		);
		const content = execFileSync('unzip', ['-p', newPath, 'content.xml']);
		expect(content.toString()).toContain(EDIT_TEXT);
		expect(await isOffered(handle)).toBe(true);

		await postUno('.uno:CloseWin');
		await waitGone(handle);
	});

	it('keeps the close prompt as it is while a save wsd started on its own fails', async function () {
		this.timeout(180000);
		const { handle, path } = await openLockedModified();
		await watchAutosaves();

		await postUno('.uno:CloseWin');
		await waitForPrompt();
		await rememberPrompt();
		await expandTechnicalDetails();

		await waitForAutosave();
		await pause(1000);
		expect(await isSamePrompt()).toBe(true);
		expect(await technicalDetailsExpanded()).toBe(true);
		expect(await promptButtons()).toEqual([
			'Cancel',
			'Close without saving',
			'Save As',
		]);

		await clickPromptButton('Cancel');
		await waitForNoPrompt();
		expect(await isOffered(handle)).toBe(true);
		await saveWritableFile(path);
		await postUno('.uno:CloseWin');
		await waitGone(handle);
	});

	it('does not prompt for a failed save wsd started on its own, but does for the next Ctrl+S', async function () {
		this.timeout(120000);
		const { handle, path } = await openLockedModified();

		await we().execute(() => {
			app.map.save(false, false);
		});
		await waitForPrompt();
		await clickPromptButton('Cancel');
		await waitForNoPrompt();

		// What wsd forwards for a failed save it started on its own.
		await we().execute(() => {
			app.map.fire('commandresult', {
				commandName: '.uno:Save',
				success: false,
				result: { type: 'string', value: 'cannot write' },
				isAutosave: true,
			});
		});
		await expectNoPromptFor(2000);
		expect(await isModified()).toBe(true);

		await we().keys(['Control', 's']);
		await waitForPrompt();
		expect(await promptButtons()).toEqual(['Cancel', 'Save As']);

		await clickPromptButton('Cancel');
		await waitForNoPrompt();
		await saveWritableFile(path);
		await postUno('.uno:CloseWin');
		await waitGone(handle);
	});

	it('brings a background tab to the front when the save for its close fails', async function () {
		this.timeout(120000);
		const { handle, path } = await openLockedModified();
		const frontHandle = await openFixture(
			browser.webEngine,
			browser.native,
			copyFixture('simple.odt'),
		);
		expect(await isFrontTab()).toBe(true);

		// A close from the tab strip runs in the background tab's own page.
		await we().switchToWindow(handle);
		expect(await isFrontTab()).toBe(false);
		await postUno('.uno:CloseWin');
		await waitForPrompt();
		await waitForFrontTab();
		expect(await promptButtons()).toEqual([
			'Cancel',
			'Close without saving',
			'Save As',
		]);

		await clickPromptButton('Cancel');
		await waitForNoPrompt();
		await saveWritableFile(path);
		await postUno('.uno:CloseWin');
		await waitGone(handle);
		await we().switchToWindow(frontHandle);
		await postUno('.uno:CloseWin');
		await waitGone(frontHandle);
	});

	it('leaves the front tab alone when a background tab closes after a good save', async function () {
		this.timeout(120000);
		const name = copyFixture('simple.odt');
		const handle = await openFixture(browser.webEngine, browser.native, name);
		await enterEditMode();
		await makeModified();
		const frontHandle = await openFixture(
			browser.webEngine,
			browser.native,
			copyFixture('simple.odt'),
		);

		await we().switchToWindow(handle);
		expect(await isFrontTab()).toBe(false);
		await postUno('.uno:CloseWin');
		await waitGone(handle, frontHandle);
		expect(await isFrontTab()).toBe(true);
		const content = execFileSync('unzip', [
			'-p',
			documentPath(name),
			'content.xml',
		]);
		expect(content.toString()).toContain(EDIT_TEXT);

		await postUno('.uno:CloseWin');
		await waitGone(frontHandle);
	});

	it('opens a read-only file read-only, says so once, and still offers Save As', async function () {
		this.timeout(120000);
		const name = copyFixture('simple.odt');
		chmodSync(documentPath(name), 0o444);
		await openFixture(browser.webEngine, browser.native, name);

		await we().waitUntil(async () => (await noticeCount()) === 1, {
			timeout: 15000,
			interval: 250,
			timeoutMsg: 'the read-only notice did not show',
		});
		expect(await we().execute(() => (app as any).isReadOnly())).toBe(true);
		expect(
			await we().execute(() => {
				const label = document.getElementById('readonlyMode');
				return label !== null && label.offsetParent !== null;
			}),
		).toBe(true);
		await pause(2000);
		expect(await noticeCount()).toBe(1);

		await postUno('.uno:SaveAs');
		const nameEdit = await browser.native.$(FILE_DIALOG_NAME);
		await nameEdit.waitForExist({ timeout: 10000 });
		const cancelButton = await browser.native.$(
			FILE_DIALOG_BUTTON + '[@name="Cancel"]',
		);
		await cancelButton.click();
		await nameEdit.waitForExist({ timeout: 10000, reverse: true });
	});

	it('shows no read-only notice for a writable file', async function () {
		this.timeout(120000);
		await openFixture(
			browser.webEngine,
			browser.native,
			copyFixture('simple.odt'),
		);
		await expectNoNotice();
	});

	it('shows no read-only notice for a PDF', async function () {
		this.timeout(120000);
		await openFixture(browser.webEngine, browser.native, writePdf());
		await expectNoNotice();
	});

	it('shows no read-only notice when a read-only file fails to load', async function () {
		this.timeout(120000);
		// The load stops at the password: the page drops its connection and
		// asks for one.
		const name = uniqueName('ods');
		copyFileSync(
			join(process.cwd(), '../../test/data/password-protected.ods'),
			documentPath(name),
		);
		chmodSync(documentPath(name), 0o444);
		const handle = await openFailing(name);

		await we().waitUntil(
			async () =>
				we().execute(
					() => document.getElementById('modal-dialog-password-popup') !== null,
				),
			{
				timeout: 30000,
				interval: 250,
				timeoutMsg: 'the password prompt did not show',
			},
		);
		await expectNoNotice();

		// With no document the tab closes at once, so give the driver time to
		// move to another document first. The keeper may have no page by now:
		// the app drops the page of a tab that stays long in the background.
		await we().execute(() => {
			setTimeout(() => {
				window.postMobileMessage('uno .uno:CloseWin');
			}, 2000);
		});
		await switchToOtherDocument(handle);
		await we().waitUntil(async () => !(await isOffered(handle)), {
			timeout: 15000,
			interval: 250,
			timeoutMsg: 'the document tab did not close',
		});
	});
});
